#include "registrymanager.h"

namespace SoftwareRegistry
{
    Manager::Manager() { download(); }

    void Manager::download()
    {
        /**
   * foreach registry
   *   if (not JSON file exists) and (JSON file not older than updateInterval)
   *     fetch API data as JSON
   *     write to JSON file
   */

        // create "/bin/wpnxm-scp" folder, if not exists
        QDir dir(QDir::currentPath() + "/bin/wpnxm-scp");
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        /**
   * Server Stack Software Registry
   */
        QString stackRegistryFile =
            QDir::currentPath() + "/bin/wpnxm-scp/stack-registry.json";

        if (fileNotExistingOrOutdated(stackRegistryFile)) {
            downloadRegistry(QUrl("http://wpn-xm.org/updatecheck.php?s=all"),
                             stackRegistryFile);
        } else {
            qDebug() << "[Loading from Cache] Server Stack Software Registry";
            stackSoftwareRegistry = File::JSON::load(stackRegistryFile);
        }

        /**
   * PHP Application Registry
   */

        // TODO download PHP Application Registry

        /**
   * Registry Metadata
   */

        // TODO download Registry Metadata
    }

    void Manager::downloadRegistry(QUrl url, QString file)
    {
        QNetworkAccessManager network;

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

        if (updateCheckResponse->error() == QNetworkReply::NoError) {

            // read response and parse JSON
            QString strReply = (QString)updateCheckResponse->readAll();
            QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8());

            // save JSON to file
            File::JSON::save(jsonResponse, file);
        } else {
            // QNetworkReply::HostNotFoundError
            qDebug() << "Request Failure: " << updateCheckResponse->errorString();

            QMessageBox::critical(QApplication::activeWindow(), "Request Failure",
                                  updateCheckResponse->errorString(), QMessageBox::Ok);
        }

        // cleanup pointer
        delete updateCheckResponse;
    }

    bool Manager::fileNotExistingOrOutdated(QString fileName)
    {
        // if the file doesn't exist, we need to update
        if (!QFile::exists(fileName)) {
            return true;
        }

        // if the file exists, we need to check if it is old
        QDateTime fileModificationDate = QFileInfo(fileName).lastModified();
        QDateTime today = QDateTime::currentDateTime();

        // updateInterval hardcoded to 3 days
        if (3 <= fileModificationDate.daysTo(today)) {
            return true;
        }

        return false;
    }

    QJsonObject Manager::getServerStackSoftwareRegistry()
    {
        return stackSoftwareRegistry.object();
    }

    /*
QJsonObject RegistriesDownloader::getPhpSoftwareRegistry()
{
    return phpSoftwareRegistry.object();
}*/
}
