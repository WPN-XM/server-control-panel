#include "servers.h"

#include <QDebug>

namespace Servers
{
    Servers::Servers(QObject *parent) : QObject(parent), settings(new Settings::SettingsManager)
    {
        QStringList installedServers = getInstalledServerNames();

        qDebug() << "[Servers] Create Server objects and tray submenus for installed servers.";

        // build server objects
        foreach (QString serverName, installedServers) {
            Server *server = new Server();

            server->lowercaseName    = serverName;
            server->name             = getCamelCasedServerName(serverName);
            server->icon             = QIcon(":/status_stop");
            server->logFiles         = getLogFiles(serverName);
            server->workingDirectory = getServerBinPath(serverName);
            server->exe              = getExecutablePath(server->name);

            QMenu *menu = new QMenu(server->name);
            menu->setObjectName(QString("menu").append(server->name));
            menu->setIcon(server->icon);

            QAction *restartAction = new QAction(QIcon(":/action_restart"), tr("Restart"), menu);
            QAction *startAction   = new QAction(QIcon(":/action_run"), tr("Start"), menu);
            QAction *stopAction    = new QAction(QIcon(":/action_stop"), tr("Stop"), menu);

            restartAction->setObjectName(QString("restart").append(server->name));
            startAction->setObjectName(QString("start").append(server->name));
            stopAction->setObjectName(QString("stop").append(server->name));

            menu->addAction(restartAction);
            menu->addSeparator();
            menu->addAction(startAction);
            menu->addAction(stopAction);

            // connect ONLY ONCE
            connect(menu, SIGNAL(triggered(QAction *)), this, SLOT(mapAction(QAction *)), Qt::UniqueConnection);

            server->trayMenu = menu;

            serverList << server;

            qDebug() << "[Servers] Server object added to serverList:\t" << serverName;
        }
    }

    void Servers::mapAction(QAction *action)
    {
        QMetaObject::invokeMethod(this, action->objectName().toLocal8Bit().constData());
    }

    /**
     * @brief Servers::getCamelCasedServerName
     * translates lowercase server and process names to internally used camel-cased
     * label names
     * @param serverName
     * @return
     */
    QString Servers::getCamelCasedServerName(QString &serverName) const
    {
        if (serverName == "nginx") {
            return "Nginx";
        }
        if (serverName == "mariadb" || serverName == "mysqld") {
            return "MariaDb";
        }
        if (serverName == "php" || serverName == "php-cgi") {
            return "PHP";
        }
        if (serverName == "memcached") {
            return "Memcached";
        }
        if (serverName == "mongodb" || serverName == "mongod") {
            return "MongoDb";
        }
        if (serverName == "postgresql" || serverName == "postgres") {
            return "PostgreSQL";
        }
        if (serverName == "redis" || serverName == "redis-server") {
            return "Redis";
        }

        return QString("Unknown");
    }

    QStringList Servers::getLogFiles(const QString &serverName) const
    {
        QString s = serverName.toLower();

        QString logsDir = QDir(settings->getString("paths/logs")).absolutePath();

        QStringList logfiles;

        if (s == "nginx") {
            logfiles << logsDir + "/error.log" << logsDir + "/access.log";
        }
        if (s == "memcached") {
            logfiles << "";
        }
        if (s == "mongodb") {
            logfiles << logsDir + "/mongodb.log";
        }
        if (s == "mariadb") {
            logfiles << logsDir + "/mariadb_error.log";
        }
        if (s == "php") {
            logfiles << logsDir + "/php_error.log";
        }
        if (s == "postgresql") {
            logfiles << logsDir + "/postgresql.log";
        }
        if (s == "redis") {
            logfiles << logsDir + "/redis.log";
        }

        return logfiles;
    }

    QString Servers::getExecutable(QString &serverName) const
    {
        QString s = serverName.toLower();
        QString exe;
        if (s == "nginx") {
            exe = "nginx.exe";
        }
        if (s == "memcached") {
            exe = "memcached.exe";
        }
        if (s == "mongodb") {
            exe = "mongod.exe";
        }
        if (s == "mariadb") {
            exe = "mysqld.exe";
        }
        if (s == "php") {
            exe = "php-cgi.exe";
        }
        if (s == "postgresql") {
            exe = "pg_ctl.exe";
        }
        if (s == "redis") {
            exe = "redis-server.exe";
        }
        return exe;
    }

    QString Servers::getExecutablePath(QString &serverName) const
    {
        QString filepath = getServerBinPath(serverName) + getExecutable(serverName);

        return QDir::toNativeSeparators(filepath);
    }

    QString Servers::getServerBinPath(QString &serverName) const
    {
        QString s = serverName.toLower();
        QString path;
        if (s == "nginx") {
            path = QDir::currentPath() + "/bin/nginx/";
        }
        if (s == "memcached") {
            path = QDir::currentPath() + "/bin/memcached/";
        }
        if (s == "mongodb") {
            path = QDir::currentPath() + "/bin/mongodb/bin/";
        }
        if (s == "mariadb") {
            path = QDir::currentPath() + "/bin/mariadb/bin/";
        }
        if (s == "php") {
            path = QDir::currentPath() + "/bin/php/";
        }
        if (s == "postgresql") {
            path = QDir::currentPath() + "/bin/pgsql/bin/";
        }
        if (s == "redis") {
            path = QDir::currentPath() + "/bin/redis/";
        }

        return QDir::toNativeSeparators(path);
    }

    QStringList Servers::getListOfServerNames() const
    {
        QStringList list;
        list << "nginx"
             << "php"
             << "mariadb"
             << "mongodb"
             << "memcached"
             << "postgresql"
             << "redis";
        return list;
    }

    QStringList Servers::getInstalledServerNames()
    {
        qDebug() << "[Servers] Check, which servers are installed.";

        QStringList list;
        foreach (QString serverName, getListOfServerNames()) {
            // these three servers are the base package.
            // we assume that they are always installed.
            // this is also for testing, because they appear installed, even if they are not.
            if (serverName == "nginx" || serverName == "php" || serverName == "mariadb" ||
                QFile::exists(getExecutablePath(serverName))) {
                qDebug() << "Installed:\t" << serverName;
                list << serverName;
            } else {
                qDebug() << "Not Installed:\t" << serverName;
            }
        }
        return list;
    }

    bool Servers::isInstalled(const QString &searchServerName)
    {
        qDebug() << "[Servers] isInstalled?" << searchServerName;

        QStringList installedServers = getInstalledServerNames();

        foreach (QString serverName, installedServers) {
            if (searchServerName == serverName) {
                qDebug() << "Installed:\t" << serverName;
                return true;
            }
        }

        return false;
    }

    QList<Server *> Servers::servers() const { return serverList; }

    Server *Servers::getServer(const QString &serverName) const
    {
        foreach (Server *server, serverList) {
            if (server->name == serverName)
                return server;
        }

        // anti "control reaches end of non-void function" faked return
        Server *server = new Server();
        server->name   = QString("Not Installed");
        return server;
    }

    void Servers::clearLogFile(const QString &serverName) const
    {
        if (settings->getBool("global/clearlogsonstart")) {
            QStringList logfiles = getLogFiles(serverName);

            foreach (QString logfile, logfiles) {
                File::truncate(logfile);
                qDebug() << ("[ %s ][ %s ] Log was cleared.\n", serverName, logfile);
            }
        }
    }

    // Server "Start - Stop - Restart" Methods

    /*
     * Nginx - Actions: run, stop, restart
     */
    void Servers::startNginx()
    {
        // already running
        if (processes->getProcessState(getServer("Nginx")->exe) == Processes::ProcessUtil::ProcessState::Running) {
            QMessageBox::warning(nullptr, tr("Nginx"), tr("Nginx already running."));
            return;
        }

        clearLogFile("Nginx");

        // http://wiki.nginx.org/CommandLine - start Nginx
        QString program = getServer("Nginx")->exe;

        QStringList arguments;
        arguments << "-p " + QDir::currentPath();
        arguments << "-c " + QDir::currentPath() + "/bin/nginx/conf/nginx.conf";

        qDebug() << "[Nginx] Starting...\n";

        Processes::ProcessUtil::startDetached(program, arguments, getServer("Nginx")->workingDirectory);

        Processes::ProcessUtil::delay(250);

        // catch startup failures
        if (processes->getProcessState("nginx.exe") == Processes::ProcessUtil::ProcessState::Running) {
            emit signalMainWindow_ServerStatusChange("Nginx", true);
        } else {
            emit signalMainWindow_ServerStatusChange("Nginx", false);
        }
    }

    void Servers::stopNginx()
    {
        // if not running, skip
        if (processes->getProcessState("nginx.exe") == Processes::ProcessUtil::ProcessState::NotRunning) {
            qDebug() << "[Nginx] Not running... Skipping stop command.";
            emit signalMainWindow_ServerStatusChange("Nginx", false);
            return;
        }

        qDebug() << "[Nginx] Stopping...\n";

        // The native stop command doesn't work on Windows.
        // http://wiki.nginx.org/CommandLine "-s stop"
        //
        // It fails with "nginx: [error] CreateFile()"
        // We could catch that string and force a process kill.
        // The bug is known since years - no fix incomming.
        //
        // I've given up on this shit.
        // Let's just do a  process multi kill.
        // Fuck off. Seriously.
        //

        /*QString serverName = "nginx";
        const QString stopNginx = getExecutable(serverName);
        QStringList args;
        args << "-p " + QDir::currentPath();
        args << "-c " + QDir::currentPath() + "/bin/nginx/conf/nginx.conf";
        args << "-s stop";
        Processes::start(stopNginx, args, getServer("Nginx")->workingDirectory);

        QProcess process;
        process.start(stopNginx, args);
        process.waitForFinished();

        // because stopping might fail with "nginx: [error] CreateFile()"
        // we need to catch it and force the shutdown with a process kill
        QByteArray p_stdout = process.readLine();
        qDebug() << p_stdout;
        if(p_stdout.contains("nginx: [error] CreateFile()") == true) {
*/

        // you know what: process multi kill. fuck off.
        while (processes->getProcessState("nginx.exe") == Processes::ProcessUtil::ProcessState::Running) {
            qDebug() << "[Nginx] Stopped using process kill!";
            processes->killProcessTree("nginx.exe");
            processes->delay(100);
        }

        emit signalMainWindow_ServerStatusChange("Nginx", false);
    }

    void Servers::reloadNginx()
    {
        QString const reloadNginx = getServer("Nginx")->exe;

        QStringList args;
        args << "-p " + QDir::currentPath();
        args << "-c " + QDir::currentPath() + "/bin/nginx/conf/nginx.conf";
        args << "-s reload";

        qDebug() << "[Nginx] Reloading...\n" << reloadNginx;

        Processes::ProcessUtil::startDetached(reloadNginx, args, getServer("Nginx")->workingDirectory);
    }

    void Servers::restartNginx()
    {
        stopNginx();
        startNginx();
    }

    /*
     * PostgreSQL Actions: run, stop, restart
     */
    void Servers::startPostgreSQL()
    {
        // if not installed, skip
        if (!QFile::exists(QDir::currentPath() + "/bin/pgsql/bin/pg_ctl.exe")) {
            qDebug() << "Not found: " + QDir::currentPath() + "/bin/pgsql/bin/pg_ctl.exe";
            qDebug() << "[PostgreSQL] Is not installed. Skipping start command.";
            return;
        }

        // already running
        if (processes->getProcessState("postgres.exe") == Processes::ProcessUtil::ProcessState::Running) {
            // if(QFile::exists(QDir::currentPath() +
            // "/bin/pgsql/data/postmaster.pid")) {
            QMessageBox::warning(0, tr("PostgreSQL"), tr("PostgreSQL is already running."));
            return;
        }

        clearLogFile("PostgreSQL");

        QString const startCmd = QDir::toNativeSeparators(QDir::currentPath() + "/bin/pgsql/bin/pg_ctl.exe");

        QStringList args;
        args << "--pgdata " + QDir::toNativeSeparators(QDir::currentPath() + "/bin/pgsql/data");
        args << "--log " + QDir::toNativeSeparators(QDir::currentPath() + "/logs/postgresql.log");
        args << "start";

        qDebug() << "[PostgreSQL] Starting...\n" << startCmd;

        Processes::ProcessUtil::start(startCmd, args, getServer("PostgreSQL")->workingDirectory);

        emit signalMainWindow_ServerStatusChange("PostgreSQL", true);
    }

    void Servers::stopPostgreSQL()
    {
        Server *server = getServer("PostgreSQL");

        // if not installed, skip
        if (!QFile::exists(server->exe)) {
            qDebug() << "[PostgreSQL] Is not installed. Skipping stop command.";
            return;
        }

        // if not running, skip
        QString file = QDir::toNativeSeparators(QDir::currentPath() + "/bin/pgsql/data/postmaster.pid");
        if (processes->getProcessState("postgres.exe") == Processes::ProcessUtil::ProcessState::NotRunning) {
            // if(!QFile::exists(file)) {
            qDebug() << "[PostgreSQL] Not running.. Skipping stop command.";
            emit signalMainWindow_ServerStatusChange("PostgreSQL", false);
            return;
        }

        QString stopCommand = QDir::toNativeSeparators(QDir::currentPath() + "/bin/pgsql/bin/pg_ctl.exe");

        QStringList args;
        args << "stop";
        args << "--pgdata " + QDir::toNativeSeparators(QDir::currentPath() + "/bin/pgsql/data");
        args << "--log " + QDir::toNativeSeparators(QDir::currentPath() + "/logs/postgresql.log");
        args << "--mode=fast";
        args << "-W";

        qDebug() << "[PostgreSQL] Stopping...";

        Processes::ProcessUtil::start(stopCommand, args, getServer("PostgreSQL")->workingDirectory);

        Processes::ProcessUtil::delay(1250); // delay PID file check, PostgreSQL must shutdown first

        // do we have a failed shutdown? if so, delete PID file, to allow a restart
        if (QFile::exists(file)) {
            qDebug() << "[PostgreSQL] PID file exists. Removing PID file to allow a "
                        "restart.";
            QFile::remove(file);
        }

        emit signalMainWindow_ServerStatusChange("PostgreSQL", false);
    }

    void Servers::restartPostgreSQL()
    {
        stopPostgreSQL();
        startPostgreSQL();
    }

    /*
     * PHP - Actions: run, stop, restart
     */
    void Servers::startPHP()
    {
        // already running: PHP
        if (processes->getProcessState("php-cgi.exe") == Processes::ProcessUtil::ProcessState::Running) {
            QMessageBox::warning(nullptr, tr("PHP"), tr("PHP is already running."));
            return;
        }

        // already running: Spawner
        if (processes->getProcessState("php-cgi-spawner.exe") == Processes::ProcessUtil::ProcessState::Running) {
            QMessageBox::warning(0, tr("PHP"), tr("Process Spawner for PHP is already running (php-cgi-spawner.exe)."));
            return;
        }

        clearLogFile("PHP");

        // disable PHP_FCGI_MAX_REQUESTS
        // to go beyond the default request limit of 500 requests
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PHP_FCGI_MAX_REQUESTS", "0");
        qDebug() << "[PHP] Set ENV:PHP_FCGI_MAX_REQUESTS \"0\" (disabled).";

        // get the PHP version
        QProcess process;
        process.start(QDir::currentPath() + "./bin/php/php.exe -n -r \"echo PHP_VERSION_ID;\"");
        process.waitForFinished();
        int phpVersion = process.readLine().toInt();
        qDebug() << "[PHP] Version " << phpVersion;

        // check that the tool "php-cgi-spawner" is present
        QString spawnUtilFile =
            QDir::toNativeSeparators(QDir::currentPath() + "/bin/php-cgi-spawner/php-cgi-spawner.exe");
        if (!QFile::exists(spawnUtilFile)) {
            qDebug() << "[PHP] Starting PHP failed. Tool \"php-cgi-spawner.exe\" missing.";
            return;
        }

        // get the nginx upstream configuration and read the defined PHP pools
        QVariantMap PHPServersToStart(getPHPServersFromNginxUpstreamConfig());

        auto end = PHPServersToStart.cend();
        for (auto item = PHPServersToStart.cbegin(); item != end; ++item) {
            QString poolName = item.key();

            // item.value = QMap (port, num childs)
            QMap<QString, QVariant> m = item.value().toMap();
            auto end2                 = m.cend();
            for (auto item2 = m.cbegin(); item2 != end2; ++item2) {
                QString port        = item2.key();
                QString phpchildren = item2.value().toString();

                QString startPHPCGI;

                // if PHP 7.1+, then use env var PHP_FCGI_CHILDREN to allow PHP spawning childs
                if (phpVersion >= 70100) {
                    env.insert("PHP_FCGI_CHILDREN", phpchildren);
                    qDebug() << "[PHP] Set ENV:PHP_FCGI_CHILDREN " << phpchildren;

                    QString startCmdStringTemplate(QDir::currentPath() + "/bin/php/php-cgi.exe -b 127.0.0.1:%1");
                    startPHPCGI = startCmdStringTemplate.arg(port);
                } else {
                    // if PHP version below 7.1, use "php-cgi-spawner.exe" to spawn multiple processes
                    QString startCmdStringTemplate(spawnUtilFile + " ./bin/php/php-cgi.exe %1 %2");
                    startPHPCGI = startCmdStringTemplate.arg(port, phpchildren);
                }

                qDebug() << "[PHP] Starting...";
                qDebug() << "        Pool:" << poolName;
                qDebug() << "        php-cgi:" << startPHPCGI;

                QProcess *process = new QProcess;
                process->setEnvironment(env.toStringList());
                process->startDetached(startPHPCGI);
                qDebug() << "        Process PID:" << process->pid() << "\n";
            }
        }

        emit signalMainWindow_ServerStatusChange("PHP", true);
    }

    QVariantMap Servers::getPHPServersFromNginxUpstreamConfig()
    {
        QVariantMap serversToStart; // QString port, QString childs
        QVariantMap serversToStartOuter; // QString poolname, serversToStart

        QString nginxUpstreamsFile = "./bin/wpnxm-scp/nginx-upstreams.json";

        QFile file(nginxUpstreamsFile);
        if (!file.exists()) {
            qDebug() << "[PHP] Nginx Upstream Configuration file not found.\n";
        }

        // load JSON
        QJsonDocument jsonDoc = File::JSON::load(nginxUpstreamsFile);
        QJsonObject json      = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools
        for (QJsonObject::Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter) {
            // get values for a "pool", we need the key "servers"
            QJsonObject jsonPool    = iter.value().toObject();
            QJsonObject jsonServers = jsonPool["servers"].toObject();
            QString poolName        = jsonPool["name"].toString();

            // iterate over 1..n jsonServers
            for (int i = 0; i < jsonServers.count(); ++i) {
                // get values for a "server"
                QJsonObject s = jsonServers.value(QString::number(i)).toObject();

                // we want to start local servers only.
                // external servers are possible, but the user has to start (and take
                // care) of them.
                if ((s["address"].toString() == "localhost") || (s["address"].toString() == "127.0.0.1")) {

                    // for starting local servers, we only need the Port and the number of
                    // PHP child processes to spawn
                    serversToStart.insert(s["port"].toString(), s["phpchildren"].toString());
                }
            }

            serversToStartOuter.insert(poolName, serversToStart);
        }

        if (serversToStart.count() == 0) {
            qDebug() << "[PHP] Found no servers to start. Check your upstream configuration.";
            qDebug() << "[PHP] At least one address must be localhost or 127.0.0.1.";
        }

        return serversToStartOuter;
    }

    void Servers::stopPHP()
    {
        // if not installed, skip
        if (!QFile::exists(getServer("PHP")->exe)) {
            qDebug() << "[PHP] Is not installed. Skipping stop command.";
            return;
        }

        // if not running, skip
        if (processes->getProcessState("php-cgi.exe") == Processes::ProcessUtil::ProcessState::NotRunning) {
            qDebug() << "[PHP] Not running... Skipping stop command.";
            emit signalMainWindow_ServerStatusChange("PHP", false);
            return;
        }

        qDebug() << "[PHP] Stopping...";

        /**
         * There is only one way stop the PHP server:
         * By terminating the process. That means we are crashing it!
         *
         * WARNING!
         *
         * The order is important.
         * The php-cgi-spawner needs to be killed before we are trying to kill PHP childs.
         *
         */

        while (processes->getProcessState("php-cgi-spawner.exe") == Processes::ProcessUtil::ProcessState::Running) {
            processes->killProcessTree("php-cgi-spawner.exe");
            processes->delay(100);
        }

        while (processes->getProcessState("php-cgi.exe") == Processes::ProcessUtil::ProcessState::Running) {
            processes->killProcessTree("php-cgi.exe");
            processes->delay(100);
        }

        emit signalMainWindow_ServerStatusChange("PHP", false);
    }

    void Servers::restartPHP()
    {
        stopPHP();
        startPHP();
    }

    /*
     * MariaDb Actions - run, stop, restart
     */
    void Servers::startMariaDb()
    {
        // if already running, skip
        if (processes->getProcessState("mysqld.exe") == Processes::ProcessUtil::ProcessState::Running) {
            QMessageBox::warning(nullptr, tr("MariaDB"), tr("MariaDB already running."));
            return;
        }

        clearLogFile("MariaDb");

        QString const startMariaDb = getServer("MariaDb")->exe;

        QStringList args;
        args << "--defaults-file=" + QDir::toNativeSeparators(QDir::currentPath() + "/bin/mariadb/my.ini");
        args << "--standalone"; // important! else maria is started as service and
        // won't detach

        qDebug() << "[MariaDB] Starting...\n";

        Processes::ProcessUtil::startDetached(startMariaDb, args, getServer("MariaDb")->workingDirectory);

        // wait for process started
        Processes::ProcessUtil::delay(500);

        // check for startup failure (immediate shutdown)
        if (processes->getProcessState("mysqld.exe") == Processes::ProcessUtil::ProcessState::Running) {
            emit signalMainWindow_ServerStatusChange("MariaDb", true);
        } else {
            emit signalMainWindow_ServerStatusChange("MariaDb", false);
        }
    }

    void Servers::stopMariaDb()
    {
        // if not installed, skip
        if (!QFile::exists(getServer("MariaDb")->exe)) {
            qDebug() << "[MariaDb] Is not installed. Skipping stop command.";
            return;
        }

        // if not running, skip
        if (processes->getProcessState("mysqld.exe") == Processes::ProcessUtil::ProcessState::NotRunning) {
            qDebug() << "[MariaDb] Not running... Skipping stop command.";
            emit signalMainWindow_ServerStatusChange("MariaDb", false);
            return;
        }

        // shutdown of "mysqld" is done by using "mysqladmin" + shutdown command
        QString stopCommand = QDir::toNativeSeparators(QDir::currentPath() + "/bin/mariadb/bin/mysqladmin.exe");

        // check, if mysqladmin.exe is present
        if (!QFile::exists(stopCommand)) {
            qDebug() << "[MariaDb] Can not stop. Missing mysqladmin.exe.";
            return;
        }

        // "mysqladmin" needs the password from the config file "my.ini"
        QString configFile = QDir::toNativeSeparators(QDir::currentPath() + "/bin/mariadb/my.ini");

        // read the password from "my.ini"
        File::INI *ini   = new File::INI(configFile.toLatin1());
        QString password = ini->getStringValue("client", "password");
        delete ini;

        // finally, build arguments for a "mysqladmin" shutdown run
        QStringList args;
        args << "--defaults-file=" + configFile;
        args << "-u root";
        args << "-p" + password;
        args << "shutdown";

        qDebug() << "[MariaDB] Stopping...";

        Processes::ProcessUtil::start(stopCommand, args, getServer("MariaDb")->workingDirectory);

        Processes::ProcessUtil::delay(1000);

        emit signalMainWindow_ServerStatusChange("MariaDb", false);
    }

    void Servers::restartMariaDb()
    {
        stopMariaDb();
        startMariaDb();
    }

    /*
     * MongoDb Actions - start, stop, restart
     */
    void Servers::startMongoDb()
    {
        // if not installed, skip
        if (!QFile::exists(getServer("MongoDb")->exe)) {
            qDebug() << "[MongoDb] Is not installed. Skipping start command.";
            return;
        }

        // if already running, skip
        if (processes->getProcessState("mongod.exe") == Processes::ProcessUtil::ProcessState::Running) {
            QMessageBox::warning(nullptr, tr("MongoDb"), tr("MongoDb already running."));
            return;
        }

        // MongoDb doesn't start, when data dir is missing...
        QString const mongoDbDataDir = QDir::currentPath() + "/bin/mongodb/data/db";
        if (QDir().exists(QDir::currentPath() + "/bin/mongodb") && !QDir().exists(mongoDbDataDir)) {
            qDebug() << "[MongoDb] Creating Directory for Mongo's Database...\n" << mongoDbDataDir;
            QDir().mkpath(mongoDbDataDir);
        }

        // MongoDb doesn't start, when logfile is missing...
        QFile f(QDir::currentPath() + "/logs/mongodb.log");
        if (!f.exists()) {
            qDebug() << "[MongoDb] Creating empty logfile...\n" << QDir::currentPath() + "/logs/mongodb.log";
            f.open(QIODevice::ReadWrite);
            f.close();
        } else {
            clearLogFile("MongoDb");
        }

        // build mongo start command
        QString const mongoStartCommand = getServer("MongoDb")->exe;

        QStringList args;
        args << "--config " + QDir::currentPath() + "/bin/mongodb/mongod.conf";

        qDebug() << "[MongoDb] Starting...\n";

        Processes::ProcessUtil::startDetached(mongoStartCommand, args, getServer("MongoDb")->workingDirectory);

        // catch startup failures
        if (processes->getProcessState("mongod.exe") == Processes::ProcessUtil::ProcessState::Running) {
            emit signalMainWindow_ServerStatusChange("MongoDb", true);
        } else {
            emit signalMainWindow_ServerStatusChange("MongoDb", false);
        }
    }

    void Servers::stopMongoDb()
    {
        // if not installed, skip
        if (!QFile::exists(getServer("MongoDb")->exe)) {
            qDebug() << "[MongoDb] Is not installed. Skipping stop command.";
            return;
        }

        // if not running, skip
        if (processes->getProcessState("mongod.exe") == Processes::ProcessUtil::ProcessState::NotRunning) {
            qDebug() << "[MongoDb] Not running... Skipping stop command.";
            emit signalMainWindow_ServerStatusChange("MongoDb", false);
            return;
        }

        /**
         * build mongo stop command based on CLI evaluation
         * mongodb is stopped via "mongo.exe --eval", not "mongodb.exe"
         */

        QString const mongoStopCommand = QDir::currentPath() + "/bin/mongodb/bin/mongo.exe";

        QStringList args;
        args << "--port " + getMongoPort();
        args << "--eval \"db.getSiblingDB('admin').shutdownServer()\"";

        qDebug() << "[MongoDb] Stopping...\n";

        Processes::ProcessUtil::start(mongoStopCommand, args, getServer("MongoDb")->workingDirectory);

        Processes::ProcessUtil::delay(1000);

        emit signalMainWindow_ServerStatusChange("MongoDb", false);
    }

    void Servers::restartMongoDb()
    {
        stopMongoDb();
        startMongoDb();
    }

    /*
     * Memcached Actions - run, stop, restart
     */
    void Servers::startMemcached()
    {

        // https://github.com/memcached/memcached/wiki/ConfiguringServer#commandline-arguments

        QString const memcachedStartCommand = getServer("Memcached")->exe;

        QStringList args;
        args << "-p " + settings->getString("memcached/tcpport");
        args << " -U " + settings->getString("memcached/udpport");
        args << " -t " + settings->getString("memcached/threads");
        args << " -c " + settings->getString("memcached/maxconnections");
        args << " -m " + settings->getString("memcached/maxmemory");

        // if not installed, skip
        if (!QFile::exists(getServer("Memcached")->exe)) {
            qDebug() << "[Memcached] Is not installed. Skipping start command.";
            return;
        }

        // if already running, skip
        if (processes->getProcessState("memcached.exe") == Processes::ProcessUtil::ProcessState::Running) {
            QMessageBox::warning(nullptr, tr("Memcached"), tr("Memcached already running."));
            return;
        }

        qDebug() << "[Memcached] Starting...\n";

        Processes::ProcessUtil::startDetached(memcachedStartCommand, args, getServer("Memcached")->workingDirectory);

        emit signalMainWindow_ServerStatusChange("Memcached", true);
    }

    void Servers::stopMemcached()
    {
        // if not installed, skip
        if (!QFile::exists(getServer("Memcached")->exe)) {
            qDebug() << "[Memcached] Is not installed. Skipping stop command.";
            return;
        }

        // if not running, skip
        if (processes->getProcessState("memcached.exe") == Processes::ProcessUtil::ProcessState::NotRunning) {
            qDebug() << "[Memcached] Not running... Skipping stop command.";
            emit signalMainWindow_ServerStatusChange("Memcached", false);
            return;
        }

        // Memcached is a fucked up application.
        // It doesn't provide a shutdown command.
        // And needs to be process killed... WTF.

        qDebug() << "[Memcached] Stopping...\n";

        Processes::ProcessUtil::killProcess("memcached.exe");

        emit signalMainWindow_ServerStatusChange("Memcached", false);
    }

    void Servers::restartMemcached()
    {
        stopMemcached();
        startMemcached();
    }

    void Servers::startRedis()
    {
        QString const redisStartCommand = getServer("Redis")->exe;

        QStringList args;
        args << "redis.windows.conf";

        // if not installed, skip
        if (!QFile::exists(getServer("Redis")->exe)) {
            qDebug() << "[Redis] Is not installed. Skipping start command.";
            return;
        }

        // if already running, skip
        if (processes->getProcessState("redis-server.exe") == Processes::ProcessUtil::ProcessState::Running) {
            QMessageBox::warning(nullptr, tr("Redis"), tr("Redis already running."));
            return;
        }

        clearLogFile("Redis");

        qDebug() << "[Redis] Starting...\n" << redisStartCommand;

        Processes::ProcessUtil::startDetached(redisStartCommand, args, getServer("Redis")->workingDirectory);

        emit signalMainWindow_ServerStatusChange("Redis", true);
    }

    void Servers::stopRedis()
    {
        // if not installed, skip
        if (!QFile::exists(getServer("Redis")->exe)) {
            qDebug() << "[Redis] Is not installed. Skipping stop command.";
            return;
        }

        // if not running, skip
        if (processes->getProcessState("redis-server.exe") == Processes::ProcessUtil::ProcessState::NotRunning) {
            qDebug() << "[Redis] Not running... Skipping stop command.";
            emit signalMainWindow_ServerStatusChange("Redis", false);
            return;
        }

        // build redis stop command
        // Note: "-a password" is not supported, yet
        QString const redisStopCommand = QDir::currentPath() + "/bin/redis/redis-cli.exe";

        QStringList args;
        args << "-h " + settings->getString("redis/bind", QVariant("127.0.0.1"));
        args << "-p " + settings->getString("redis/port"); // , QVariant(QString('6379'))
        args << "shutdown";

        qDebug() << "[Redis] Stopping...\n";

        Processes::ProcessUtil::start(redisStopCommand, args, getServer("Redis")->workingDirectory);

        emit signalMainWindow_ServerStatusChange("Redis", false);
    }

    void Servers::restartRedis()
    {
        stopRedis();
        startRedis();
    }

    QString Servers::getMongoPort()
    {
        QString file = QDir(settings->getString("mongodb/config")).absolutePath();

        if (!QFile(file).exists()) {
            qDebug() << "[Error]" << file << "not found";
        }

        File::Yml *yml    = new File::Yml();
        YAML::Node config = yml->load(file);

        return QString::fromStdString(config["net"]["port"].as<std::string>());
    }
} // namespace Servers
