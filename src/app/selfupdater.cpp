#include "selfupdater.h"

namespace Updater
{

    /**
     * @brief self_update::self_update
     *
     * Self_Update implements a self-update strategy for this executable.
     *
     * 1. check, if new version available
     *    updateAvailable() -> getUpdateInfo()
     * 2. download new version (zip)
     * 3. remove .old file
     * 4. rename running "wpn-xm.exe" to "wpn-xm.exe.old"
     * 5. extract "wpn-xm.exe" (new version replaces old one)
     * 6. indicate need for a manual restart
     *    - if user selects "restart"
     *      - start new version as detached Process
     *      - exit this version
     */
    SelfUpdater::SelfUpdater(Settings::SettingsManager *oSettings)
    {
        qDebug() << "[SelfUpdater] Started...";

        settings = oSettings;

        userRequestedUpdate = false;
    }

    SelfUpdater::~SelfUpdater() { delete settings; }

    void SelfUpdater::run()
    {
        if (network.networkAccessible() == QNetworkAccessManager::NotAccessible) {
            qDebug() << "[SelfUpdater] Run skipped, because no network.";
            return;
        }

        // check, if interval was set in INI
        // if not set, the interval defaults to "notset"
        QString intervalString = settings->getString("selfupdater/interval", QVariant(QString("notset")));

        qint64 lastTimeChecked = settings->get("selfupdater/last_time_checked").toLongLong();

        qint64 interval = QDateTime::currentDateTime().currentMSecsSinceEpoch();

        if (intervalString == "daily") {
            interval = QDateTime::currentDateTime().addDays(Interval::Daily).currentMSecsSinceEpoch();
        }
        if (intervalString == "weekly") {
            interval = QDateTime::currentDateTime().addDays(Interval::Weekly).currentMSecsSinceEpoch();
        }
        if (intervalString == "monthly") {
            interval = QDateTime::currentDateTime().addDays(Interval::Monthly).currentMSecsSinceEpoch();
        }

        qint64 now = QDateTime::currentDateTime().currentMSecsSinceEpoch();

        bool timeForUpdateCheck = false;
        if (now - interval > lastTimeChecked) {
            timeForUpdateCheck = true;
        }

        qDebug() << "[SelfUpdater] IsUserRequestedUpdate: " << userRequestedUpdate;
        qDebug() << "[SelfUpdater] LastTimeChecked: " << QVariant(lastTimeChecked).toDateTime().toString(Qt::ISODate);
        qDebug() << "[SelfUpdater] TimeForUpdateChecked: " << timeForUpdateCheck;

        // run update in 3 cases
        // 1. userRequestedUpdate = forced update run, regardless of interval
        // 2. lastTimeCheck 0 = an update was never done before
        // 3. based on the selected update interval = it's now time to check for updates
        if (userRequestedUpdate || lastTimeChecked == 0 || timeForUpdateCheck) {

            // set the last time check flag
            settings->set("selfupdater/last_time_checked", QDateTime::currentDateTime().toString(Qt::ISODate));

            // setup download folder
            downloadFolder = QCoreApplication::applicationDirPath() + QDir::separator() + "downloads";
            if (!QDir(downloadFolder).exists()) {
                QDir(downloadFolder).mkpath(".");
            }

            if (updateAvailable()) {
                emit notifyUpdateAvailable(versionInfo);
                if (settings->getBool("selfupdater/autoupdate")) {
                    doUpdate();
                }
            }
        }
    }

    void SelfUpdater::doUpdate()
    {
        downloadNewVersion();
        renameExecutable();
        // extract();             is called when download transfer finished
        // askForRestart();       is called when extraction finished
    }

    bool SelfUpdater::updateAvailable()
    {
        versionInfo = getVersionInfo();

        // qDebug() << versionInfo;

        return versionInfo["update_available"].toBool();
    }

    void SelfUpdater::downloadNewVersion()
    {
        qDebug() << "SelfUpdater::downloadNewVersion";

        QString downloadURL(versionInfo["url"].toString());

        QNetworkRequest request(downloadURL);

        downloadManager.setDownloadFolder(downloadFolder);
        downloadManager.setDownloadMode(Downloader::DownloadItem::DownloadMode::SkipIfExists);
        downloadManager.setQueueMode(Downloader::DownloadManager::QueueMode::Serial);
        downloadManager.get(request);

        Downloader::TransferItem *transfer = downloadManager.findTransfer(downloadURL);
        connect(transfer, SIGNAL(transferFinished(Downloader::TransferItem *)), this, SLOT(extract()));

        // finally: invoke downloading
        QMetaObject::invokeMethod(&downloadManager, "checkForAllDone", Qt::QueuedConnection);
    }

    void SelfUpdater::renameExecutable()
    {
        QString dirPath        = QCoreApplication::applicationDirPath();
        QString exeFilePath    = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        QString exeName        = QFileInfo(exeFilePath).fileName();
        QString oldExeName     = exeName + ".old";
        QString oldExeFilePath = QDir::toNativeSeparators(dirPath + QDir::separator() + oldExeName);

        qDebug() << "[SelfUpdater] Renaming";
        qDebug() << exeFilePath;
        qDebug() << oldExeFilePath;

        // delete the destination file first (the old ".old" file)
        if (QFile::exists(oldExeFilePath)) {
            qDebug() << "[SelfUpdater] Deleted old wpn-xm.exe.old:" << QFile::remove(oldExeFilePath);
        }

        qDebug() << "[SelfUpdater] Renamed wpn-xm.exe to wpn-xm.exe.old:" << QFile::rename(exeFilePath, oldExeFilePath);
    }

    void SelfUpdater::extract()
    {
        qDebug() << "[SelfUpdater] Extract started.";

        QUrl url(versionInfo["url"].toString());
        QString fileToExtract = "wpn-xm.exe";
        QString zipArchiveFile(QDir::toNativeSeparators(downloadFolder + QDir::separator() + url.fileName()));
        QString targetPath(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));

        if (!QFile::exists(zipArchiveFile)) {
            qDebug() << "[SelfUpdater] Zip File missing" << zipArchiveFile;
        }

        qDebug() << "[SelfUpdater] Extracting " << fileToExtract << "from" << zipArchiveFile << "to" << targetPath;

        QuaZipFileInfo info;

        QuaZip zipArchive(zipArchiveFile);

        if (!zipArchive.open(QuaZip::mdUnzip)) {
            qWarning("Cannot open zip archive, error: %d", zipArchive.getZipError());
            // return false;
        }

        qDebug() << "[SelfUpdater] Extracting files:" << zipArchive.getFileNameList();

        QuaZipFile zippedFile(&zipArchive);

        QFile outFile;

        for (bool more = zipArchive.goToFirstFile(); more; more = zipArchive.goToNextFile()) {

            if (!zipArchive.getCurrentFileInfo(&info)) {
                qWarning("Cannot get zip file info, error: %d", zipArchive.getZipError());
            }

            // it's a folder
            if (info.name.endsWith("/")) {
                QDir dir(targetPath);
                dir.mkpath(info.name);
                continue;
            }

            // is this the file we want?
            if (!fileToExtract.isEmpty()) {
                if (!info.name.contains(fileToExtract)) {
                    continue;
                }
            }

            zippedFile.open(QIODevice::ReadOnly);

            QString name = QString("%1/%2").arg(targetPath).arg(fileToExtract);

            if (zippedFile.getZipError() != UNZ_OK) {
                qWarning("Cannot get actual file name, error: %d", zippedFile.getZipError());
            }

            outFile.setFileName(name);
            outFile.open(QIODevice::WriteOnly);
            QByteArray data = zippedFile.readAll();
            outFile.write(data);
            outFile.close();

            if (zippedFile.getZipError() != UNZ_OK) {
                qWarning("Read zip file error: %d", zippedFile.getZipError());
            }

            if (!zippedFile.atEnd()) {
                qWarning("Read all data from zip file, but it's not EOF.");
            }

            zippedFile.close();

            if (zippedFile.getZipError() != UNZ_OK) {
                qWarning("Cannot close zip file, error: %d", zippedFile.getZipError());
            }
        }

        zipArchive.close();

        if (zipArchive.getZipError() != UNZ_OK) {
            qWarning("Cannot close zip archive: %d", zipArchive.getZipError());
        }

        qDebug() << "[SelfUpdater] Extract: OK.";

        if (settings->getBool("selfupdater/autorestart")) {
            emit notifyRestartNeeded(versionInfo);
        } else {
            askForRestart();
        }
    }

    void SelfUpdater::askForUpdate()
    {
        QString t(
            "A new version of the Server Control Panel is available:"
            "<p align='center'><b><FONT COLOR='#a9a9a9' FONT SIZE = 5>"
            "%1 v%2"
            "</b></p></br>");

        QString text = t.arg(versionInfo["software_name"].toString(), versionInfo["latest_version"].toString());

        QString infoText = "Do you want to update now?";

        QPixmap iconPixmap = QIcon(":/update").pixmap(80, 80);

        QMessageBox msgBox;
        msgBox.setIconPixmap(iconPixmap);
        msgBox.setWindowTitle("WPN-XM Server Control Panel - Update Notification");
        msgBox.setText(text);
        msgBox.setInformativeText(infoText);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setButtonText(QMessageBox::Yes, tr("Update"));
        msgBox.setButtonText(QMessageBox::No, tr("Continue"));
        msgBox.setDefaultButton(QMessageBox::Yes);

        if (msgBox.exec() == QMessageBox::Yes) {
            doUpdate();
        }
    }

    void SelfUpdater::askForRestart()
    {
        QString text = "The Server Control Panel was updated from ";
        text.append(QString("v%1 to v%2.").arg(APP_VERSION_SHORT, versionInfo["latest_version"].toString()));

        QString infoText =
            "The update was installed. "
            "You can restart the Server Control Panel now, "
            "or continue working and restart later.";

        QPixmap iconPixmap = QIcon(":/update").pixmap(80, 80);

        QMessageBox msgBox;
        msgBox.setIconPixmap(iconPixmap);
        msgBox.setWindowTitle("WPN-XM Server Control Panel - Update Notification");
        msgBox.setText(text);
        msgBox.setInformativeText(infoText);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setButtonText(QMessageBox::Yes, tr("Restart SCP"));
        msgBox.setButtonText(QMessageBox::No, tr("Continue"));
        msgBox.setDefaultButton(QMessageBox::Yes);

        if (msgBox.exec() == QMessageBox::Yes) {
            // Should we send a final farewell signal before we leave?
            // QApplication::aboutToQuit(finalFarewellSignal);

            // let's hope that starting the new process is slow.
            // so slow, that we are not running into the single application check.

            QString exe = QApplication::applicationFilePath();
            if (QFile::exists(exe)) {
                qDebug() << "[SelfUpdater] Restarting: " << exe;
                QProcess::startDetached(exe);
            } else if (QFile::exists(QApplication::applicationDirPath() + "/wpn-xm.exe")) {
                qDebug() << "[SelfUpdater] Restarting (debug): " << QApplication::applicationDirPath() + "/wpn-xm.exe";
                QProcess::startDetached(QApplication::applicationDirPath() + "/wpn-xm.exe");
            } else {
                qDebug() << "[SelfUpdater] Executable for restart not found!" << exe;
            }

            QApplication::exit();
        }
    }

    // ----------------------------------------------------------------

    QJsonObject SelfUpdater::getVersionInfo()
    {
        QString url = getUpdateCheckURL();

        // qDebug() << "[SelfUpdater] UpdateCheckURL: " << url;

        // QNAM is non-blocking / non-synchronous, but we want to wait until reply has
        // been received
        // create custom temporary event loop on stack to block the stack until
        // finished received
        QEventLoop eventLoop;

        // "quit()" the event-loop, when the network request "finished()"
        QObject::connect(&network, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));

        // the HTTP request
        QNetworkRequest req(url);
        QNetworkReply *updateCheckResponse = network.get(req);

        // run event loop, which blocks the stack, until "exit()" has been called
        eventLoop.exec();

        QJsonDocument jsonResponse;

        if (updateCheckResponse->error() == QNetworkReply::NoError) {

            // read response and parse JSON
            QString strReply = updateCheckResponse->readAll().data();
            jsonResponse     = QJsonDocument::fromJson(strReply.toUtf8());

            // qDebug() << "RawResponse: " << strReply;
            // qDebug() << "JsonResponse: " << jsonResponse;
        } else {
            // QNetworkReply::HostNotFoundError
            // qDebug() << "Request Failure: " << updateCheckResponse->errorString();

            QMessageBox::critical(QApplication::activeWindow(), "Request Failure", updateCheckResponse->errorString(),
                                  QMessageBox::Ok);
        }

        eventLoop.exit();

        // cleanup
        delete updateCheckResponse;

        return jsonResponse.object();
    }

    QString SelfUpdater::getUpdateCheckURL()
    {
        QString url("https://wpn-xm.org/updatecheck.php");

        // software
        url.append("?s=");
        if (WindowsAPI::QtWin::running_on_64bit_os()) {
            url.append("wpnxm-scp-x64");
        } else {
            url.append("wpnxm-scp");
        }

        // version
        url.append("&v=");
        QString version(APP_VERSION_SHORT);
        if (version != "@APPVERSIONSHORT@") {
            url.append(version);
        } else {
            url.append("0.8.4"); // hardcoded for testing
        }

        return url;
    }
} // namespace Updater
