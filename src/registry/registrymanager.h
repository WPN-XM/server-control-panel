#ifndef REGISTRIESDOWNLOADER_H
#define REGISTRIESDOWNLOADER_H

#include "src/json.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <QJsonDocument>
#include <QJsonObject>

#include <QApplication>
#include <QMessageBox>

namespace SoftwareRegistry
{
    class Manager
    {
    public:
        Manager();
        QJsonObject getServerStackSoftwareRegistry();
        // QJsonObject getPhpSoftwareRegistry();
    private:
        void download();
        void downloadRegistry(QUrl url, QString file);
        bool fileNotExistingOrOutdated(QString fileName);

    protected:
        QJsonDocument stackSoftwareRegistry;
        // QJsonDocument phpSoftwareRegistry;
    };
}

#endif // REGISTRIESDOWNLOADER_H
