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
    SelfUpdater::SelfUpdater()
    {
        qDebug() << "[SelfUpdater] Started...";

        userRequestedUpdate = false;
    }

    SelfUpdater::~SelfUpdater() {}

    void SelfUpdater::run()
    {
        if (network.networkAccessible() == QNetworkAccessManager::NotAccessible) {
            qDebug() << "[SelfUpdater] Run skipped, because no network.";
            return;
        }

        // check, if interval was set in INI
        // if not set, the interval defaults to off
        QString intervalString = settings->get("selfupdater/interval", QVariant(QString("off"))).toString();

        qint64 lastTimeChecked =  settings->get("selfupdater/last_time_checked", QVariant(0)).toDateTime().currentMSecsSinceEpoch();

        qint64 interval;
        if(intervalString == "off") {
            interval = QDateTime::currentDateTime().currentMSecsSinceEpoch();
        }
        if(intervalString == "daily") {
            interval = QDateTime::currentDateTime().addDays(Interval::Daily).currentMSecsSinceEpoch();
        }
        if(intervalString == "weekly") {
            interval = QDateTime::currentDateTime().addDays(Interval::Weekly).currentMSecsSinceEpoch();
        }
        if(intervalString == "monthly") {
            interval = QDateTime::currentDateTime().addDays(Interval::Monthly).currentMSecsSinceEpoch();
        }

        qint64 now = QDateTime::currentDateTime().currentMSecsSinceEpoch();

        // run update in 3 cases
        // 1. userRequestedUpdate = forced update run, regardless of interval
        // 2. lastTimeCheck 0 = an update was never done before
        // 3. based on the selected update interval = it's now time to update
        if(userRequestedUpdate
           || lastTimeChecked == 0
           || now - interval > lastTimeChecked) {
            // set the last time check flag
             settings->get("selfupdater/last_time_checked", QDateTime::currentDateTime().toString(Qt::ISODate));

            // setup download folder
            downloadFolder =
                QCoreApplication::applicationDirPath() + QDir::separator() + "downloads";
            if (!QDir(downloadFolder).exists()) {
                QDir(downloadFolder).mkpath(".");
            }

            if (updateAvailable()) {
                qDebug() << "[SelfUpdater] Update available \n VersionInfo:" << versionInfo;

                emit notifyUpdateAvailable(versionInfo);

                if (settings->get("selfupdater/autoupdate").toBool()) {
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
        return versionInfo["update_available"].toBool();
    }

    void SelfUpdater::downloadNewVersion()
    {
        qDebug() << "SelfUpdater::downloadNewVersion";

        QString downloadURL(versionInfo["url"].toString());

        QNetworkRequest request(downloadURL);

        downloadManager.setDownloadFolder(downloadFolder);
        downloadManager.setDownloadMode(
            Downloader::DownloadItem::DownloadMode::SkipIfExists);
        downloadManager.setQueueMode(Downloader::DownloadManager::QueueMode::Serial);
        downloadManager.get(request);

        Downloader::TransferItem *transfer =
            downloadManager.findTransfer(downloadURL);
        connect(transfer, SIGNAL(transferFinished(Downloader::TransferItem *)), this,
                SLOT(extract()));

        // finally: invoke downloading
        QMetaObject::invokeMethod(&downloadManager, "checkForAllDone",
                                  Qt::QueuedConnection);
    }

    void SelfUpdater::extract()
    {
        qDebug() << "SelfUpdater::extract";

        QUrl url(versionInfo["url"].toString());
        QString fileToExtract = "wpn-xm.exe";
        QString zipFile(QDir::toNativeSeparators(downloadFolder + QDir::separator() +
                                                 url.fileName()));
        QString targetPath(
            QDir::toNativeSeparators(QCoreApplication::applicationDirPath()));

        if (!QFile(zipFile).exists()) {
            qDebug() << "[SelfUpdater] Zip File missing" << zipFile;
        }

        qDebug() << "[SelfUpdater] Extracting " << fileToExtract << "from" << zipFile
                 << "to" << targetPath;

        // WTF? extractFile() doesn't work ???
        // qDebug() << "[SelfUpdater] Filelist:" << JlCompress::getFileList(zipFile);
        // qDebug() << JlCompress::extractFile( zipFile, fileToExtract,
        // QDir::toNativeSeparators(downloadFolder));
        // qDebug() << JlCompress::extractFile( zipFile, fileToExtract, targetPath);

        // thanks to me, the zip file contains only the executable
        // so extractDir() is basically extractFile(), but still.... grrrr
        QStringList extractedFiles = JlCompress::extractDir(zipFile, targetPath);
        if (QFileInfo(extractedFiles.at(0)).fileName() == "wpn-xm.exe") {
            qDebug() << "[SelfUpdater] ---- Success! ----";

            if (settings->get("selfupdater/autorestart").toBool()) {
                emit notifyRestartNeeded(versionInfo);
            } else {
                askForRestart();
            }
        }
    }

    void SelfUpdater::renameExecutable()
    {
        QString dirPath = QCoreApplication::applicationDirPath();
        QString exeFilePath =
            QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        QString exeName = QFileInfo(exeFilePath).fileName();
        QString oldExeName = exeName + ".old";
        QString oldExeFilePath =
            QDir::toNativeSeparators(dirPath + QDir::separator() + oldExeName);

        qDebug() << "[SelfUpdater] Renaming";
        qDebug() << exeFilePath;
        qDebug() << oldExeFilePath;

        // delete the destination file first (the old ".old" file)
        if (QFile().exists(oldExeFilePath)) {
            qDebug() << "[SelfUpdater] delete target first:"
                     << QFile::remove(oldExeFilePath);
        }

        qDebug() << "[SelfUpdater] Move:"
                 << QFile::rename(exeFilePath,
                                  oldExeFilePath); // wpn-xm.exe -> wpn-xm.exe.old
    }

    void SelfUpdater::askForUpdate()
    {
        QString t(
            "A new version of the Server Control Panel is available:"
            "<p><b><FONT COLOR='#a9a9a9' FONT SIZE = 4>"
            "%1 v%2."
            "</b></p></br>");
        QString text = t.arg(versionInfo["software_name"].toString(),
                             versionInfo["latest_version"].toString());

        QString infoText = "Do you want to update now?";

        QPixmap iconPixmap = QIcon(":/update").pixmap(80, 80);

        QMessageBox msgBox;
        msgBox.setIconPixmap(iconPixmap);
        msgBox.setWindowTitle("WPN-XM Server Control Panel - Self Updater");
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
        text.append(
            QString("v%1 to v%2.")
                .arg(APP_VERSION_SHORT, versionInfo["latest_version"].toString()));

        QString infoText =
            "The update was installed. "
            "You can restart the Server Control Panel now, "
            "or continue working and restart later.";

        QPixmap iconPixmap = QIcon(":/update").pixmap(80, 80);

        QMessageBox msgBox;
        msgBox.setIconPixmap(iconPixmap);
        msgBox.setWindowTitle("WPN-XM Server Control Panel - Self Updater");
        msgBox.setText(text);
        msgBox.setInformativeText(infoText);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setButtonText(QMessageBox::Yes, tr("Restart SCP"));
        msgBox.setButtonText(QMessageBox::No, tr("Continue"));
        msgBox.setDefaultButton(QMessageBox::Yes);

        if (msgBox.exec() == QMessageBox::Yes) {
            // Should we send a final farewell signal before we leave?
            // QApplication::aboutToQuit(finalFarewellSignal);

            // cross fingers and hope and pray, that starting the new process is slow
            // and we are not running into the single application check.. ;)

            QProcess p;
            p.startDetached(QApplication::applicationFilePath());

            QApplication::exit();
        }
    }

    // ----------------------------------------------------------------

    QJsonObject SelfUpdater::getVersionInfo()
    {
        QString url = getUpdateCheckURL();

        // QNAM is non-blocking / non-synchronous, but we want to wait until reply has
        // been received
        // create custom temporary event loop on stack to block the stack until
        // finished received
        QEventLoop eventLoop;

        // "quit()" the event-loop, when the network request "finished()"
        QObject::connect(&network, SIGNAL(finished(QNetworkReply *)), &eventLoop,
                         SLOT(quit()));

        // the HTTP request
        QNetworkRequest req(url);
        QNetworkReply *updateCheckResponse = network.get(req);

        // run event loop, which blocks the stack, until "finished()" has been called
        eventLoop.exec();

        QJsonDocument jsonResponse;

        if (updateCheckResponse->error() == QNetworkReply::NoError) {

            // read response and parse JSON
            QString strReply = (QString)updateCheckResponse->readAll();
            jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());
        } else {
            // QNetworkReply::HostNotFoundError
            qDebug() << "Request Failure: " << updateCheckResponse->errorString();

            QMessageBox::critical(QApplication::activeWindow(), "Request Failure",
                                  updateCheckResponse->errorString(), QMessageBox::Ok);
        }

        // cleanup
        delete updateCheckResponse;

        return jsonResponse.object();
    }

    QString SelfUpdater::getUpdateCheckURL()
    {
        QString url("http://wpn-xm.org/updatecheck.php");

        // software
        url.append("?s=");
        if (WindowsAPI::QtWin::running_on_64_bits_os()) {
            url.append("wpnxmscp-x64");
        } else {
            url.append("wpnxmscp");
        }

        // version
        url.append("&v=");
        QString version(APP_VERSION_SHORT);
        if (version != "@APPVERSIONSHORT@") {
            url.append(version);
        } else {
            url.append(
                "0.8.4"); // hardcoded for local testing (formerly version_localdev.h)
        }

        return url;
    }
}
