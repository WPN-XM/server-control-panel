#ifndef SERVERS_H
#define SERVERS_H

#include <QApplication>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QProcess>
#include <QTimer>

#include <QJsonDocument>
#include <QJsonObject>

#include "file/filehandling.h"
#include "file/json.h"
#include "file/ini.h"
#include "file/yml.h"
#include "settings.h"
#include "processviewer/processes.h"

namespace Servers
{
    class Server : public QObject
    {
        Q_OBJECT

    public:
        QString lowercaseName;
        QString name;
        QIcon icon;
        QString workingDirectory;
        QStringList logFiles;
        QString exe;

        QMenu *trayMenu;
    };

    class Servers : public QObject
    {
        Q_OBJECT

    public:
        Servers(QObject *parent = nullptr);
        Servers(Processes *processes, QObject *parent = nullptr);

        Processes *processes;
        Settings::SettingsManager *settings;

        QList<Server *> servers() const;
        QStringList getListOfServerNames() const;
        QStringList getInstalledServerNames();
        bool isInstalled(const QString &serverName);
        QString getCamelCasedServerName(QString &serverName) const;
        Server *getServer(const QString &serverName) const;
        QString getExecutable(QString &serverName) const;
        QString getExecutablePath(QString &serverName) const;
        QString getServerBinPath(QString &serverName) const;

        QStringList getLogFiles(const QString &serverName) const;
        void clearLogFile(const QString &serverName) const;

        QVariantMap getPHPServersFromNginxUpstreamConfig();

        QString getMongoPort();

    public slots:

        // Status Action Slots
        // void updateProcessStates(Processes::ProcessState state);

        // This slot action handles clicks on server commands in the tray menu.
        void mapAction(QAction *action);

        // Nginx Action Slots
        void startNginx();
        void stopNginx();
        void reloadNginx();
        void restartNginx();

        // PHP Action Slots
        void startPHP();
        void stopPHP();
        void restartPHP();

        // MariaDb Action Slots
        void startMariaDb();
        void stopMariaDb();
        void restartMariaDb();

        // MongoDB Action Slots
        void startMongoDb();
        void stopMongoDb();
        void restartMongoDb();

        // Memcached Action Slots
        void startMemcached();
        void stopMemcached();
        void restartMemcached();

        // PostgreSQL Action Slots
        void startPostgreSQL();
        void stopPostgreSQL();
        void restartPostgreSQL();

        // Redis Action Slots
        void startRedis();
        void stopRedis();
        void restartRedis();

    signals:
        void signalMainWindow_ServerStatusChange(QString label, bool enabled);

    private:
        QList<Server *> serverList;
    };
} // namespace Servers
#endif // SERVERS_H
