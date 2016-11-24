#include "configurationdialog.h"
#include "ui_configurationdialog.h"

#include "nginxaddpooldialog.h"
#include "nginxaddserverdialog.h"
#include "../json.h"
#include "src/ini.h"

namespace Configuration
{
    ConfigurationDialog::ConfigurationDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ConfigurationDialog)
    {
        ui->setupUi(this);

        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        this->settings = new Settings::SettingsManager;
        readSettings();

        // setup autostart section
        hideAutostartCheckboxesOfNotInstalledServers();
        toggleAutostartDaemonCheckboxes(ui->checkbox_autostartDaemons->isChecked());
        connect(ui->checkbox_autostartDaemons, SIGNAL(clicked(bool)),
                this, SLOT(toggleAutostartDaemonCheckboxes(bool)));

        // load initial data for pages
        loadNginxUpstreams();

        connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onClickedButtonBoxOk()));

        ui->configMenuTreeWidget->expandAll();
    }

    ConfigurationDialog::~ConfigurationDialog()
    {
        delete ui;
    }

    /**
     * Search for items in the "Configuration Menu" TreeWidget
     *
     * @brief ConfigurationDialog::on_configMenuSearchLineEdit_textChanged
     * @param query
     */
    void ConfigurationDialog::on_configMenuSearchLineEdit_textChanged(const QString &query)
    {
        ui->configMenuTreeWidget->expandAll();

        // Iterate over all child items : filter items with "contains" query
        QTreeWidgetItemIterator iterator(ui->configMenuTreeWidget, QTreeWidgetItemIterator::All);
        while(*iterator)
        {
            QTreeWidgetItem *item = *iterator;
            if(item && item->text(0).contains(query, Qt::CaseInsensitive)) {
                item->setHidden(false);
            } else {
                // Problem: the matched child is visibile, but parent is hidden, because no match.
                // so, lets hide only items without childs.
                // any not matching parent will stay visible.. until next iteration, see below.
                if(item->childCount() == 0) {
                    item->setHidden(true);
                }
            }
            ++iterator;
        }

        // Iterate over items with childs : hide, if they do not have a matching (visible) child (see above).
        QTreeWidgetItemIterator parentIterator(ui->configMenuTreeWidget, QTreeWidgetItemIterator::HasChildren);
        while(*parentIterator)
        {
            QTreeWidgetItem *item = *parentIterator;
            // count the number of hidden childs
            int childs = item->childCount();
            int hiddenChilds = 0;
            for (int i = 0; i < childs; ++i) {
                if(item->child(i)->isHidden()) {
                    ++hiddenChilds;
                }
            }
            // finally: if all childs are hidden, hide the parent (*item), too
            if(hiddenChilds == childs) {
                item->setHidden(true);
            }
            ++parentIterator;
        }
    }

    void ConfigurationDialog::setServers(Servers::Servers *servers)
    {
        this->servers = servers;
    }

    void ConfigurationDialog::readSettings()
    {
        // read settings from INI and prefill config dialog items with default values

        ui->checkbox_runOnStartUp->setChecked(settings->get("global/runonstartup", false).toBool());
        ui->checkbox_autostartDaemons->setChecked(settings->get("global/autostartdaemons", false).toBool());
        ui->checkbox_startMinimized->setChecked(settings->get("global/startminimized", false).toBool());

        ui->checkbox_autostart_PHP->setChecked(settings->get("autostart/php", true).toBool());
        ui->checkbox_autostart_Nginx->setChecked(settings->get("autostart/nginx", true).toBool());
        ui->checkbox_autostart_MariaDb->setChecked(settings->get("autostart/mariadb", true).toBool());
        ui->checkbox_autostart_MongoDb->setChecked(settings->get("autostart/mongodb", false).toBool());
        ui->checkbox_autostart_Memcached->setChecked(settings->get("autostart/memcached", false).toBool());
        ui->checkbox_autostart_Postgresql->setChecked(settings->get("autostart/postgresql", false).toBool());
        ui->checkbox_autostart_Redis->setChecked(settings->get("autostart/redis", false).toBool());

        ui->checkbox_clearLogsOnStart->setChecked(settings->get("global/clearlogsonstart", false).toBool());
        ui->checkbox_stopDaemonsOnQuit->setChecked(settings->get("global/stopdaemonsonquit", false).toBool());

        ui->checkbox_onStartAllMinimize->setChecked(settings->get("global/onstartallminimize", false).toBool());
        ui->checkbox_onStartAllOpenWebinterface->setChecked(settings->get("global/onstartallopenwebinterface", false).toBool());

        ui->lineEdit_SelectedEditor->setText(settings->get("global/editor", QVariant(QString("notepad.exe")) ).toString());

        /**
         * Configuration > Updater > Self Updater
         */

        ui->checkBox_SelfUpdater_RunOnStartUp->setChecked(settings->get("selfupdater/runonstartup", false).toBool());
        ui->checkBox_SelfUpdater_AutoUpdate->setChecked(settings->get("selfupdater/autoupdate", false).toBool());
        ui->checkBox_SelfUpdater_AutoRestart->setChecked(settings->get("selfupdater/autorestart", false).toBool());

        /**
         * Configuration > Components > Xdebug
         */

        // remote
        ui->checkBox_xdebug_remote_enable->setChecked(settings->get("xdebug/remote_enable", true).toBool());
        ui->lineEdit_xdebug_remote_host->setText(settings->get("xdebug/remote_host", QVariant(QString("127.0.0.1"))).toString());
        ui->lineEdit_xdebug_remote_port->setText(settings->get("xdebug/remote_port", QVariant(QString("9100"))).toString());
        ui->checkBox_xdebug_remote_autostart->setChecked(settings->get("xdebug/remote_autostart", true).toBool());
        ui->lineEdit_xdebug_remote_handler->setText(settings->get("xdebug/remote_handler", QVariant(QString("dbgp"))).toString());
        ui->comboBox_xdebug_remote_mode->setCurrentText(settings->get("xdebug/remote_mode", QVariant(QString("req"))).toString());
        // profiler
        ui->checkBox_xdebug_enable_profiler->setChecked(settings->get("xdebug/enable_profiler", true).toBool());
        ui->checkBox_xdebug_remove_old_logs->setChecked(settings->get("xdebug/remove_old_logs", true).toBool());

        ui->lineEdit_xdebug_idekey->setText(settings->get("xdebug/idekey", QVariant(QString("netbeans-xdebug"))).toString());

        /**
         * Configuration > Components > MariaDB
         */

        ui->lineEdit_mariadb_port->setText(settings->get("mariadb/port", QVariant(QString("3306"))).toString() );

        /**
         * Configuration > Components > MongoDB
         */

        ui->lineEdit_mongodb_bindip->setText(settings->get("mongodb/bind_ip", QVariant(QString("127.0.0.1"))).toString() );
        ui->lineEdit_mongodb_port->setText(settings->get("mongodb/port", QVariant(QString("27017"))).toString() );
        ui->comboBox_mongodb_storageengine->setCurrentIndex(
           ui->comboBox_mongodb_storageengine->findText(
                       settings->get("mongodb/storageengine", QVariant(QString("wiredTiger"))).toString()
        ));
        ui->checkBox_mongodb_fork->setChecked(settings->get("mongodb/fork", true).toBool());
        ui->checkBox_mongodb_rest->setChecked(settings->get("mongodb/rest", true).toBool());
        ui->checkBox_mongodb_verbose->setChecked(settings->get("mongodb/verbose", true).toBool());
        ui->checkBox_mongodb_noauth->setChecked(settings->get("mongodb/noauth", true).toBool());
        ui->lineEdit_mongodb_dbpath->setText(settings->get("mongodb/dbpath",
            QVariant(QString(QDir::currentPath() + "/bin/mongodb/data/db"))).toString()
        );

        /**
         * Configuration > Components > PostgreSQL
         */

        ui->lineEdit_postgresql_port->setText(settings->get("postgresql/port", QVariant(QString("3306"))).toString() );

        /**
         * Configuration > Components > Memcached
         */

        ui->lineEdit_memcached_tcpport->setText(settings->get("memcached/tcpport", QVariant(QString("11211"))).toString() );
        ui->lineEdit_memcached_udpport->setText(settings->get("memcached/udpport", QVariant(QString("0"))).toString() );
        ui->lineEdit_memcached_threads->setText(settings->get("memcached/threads", QVariant(QString("2"))).toString() );
        ui->lineEdit_memcached_maxconnections->setText(settings->get("memcached/maxconnections", QVariant(QString("2048"))).toString() );
        ui->lineEdit_memcached_maxmemory->setText(settings->get("memcached/maxmemory", QVariant(QString("2048"))).toString() );

        /**
         * Configuration > Components > Redis
         */
        ui->lineEdit_redis_bind->setText(settings->get("redis/bind", QVariant(QString("127.0.0.1"))).toString() );
        ui->lineEdit_redis_port->setText(settings->get("redis/port", QVariant(QString("6379"))).toString() );
    }

    void ConfigurationDialog::writeSettings()
    {
        // we convert the type "boolean" from isChecked() to "int".
        // because i like having a simple 0/1 in the INI file, instead of true/false.

        /**
         * Page "Server Control Panel" - Tab "Configuration"
         */

        settings->set("global/runonstartup",      int(ui->checkbox_runOnStartUp->isChecked()));
        settings->set("global/startminimized",    int(ui->checkbox_startMinimized->isChecked()));
        settings->set("global/autostartdaemons",  int(ui->checkbox_autostartDaemons->isChecked()));

        settings->set("global/clearlogsonstart",  int(ui->checkbox_clearLogsOnStart->isChecked()));
        settings->set("global/stopdaemonsonquit", int(ui->checkbox_stopDaemonsOnQuit->isChecked()));

        settings->set("global/onstartallminimize",         int(ui->checkbox_onStartAllMinimize->isChecked()));
        settings->set("global/onstartallopenwebinterface", int(ui->checkbox_onStartAllOpenWebinterface->isChecked()));

        settings->set("global/editor",            QString(ui->lineEdit_SelectedEditor->text()));

        /**
         * Configuration > Updater > Self Updater
         */

        settings->set("selfupdater/runonstartup", int(ui->checkBox_SelfUpdater_RunOnStartUp->isChecked()));
        settings->set("selfupdater/autoupdate",   int(ui->checkBox_SelfUpdater_AutoUpdate->isChecked()));
        settings->set("selfupdater/autorestart",  int(ui->checkBox_SelfUpdater_AutoRestart->isChecked()));

        /**
         * Autostart Servers with the Server Control Panel
         */

        settings->set("autostart/nginx",          int(ui->checkbox_autostart_Nginx->isChecked()));
        settings->set("autostart/php",            int(ui->checkbox_autostart_PHP->isChecked()));
        settings->set("autostart/mariadb",        int(ui->checkbox_autostart_MariaDb->isChecked()));
        settings->set("autostart/mongodb",        int(ui->checkbox_autostart_MongoDb->isChecked()));
        settings->set("autostart/memcached",      int(ui->checkbox_autostart_Memcached->isChecked()));
        settings->set("autostart/postgresql",     int(ui->checkbox_autostart_Postgresql->isChecked()));
        settings->set("autostart/redis",          int(ui->checkbox_autostart_Redis->isChecked()));

        /**
         * Configuration > Components > PHP
         */

        /**
         * Configuration > Components > Nginx
         */

        /**
         * Configuration > Components > XDebug
         */

        settings->set("xdebug/remote_enable",     QString(ui->checkBox_xdebug_remote_enable->isChecked()));
        settings->set("xdebug/remote_host",       QString(ui->lineEdit_xdebug_remote_host->text()));
        settings->set("xdebug/remote_port",       QString(ui->lineEdit_xdebug_remote_port->text()));
        settings->set("xdebug/remote_autostart",  QString(ui->checkBox_xdebug_remote_autostart->isChecked()));
        settings->set("xdebug/remote_handler",    QString(ui->lineEdit_xdebug_remote_handler->text()));
        settings->set("xdebug/remote_mode",       QString(ui->comboBox_xdebug_remote_mode->currentText()));

        settings->set("xdebug/idekey",            QString(ui->lineEdit_xdebug_idekey->text()));

        /**
         * Configuration > Components > MariaDB
         */

        settings->set("mariadb/port",             QString(ui->lineEdit_mariadb_port->text()));

        /**
         * Configuration > Components > MongoDB
         */

        settings->set("mongodb/bind_ip",          QString(ui->lineEdit_mongodb_bindip->text()));
        settings->set("mongodb/port",             QString(ui->lineEdit_mongodb_port->text()));
        settings->set("mongodb/storageengine",    QString(ui->comboBox_mongodb_storageengine->currentText()));
        settings->set("mongodb/fork",             QString(ui->checkBox_mongodb_fork->isChecked()));
        settings->set("mongodb/noauth",           QString(ui->checkBox_mongodb_noauth->isChecked()));
        settings->set("mongodb/rest",             QString(ui->checkBox_mongodb_rest->isChecked()));
        settings->set("mongodb/verbose",          QString(ui->checkBox_mongodb_verbose->isChecked()));
        settings->set("mongodb/dbpath",           QString(ui->lineEdit_mongodb_dbpath->text()));

        /**
         * Configuration > Components > PostgreSQL
         */
        settings->set("postgresql/port",          QString(ui->lineEdit_postgresql_port->text()));

        /**
         * Configuration > Components > Redis
         */
        settings->set("redis/bind",               QString(ui->lineEdit_redis_bind->text()));
        settings->set("redis/port",               QString(ui->lineEdit_redis_port->text()));

        /**
         * Configuration > Components > Memcached
         */

        settings->set("memcached/tcpport",        QString(ui->lineEdit_memcached_tcpport->text()));
        settings->set("memcached/udpport",        QString(ui->lineEdit_memcached_udpport->text()));
        settings->set("memcached/threads",        QString(ui->lineEdit_memcached_threads->text()));
        settings->set("memcached/maxconnections", QString(ui->lineEdit_memcached_maxconnections->text()));
        settings->set("memcached/maxmemory",      QString(ui->lineEdit_memcached_maxmemory->text()));

        /**
         * Tab "Upstream" > Page "Nginx"
         */
        saveSettings_Nginx_Upstream();

        /**
         * Tab "Configuration" > Page "MariaDB"
         */
        saveSettings_MariaDB_Configuration();

        /**
         * Tab "Configuration" > Page "Regis"
         */
        saveSettings_Redis_Configuration();

        /**
         *Tab "Configuration" > Page "MongoDB"
         */
        //saveSettings_MongoDB_Configuration();
    }

    void ConfigurationDialog::saveSettings_PostgreSQL_Configuration()
    {
        QString file = settings->get("postgresql/config").toString();
        if(!QFile(file).exists()) {
            qDebug() << "[Error]" << file << "not found";
        }

        File::INI *ini = new File::INI(file.toLatin1());

        ini->setStringValue("postgresql", "port", ui->lineEdit_postgresql_port->text().toLatin1());

        ini->writeConfigFile();
    }

    /**
     * Redis uses a custom configuration file format
     * with a custom read and write mechanism for the config file: CONFIG GET + CONFIG SET.
     * We read the file as a standard text file and replace lines in the content.
     */
    void ConfigurationDialog::saveSettings_Redis_Configuration()
    {
        // get redis configuration file path
        QString file = settings->get("redis/config").toString();
        if(!QFile(file).exists()) {
            qDebug() << "[Error]" << file << "not found";
        }

        // read file
        QString configContent = File::Text::load(file.toLatin1());

        // split linewise by newline command
        QStringList configLines = configContent.split(QRegExp("\n|\r\n|\r"));

        // clear the content (so that we can re-add all the lines)
        configContent.clear();

        // prepare line(s) to replace
        QString newline_bind = "bind " + ui->lineEdit_redis_bind->text().toLatin1();
        QString newline_port = "port " + ui->lineEdit_redis_port->text().toLatin1();

        // iterate over all lines and replace or re-add lines
        QString line;
        for (int i = 0; i < configLines.size(); i++)
        {
            line = configLines.at(i);

            // replace line: port
            if(line.startsWith("port", Qt::CaseInsensitive)) {
                configContent.append(QString("%0\n").arg(newline_port));
            }

            // replace line: bind
            else if(line.startsWith("bind", Qt::CaseInsensitive)) {
                configContent.append(QString("%0\n").arg(newline_bind));
            }

            // append "old" line
            else {
                configContent.append(QString("%0\n").arg(line));
            }

            line.clear();
        }


        // remove last newline command
        configContent = configContent.trimmed();

        // write file
        File::Text::save(configContent, file);
    }

    void ConfigurationDialog::saveSettings_Xdebug_Configuration()
    {
        // get xdebug configuration file path
        // xdebug configuration directives are set in php.ini
        QString file = settings->get("php/config").toString();
        if(!QFile(file).exists()) {
            qDebug() << "[Error]" << file << "not found";
        }

        File::INI *ini = new File::INI(file.toLatin1());

        // remote
        ini->setBoolValue("xdebug", "remote_enable",       ui->checkBox_xdebug_remote_enable->isChecked());
        ini->setStringValue("xdebug", "remote_host",       ui->lineEdit_xdebug_remote_host->text().toLatin1());
        ini->setStringValue("xdebug", "remote_port",       ui->lineEdit_xdebug_remote_port->text().toLatin1());
        ini->setBoolValue("xdebug", "remote_autostart",    ui->checkBox_xdebug_remote_autostart->isChecked());
        ini->setStringValue("xdebug", "remote_handler",    ui->lineEdit_xdebug_remote_handler->text().toLatin1());
        ini->setStringValue("xdebug", "remote_mode",       ui->comboBox_xdebug_remote_mode->currentText().toLatin1());
        // profiler
        ini->setBoolValue("xdebug", "enable_profiler",     ui->checkBox_xdebug_enable_profiler->isChecked());
        ini->setBoolValue("xdebug", "remove_old_logs",     ui->checkBox_xdebug_remove_old_logs->isChecked());
        ini->setStringValue("xdebug", "idekey",            ui->lineEdit_xdebug_idekey->text().toLatin1());

        ini->writeConfigFile();
    }

    void ConfigurationDialog::saveSettings_MariaDB_Configuration()
    {
        QString file = settings->get("mariadb/config").toString();
        if(!QFile(file).exists()) {
            qDebug() << "[Error]" << file << "not found";
        }

        File::INI *ini = new File::INI(file.toLatin1());
        // set port to "[client] port" and "[mysqld] port"
        ini->setStringValue("client", "port", ui->lineEdit_mariadb_port->text().toLatin1());
        ini->setStringValue("mysqld", "port", ui->lineEdit_mariadb_port->text().toLatin1());
        ini->writeConfigFile();
    }

    void ConfigurationDialog::saveSettings_MongoDB_Configuration()
    {
        // TODO fix crash: QWaitCondition: Destroyed while threads are still waiting

        QString file = settings->get("mongodb/config").toString();
        if(!QFile(file).exists()) {
            qDebug() << "[Error]" << file << "not found";
        }

        // TODO switch to YAML (because the newer Mongodb versions use YAML as config format)

        File::INI *ini = new File::INI(file.toLatin1());

        ini->setStringValue("mongodb", "bind_ip",       ui->lineEdit_mongodb_bindip->text().toLatin1());
        ini->setStringValue("mongodb", "port",          ui->lineEdit_mongodb_port->text().toLatin1());
        ini->setStringValue("mongodb", "storageengine", ui->comboBox_mongodb_storageengine->currentText().toLatin1());
        ini->setBoolValue("mongodb", "fork",            ui->checkBox_mongodb_fork->isChecked());
        ini->setBoolValue("mongodb", "rest",            ui->checkBox_mongodb_rest->isChecked());
        ini->setBoolValue("mongodb", "verbose",         ui->checkBox_mongodb_verbose->isChecked());
        ini->setBoolValue("mongodb", "noauth",          ui->checkBox_mongodb_noauth->isChecked());
        ini->setStringValue("mongodb", "dbpath",        ui->lineEdit_mongodb_dbpath->text().toLatin1());

        ini->writeConfigFile();
    }

    void ConfigurationDialog::saveSettings_Nginx_Upstream()
    {
        QJsonObject upstreams;
        upstreams.insert("pools", serialize_toJSON_Nginx_Upstream_PoolsTable(ui->tableWidget_pools));

        // write JSON file
        QJsonDocument jsonDoc;
        jsonDoc.setObject(upstreams);
        File::JSON::save(jsonDoc, "./bin/wpnxm-scp/nginx-upstreams.json");

        // update Nginx upstream config files
        writeNginxUpstreamConfigs(jsonDoc);
    }

    void ConfigurationDialog::writeNginxUpstreamConfigs(QJsonDocument jsonDoc)
    {        
        createNginxConfUpstreamFolderIfNotExists_And_clearOldConfigs();

        // build servers string by iterating over all pools

        QJsonObject json = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools (key)
        for (QJsonObject:: Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter)
        {
            // the "value" object has the key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            QString poolName        = jsonPool["name"].toString();
            QString method          = jsonPool["method"].toString();
            QJsonObject jsonServers = jsonPool["servers"].toObject();

            // build "servers" block for later insertion into the upstream template string
            QString servers;

            // iterate over all servers
            for (int i = 0; i < jsonServers.count(); ++i) {
                // get values for this server
                QJsonObject s = jsonServers.value(QString::number(i)).toObject();

                // use values to build server string
                QString server = QString("    server %1:%2 weight=%3 max_fails=%4 fail_timeout=%5;\n")
                            .arg(s["address"].toString(),
                                s["port"].toString(), s["weight"].toString(),
                                s["maxfails"].toString(),s["failtimeout"].toString()
                            );

                servers.append(server);
            }

            // upstream template string
            QString upstream(
                "#\n"
                "# Automatically generated Nginx Upstream definition.\n"
                "# Do not edit manually!\n"
                "\n"
                "upstream "+poolName+" {\n"
                "    "+method+";\n"
                "\n"
                +servers+
                "}\n"
            );

            QString filename("./bin/nginx/conf/upstreams/"+poolName+".conf");

            QFile file(filename);
            if (file.open(QIODevice::ReadWrite | QFile::Truncate)) {
                QTextStream stream(&file);
                stream << upstream << endl;
            }
            file.close();

            qDebug() << "[Nginx Upstream Config] Saved: " << filename;
        }
    }

    void ConfigurationDialog::createNginxConfUpstreamFolderIfNotExists_And_clearOldConfigs()
    {
        QDir dir("./bin/nginx/conf/upstreams");

        // create Nginx Conf Upstream Folder If Not Exists
        if(!dir.exists()) {
            dir.mkpath(".");
        }

        // delete old upstream configs
        dir.setNameFilters(QStringList() << "*.conf");
        dir.setFilter(QDir::Files);
        foreach(QString dirFile, dir.entryList()) {
            dir.remove(dirFile);
        }
    }

    QJsonValue ConfigurationDialog::serialize_toJSON_Nginx_Upstream_PoolsTable(QTableWidget *pools)
    {
        QJsonObject jsonPools; // 1..n jsonPool's
        QJsonObject jsonPool;  // pool key/value pairs

        int rows = pools->rowCount();

        for (int i = 0; i < rows; ++i) {

            QString poolName = pools->item(i, NginxAddPoolDialog::Column::Pool)->text();
            QString method = pools->item(i, NginxAddPoolDialog::Column::Method)->text();

            jsonPool.insert("name", poolName);
            jsonPool.insert("method", method);

            // serialize the currently displayed server table
            if(ui->tableWidget_servers->property("servers_of_pool_name") == poolName) {
                qDebug() << "Serializing the currently displayed";
                qDebug() << "Servers Table of Pool" << ui->tableWidget_servers->property("servers_of_pool_name") << poolName;

                jsonPool.insert("servers", serialize_toJSON_Nginx_Upstream_ServerTable(ui->tableWidget_servers));
            } else {
                qDebug() << "Loading table data from file -- Servers of Pool" << poolName;

                // and re-use json data from file for the non-displayed ones
                QJsonObject poolFromJsonFile = getNginxUpstreamPoolByName(poolName);
                jsonPool.insert("servers", poolFromJsonFile["servers"]);
            }

            jsonPools.insert(QString::number(i), QJsonValue(jsonPool));
        }

        return QJsonValue(jsonPools);
    }

    QJsonValue ConfigurationDialog::serialize_toJSON_Nginx_Upstream_ServerTable(QTableWidget *servers)
    {
        QJsonObject jsonServers;     // 1..n jsonServer's
        QJsonObject jsonServer;      // server key/value pairs

        int rows = servers->rowCount();

        for (int i = 0; i < rows; ++i) {

            jsonServer.insert("address",     servers->item(i, 0/*NginxAddServerDialog::Column::Address*/)->text());
            jsonServer.insert("port",        servers->item(i, 1/*NginxAddServerDialog::Column::Port*/)->text());
            jsonServer.insert("weight",      servers->item(i, 2/*NginxAddServerDialog::Column::Weight*/)->text());
            jsonServer.insert("maxfails",    servers->item(i, 3/*NginxAddServerDialog::Column::MaxFails*/)->text());
            jsonServer.insert("failtimeout", servers->item(i, 4/*NginxAddServerDialog::Column::FailTimeout*/)->text());
            jsonServer.insert("phpchildren", servers->item(i, 5/*NginxAddServerDialog::Column::PHPChildren*/)->text());

            jsonServers.insert(QString::number(i), QJsonValue(jsonServer));
        }

        return QJsonValue(jsonServers);
    }

    void ConfigurationDialog::onClickedButtonBoxOk()
    {
        writeSettings();
        toggleRunOnStartup();
    }

    bool ConfigurationDialog::runOnStartUp() const
    {
        return (ui->checkbox_runOnStartUp->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setRunOnStartUp(bool run)
    {
        ui->checkbox_runOnStartUp->setChecked(run);
    }

    bool ConfigurationDialog::runAutostartDaemons() const
    {
        return (ui->checkbox_autostartDaemons->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setAutostartDaemons(bool run)
    {
        ui->checkbox_autostartDaemons->setChecked(run);
    }

    void ConfigurationDialog::toggleAutostartDaemonCheckboxes(bool run)
    {
        // Note: layout doesn't "inject" itself in the parent-child tree, so findChildren() doesn't work

        // left box
        for (int i = 0; i < ui->autostartDaemonsFormLayout->count(); ++i)
        {
          ui->autostartDaemonsFormLayout->itemAt(i)->widget()->setEnabled(run);
        }

        // right box
        for (int i = 0; i < ui->autostartDaemonsFormLayout2->count(); ++i)
        {
          ui->autostartDaemonsFormLayout2->itemAt(i)->widget()->setEnabled(run);
        }
    }

    void ConfigurationDialog::hideAutostartCheckboxesOfNotInstalledServers()
    {
        QStringList installed = this->servers->getListOfServerNamesInstalled();

        QList<QCheckBox *> boxes = ui->tabWidget->findChildren<QCheckBox *>(QRegExp("checkbox_autostart_\\w"));

        for(int i = 0; i < boxes.size(); ++i) {
           QCheckBox *box = boxes.at(i);

           // return last part of "checkbox_autostart_*"
           QString name = box->objectName().section("_", -1).toLower();
           QString labelName = this->servers->getCamelCasedServerName(name) + "Label";
           QLabel *label = ui->tabWidget->findChild<QLabel *>(labelName);

           if(installed.contains(name) == true) {
               qDebug() << "[" + name + "] Autostart Checkbox visible.";
               box->setVisible(true);
               //label->setVisible(true);
           } else {
               qDebug() << "[" + name + "] Autostart Checkbox hidden.";
               box->setVisible(false);
               label->setVisible(false);
           }
        }
    }

    bool ConfigurationDialog::runClearLogsOnStart() const
    {
        return (ui->checkbox_clearLogsOnStart->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setClearLogsOnStart(bool run)
    {
        ui->checkbox_clearLogsOnStart->setChecked(run);
    }

    bool ConfigurationDialog::stopDaemonsOnQuit() const
    {
        return (ui->checkbox_stopDaemonsOnQuit->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setStopDaemonsOnQuit(bool run)
    {
        ui->checkbox_stopDaemonsOnQuit->setChecked(run);
    }

    void ConfigurationDialog::toggleRunOnStartup()
    {
        // Windows %APPDATA% = Roaming ... Programs\Startup
        QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "\\Startup";

        if(runOnStartUp()) {
            // Add WPN-XM SCP shortcut to the Windows Autostart folder.
            // In Windows terminology "shortcuts" are "shell links".
            WindowsAPI::QtWin::CreateShellLink(
                qApp->applicationFilePath(),"","WPN-XM Server Control Panel", // app, args, desc
                qApp->applicationFilePath(),0,                                // icon path and idx
                qApp->applicationDirPath(),                                   // working dir
                startupDir + "\\WPN-XM Server Control Panel.lnk"              // filepath of shortcut
            );
        } else {
            // remove link
            QFile::remove(startupDir+"\\WPN-XM Server Control Panel.lnk");
        }
    }

    void ConfigurationDialog::fileOpen()
    {
        QString file = QFileDialog::getOpenFileName(this, tr("Select Editor..."),
                getenv("PROGRAMFILES"), tr("Executables (*.exe);;All Files (*)"));

        file = QDir::toNativeSeparators(file);

        ui->lineEdit_SelectedEditor->setText(file);
    }

    void ConfigurationDialog::on_toolButton_SelectEditor_clicked()
    {
        ConfigurationDialog::fileOpen();
    }

    void ConfigurationDialog::on_toolButton_ResetEditor_clicked()
    {
        ui->lineEdit_SelectedEditor->setText("notepad.exe");
    }

    void ConfigurationDialog::on_configMenuTreeWidget_clicked(const QModelIndex &index)
    {
        // a click on a menu item switches to the matching page in the stacked widget
        QString menuitem = ui->configMenuTreeWidget->model()->data(index).toString().toLower().remove(" ");
        QWidget *w = ui->stackedWidget->findChild<QWidget *>(menuitem);
        if(w != 0)
            ui->stackedWidget->setCurrentWidget(w);
        else
            qDebug() << "[Config Menu] There is no page " << menuitem << " in the stack widget.";
    }

    void ConfigurationDialog::on_pushButton_Nginx_Upstream_AddPool_clicked()
    {
        int result;

        NginxAddPoolDialog *dialog = new NginxAddPoolDialog();
        dialog->setWindowTitle("Nginx - Add Pool");

        //ui->tableWidget_pools->setSelectionBehavior(QAbstractItemView::SelectRows);
        //ui->tableWidget_pools->setSelectionMode(QAbstractItemView::SingleSelection);

        result = dialog->exec();

        if(result == QDialog::Accepted) {
            int row = ui->tableWidget_pools->rowCount();
            ui->tableWidget_pools->insertRow(row);
            ui->tableWidget_pools->setItem(row, NginxAddPoolDialog::Column::Pool,   new QTableWidgetItem(dialog->pool()));
            ui->tableWidget_pools->setItem(row, NginxAddPoolDialog::Column::Method, new QTableWidgetItem(dialog->method()));
        }

        delete dialog;
    }

    void ConfigurationDialog::on_pushButton_Nginx_Upstream_AddServer_clicked()
    {
        int result;

        NginxAddServerDialog *dialog = new NginxAddServerDialog();
        dialog->setWindowTitle("Nginx - Add Server");

        ui->tableWidget_servers->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget_servers->setSelectionMode(QAbstractItemView::SingleSelection);

        result = dialog->exec();

        if(result == QDialog::Accepted) {
            int row = ui->tableWidget_servers->rowCount();
            ui->tableWidget_servers->insertRow(row);
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::Address,     new QTableWidgetItem(dialog->address()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::Port,        new QTableWidgetItem(dialog->port()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::Weight,      new QTableWidgetItem(dialog->weight()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::MaxFails,    new QTableWidgetItem(dialog->maxfails()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::Timeout,     new QTableWidgetItem(dialog->timeout()));
            ui->tableWidget_servers->setItem(row, NginxAddServerDialog::Column::PHPChildren, new QTableWidgetItem(dialog->phpchildren()));
        }

        delete dialog;
    }

    void ConfigurationDialog::loadNginxUpstreams()
    {
        // clear servers table - clear content and remove all rows
        ui->tableWidget_pools->setRowCount(0);
        ui->tableWidget_servers->setRowCount(0);

        // load JSON
        QJsonDocument jsonDoc = File::JSON::load("./bin/wpnxm-scp/nginx-upstreams.json");
        QJsonObject json = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools
        for (QJsonObject:: Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter)
        {
            // The "value" are the key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            // --- Fill Pools Table ---

            // insert new row
            int insertRow = ui->tableWidget_pools->rowCount();
            ui->tableWidget_pools->insertRow(insertRow);

            // insert column values
            ui->tableWidget_pools->setItem(insertRow,NginxAddPoolDialog::Column::Pool,
                                           new QTableWidgetItem(jsonPool["name"].toString()));
            ui->tableWidget_pools->setItem(insertRow,NginxAddPoolDialog::Column::Method,
                                           new QTableWidgetItem(jsonPool["method"].toString()));
        }

        // --- Fill Servers Table ---

        // get the first pool, then the "server" key
        QJsonObject jsonPoolFirst = jsonPools.value(QString::number(0)).toObject();

        updateServersTable(jsonPoolFirst);
    }

    void ConfigurationDialog::on_tableWidget_pools_itemSelectionChanged()
    {
        // there is a selection, but its not a row selection
        if(ui->tableWidget_pools->selectionModel()->selectedRows(0).size() <= 0) {
            return;
        }

        // get "pool" from selection
        QString selectedPoolName = ui->tableWidget_pools->selectionModel()->selectedRows().first().data().toString();

        // there is a selection, but the selection is already the currently displayed table view
        if (ui->tableWidget_servers->property("servers_of_pool_name") == selectedPoolName) {
            return;
        }

        // get the pool and update servers table
        QJsonObject jsonPool = getNginxUpstreamPoolByName(selectedPoolName);
        updateServersTable(jsonPool);
    }

    void ConfigurationDialog::updateServersTable(QJsonObject jsonPool)
    {
        // clear servers table - clear content and remove all rows
        ui->tableWidget_servers->setRowCount(0);

        // set new "pool name" as table property (table view identifier)
        ui->tableWidget_servers->setProperty("servers_of_pool_name", jsonPool["name"].toString());

        // key "servers"
        QJsonObject jsonServers = jsonPool["servers"].toObject();

        for (int i = 0; i < jsonServers.count(); ++i) {

            // values for a "server"
            QJsonObject values = jsonServers.value(QString::number(i)).toObject();

            // insert new row
            int insertRow = ui->tableWidget_servers->rowCount();
            ui->tableWidget_servers->insertRow(insertRow);

            // insert column values
            ui->tableWidget_servers->setItem(
                        insertRow,0/*NginxAddServerDialog::Column::Address*/,     new QTableWidgetItem(values["address"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,1/*NginxAddServerDialog::Column::Port*/,        new QTableWidgetItem(values["port"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,2/*NginxAddServerDialog::Column::Weight*/,      new QTableWidgetItem(values["weight"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,3/*NginxAddServerDialog::Column::MaxFails*/,    new QTableWidgetItem(values["maxfails"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,4/*NginxAddServerDialog::Column::FailTimeout*/, new QTableWidgetItem(values["failtimeout"].toString()));
            ui->tableWidget_servers->setItem(
                        insertRow,5/*NginxAddServerDialog::Column::PHPChildren*/, new QTableWidgetItem(values["phpchildren"].toString()));
        }
    }

    QJsonObject ConfigurationDialog::getNginxUpstreamPoolByName(QString requestedPoolName)
    {
        // load JSON
        QJsonDocument jsonDoc = File::JSON::load("./bin/wpnxm-scp/nginx-upstreams.json");
        QJsonObject json = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools
        for (QJsonObject:: Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter)
        {
            // "value" is key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            // key "name" = poolName
            if(jsonPool["name"].toString() == requestedPoolName) {
                return jsonPool;
            }
        }

        return QJsonObject();
    }



}
