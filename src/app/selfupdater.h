#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include "settings.h"
#include "updater/downloadmanager.h"
#include "version.h"
#include "windowsapi.h"

#include "quazip/JlCompress.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>

namespace Updater
{
    class SelfUpdater : public QObject
    {
        Q_OBJECT

    public:
        explicit SelfUpdater(Settings::SettingsManager *oSettings);
        explicit SelfUpdater(Settings::SettingsManager &oSettings) = delete;
        ~SelfUpdater() override;

        void run();
        bool updateAvailable();
        void doUpdate();

        void downloadNewVersion();
        void renameExecutable();

        void askForRestart();

        enum Interval
        {
            Off     = 0,
            Daily   = 1,
            Weekly  = 7,
            Monthly = 30
        };

    public slots:
        void extract();
        void askForUpdate();

    private:
        QString getUpdateCheckURL();
        QJsonObject getVersionInfo();
        QJsonObject versionInfo;
        QString downloadFolder;
        bool userRequestedUpdate;

    protected:
        Downloader::DownloadManager downloadManager;
        QNetworkAccessManager network;
        Settings::SettingsManager *settings;

    signals:
        QJsonObject notifyUpdateAvailable(QJsonObject versionInfo);
        QJsonObject notifyRestartNeeded(QJsonObject versionInfo);
    };
} // namespace Updater

#endif // SELF_UPDATE_H
