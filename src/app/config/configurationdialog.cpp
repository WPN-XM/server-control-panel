#include "configurationdialog.h"
#include "ui_configurationdialog.h"

namespace Configuration
{
    ConfigurationDialog::ConfigurationDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ConfigurationDialog)
    {
        ui->setupUi(this);

        setWindowTitle("WPN-XM Server Control Panel - Configuration");

        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        settings = new Settings::SettingsManager;
        readSettings();

        hideComponentsNotInstalledInMenuTree();

        // setup autostart section
        hideAutostartCheckboxesOfNotInstalledServers();
        toggleAutostartServerCheckboxes(ui->checkbox_autostartServers->isChecked());
        connect(ui->checkbox_autostartServers, SIGNAL(clicked(bool)), this,
                SLOT(toggleAutostartServerCheckboxes(bool)));

        // load initial data for pages
        loadNginxUpstreams();

        createPHPExtensionListWidget();
        QObject::connect(ui->php_extensions_listWidget, SIGNAL(itemChanged(QListWidgetItem *)), this,
                         SLOT(PHPExtensionListWidgetHighlightChecked(QListWidgetItem *)));

        // plugins
        ui->listWidget_plugins->setLayoutDirection(Qt::LeftToRight);
        ui->listWidget_plugins->setItemDelegate(new PluginListDelegate(ui->listWidget_plugins));

        PluginManager *pManager                  = new PluginManager();
        const QList<Plugins::Plugin> &allPlugins = pManager->getAvailablePlugins();

        foreach (const Plugins::Plugin &plugin, allPlugins) {

            qDebug() << plugin.metaData.name;

            QListWidgetItem *item = new QListWidgetItem(ui->listWidget_plugins);
            // QIcon icon = QIcon(desc.icon);
            // item->setIcon(icon);
            QString pluginInfo = QString("<b>%1</b> %2<br/><i>%3</i><br/>")
                                     .arg(plugin.metaData.name, plugin.metaData.version, plugin.metaData.author);
            item->setToolTip(pluginInfo);

            item->setText(plugin.metaData.name);
            item->setData(Qt::UserRole, plugin.metaData.version);
            item->setData(Qt::UserRole + 1, plugin.metaData.author);
            item->setData(Qt::UserRole + 2, plugin.metaData.description);

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(plugin.isLoaded() ? Qt::Checked : Qt::Unchecked);
            item->setData(Qt::UserRole + 10, QVariant::fromValue(plugin));

            ui->listWidget_plugins->addItem(item);
        }

        connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(onClickedButtonBoxOk()));

        ui->configMenuTreeWidget->expandAll();
    }

    ConfigurationDialog::~ConfigurationDialog() { delete ui; }

    QStringList ConfigurationDialog::getAvailablePHPExtensions()
    {
        QStringList availableExtensions;

        QString folder = QDir::currentPath() + "/bin/php/ext";
        QDir ext(folder);
        ext.setNameFilters(QStringList() << "*.dll");
        QStringList fileList = ext.entryList();

        for (int i = 0; i < fileList.size(); ++i) {
            availableExtensions << fileList.at(i).chopped(4).mid(4);
        }

        // qDebug() << availableExtensions;

        return availableExtensions;
    }

    QStringList ConfigurationDialog::getEnabledPHPExtensions()
    {
        QStringList enabledExtensions;

        QString filename = QDir::toNativeSeparators(QDir::currentPath() + "/bin/php/php.ini");
        QFile f(filename);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << " Could not open the file for reading";
        }

        QTextStream in(&f);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("extension=")) {
                enabledExtensions << line.mid(10);
            }
        }

        f.close();

        return enabledExtensions;
    }

    void ConfigurationDialog::createPHPExtensionListWidget()
    {
        QStringList availableList = getAvailablePHPExtensions();
        QStringList enabledList   = getEnabledPHPExtensions();

        ui->php_extensions_listWidget->addItems(availableList);

        QListWidgetItem *item = nullptr;
        for (int i = 0; i < ui->php_extensions_listWidget->count(); ++i) {
            item = ui->php_extensions_listWidget->item(i);
            // special handling for zend_extension xdebug
            if (item->text() == "xdebug") {
                item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsUserCheckable);
                item->setBackgroundColor(QColor("lightgray"));
            } else {
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            }

            if (enabledList.contains(item->text())) {
                item->setCheckState(Qt::Checked);
                item->setBackgroundColor(QColor("#90ee90"));
            } else {
                item->setCheckState(Qt::Unchecked);
            }
        }
    }

    void ConfigurationDialog::PHPExtensionListWidgetHighlightChecked(QListWidgetItem *item)
    {
        // disconnect is needed, because setBackgroundColor triggers itemChanged
        // we need to avoid getting two signals, because we want to save the state only once
        QObject::disconnect(ui->php_extensions_listWidget, SIGNAL(itemChanged(QListWidgetItem *)), this,
                            SLOT(PHPExtensionListWidgetHighlightChecked(QListWidgetItem *)));

        if (item->checkState() == Qt::Checked) {
            item->setBackgroundColor(QColor("#90ee90"));
            savePHPExtensionState(item->text(), true);
        } else {
            item->setBackgroundColor(QColor("#ffffff"));
            savePHPExtensionState(item->text(), false);
        }

        QObject::connect(ui->php_extensions_listWidget, SIGNAL(itemChanged(QListWidgetItem *)), this,
                         SLOT(PHPExtensionListWidgetHighlightChecked(QListWidgetItem *)));
    }

    void ConfigurationDialog::savePHPExtensionState(QString ext, bool enable)
    {
        QString filename = QDir::toNativeSeparators(QDir::currentPath() + "/bin/php/php.ini");
        QFile f(filename);
        if (!f.open(QIODevice::ReadWrite | QIODevice::Text)) {
            qDebug() << " Could not open the file for reading and writing.";
        }

        QString content;
        QTextStream in(&f);
        while (!in.atEnd()) {
            content = in.readAll();
        }

        QString extName = QString("extension=").append(ext);

        if (content.contains(extName)) {
            if (enable) {
                content.replace(QString(";extension=").append(ext), extName);
            } else {
                content.replace(extName, QString(";extension=").append(ext));
            }
        } else {
            // no entry in ini, yet. add to ini
            if (enable) {
                content.append(extName);
            } else {
                content.append(QString(";extension=").append(ext));
            }
        }

        f.seek(0);
        f.write(content.toUtf8());
        f.close();
    }

    /**
     * Search for items in the "Configuration Menu" TreeWidget
     * by using type-ahead search
     *
     * @brief ConfigurationDialog::on_configMenuSearchLineEdit_textChanged
     * @param query
     */
    void ConfigurationDialog::on_configMenuSearchLineEdit_textChanged(const QString &query)
    {
        ui->configMenuTreeWidget->expandAll();

        // Iterate over all child items : filter items with "contains" query
        QTreeWidgetItemIterator iterator(ui->configMenuTreeWidget, QTreeWidgetItemIterator::All);
        while (*iterator) {
            QTreeWidgetItem *item = *iterator;
            if (item->text(0).contains(query, Qt::CaseInsensitive)) {
                item->setHidden(false);
            } else {
                // Problem:
                // the matched child is visibile, but parent is hidden, because no match.
                // so, lets hide only items without childs.
                // any not matching parent will stay visible..
                // until we iterate over items with hidden childs and hide the parent
                // see below
                if (item->childCount() == 0) {
                    item->setHidden(true);
                }
            }
            ++iterator;
        }

        // Iterate over items with hidden childs (parents) and hide them too
        QTreeWidgetItemIterator parentIterator(ui->configMenuTreeWidget,
                                               QTreeWidgetItemIterator::HasChildren | QTreeWidgetItemIterator::Hidden);

        while (*parentIterator) {
            QTreeWidgetItem *item = *parentIterator;
            item->setHidden(true);
            ++parentIterator;
        }
    }

    void ConfigurationDialog::setServers(Servers::Servers *servers) { this->servers = servers; }

    /**
     * @brief ConfigurationDialog::readSettings
     *
     * Reads settings from INI and prefills config dialog items with default values.
     */
    void ConfigurationDialog::readSettings()
    {
        ui->checkbox_runOnStartUp->setChecked(getSettingBool("global/runonstartup", false));
        ui->checkbox_autostartServers->setChecked(getSettingBool("global/autostartservers", false));
        ui->checkbox_startMinimized->setChecked(getSettingBool("global/startminimized", false));

        ui->checkbox_autostart_PHP->setChecked(getSettingBool("autostart/php", true));
        ui->checkbox_autostart_Nginx->setChecked(getSettingBool("autostart/nginx", true));
        ui->checkbox_autostart_MariaDb->setChecked(getSettingBool("autostart/mariadb", true));
        if (isServerInstalled("mongodb")) {
            ui->checkbox_autostart_MongoDb->setChecked(getSettingBool("autostart/mongodb", false));
        }
        if (isServerInstalled("memcached")) {
            ui->checkbox_autostart_Memcached->setChecked(getSettingBool("autostart/memcached", false));
        }
        if (isServerInstalled("postgresql")) {
            ui->checkbox_autostart_Postgresql->setChecked(getSettingBool("autostart/postgresql", false));
        }
        if (isServerInstalled("redis")) {
            ui->checkbox_autostart_Redis->setChecked(getSettingBool("autostart/redis", false));
        }

        ui->checkbox_clearLogsOnStart->setChecked(getSettingBool("global/clearlogsonstart", false));
        ui->checkbox_stopServersOnQuit->setChecked(getSettingBool("global/stopServersonquit", false));

        ui->checkbox_onStartAllMinimize->setChecked(getSettingBool("global/onstartallminimize", false));
        ui->checkbox_onStartAllOpenWebinterface->setChecked(getSettingBool("global/onstartallopenwebinterface", false));

        ui->lineEdit_SelectedEditor->setText(getSettingString("global/editor", QString("notepad.exe")));

        //
        // Configuration > Updater > Self Updater
        //

        ui->checkBox_SelfUpdater_RunOnStartUp->setChecked(getSettingBool("selfupdater/runonstartup", false));
        ui->checkBox_SelfUpdater_AutoUpdate->setChecked(getSettingBool("selfupdater/autoupdate", false));
        ui->checkBox_SelfUpdater_AutoRestart->setChecked(getSettingBool("selfupdater/autorestart", false));

        ui->comboBox_SelfUpdater_Interval->setCurrentText(getSettingString("selfupdater/interval", QString("1")));
        ui->dateTimeEdit_SelfUpdater_Last_Time_Checked->setDateTime(
            settings->get("selfupdater/last_time_checked", QVariant(0)).toDateTime());

        //
        // Configuration > Components > Xdebug
        //

        // remote
        ui->checkBox_xdebug_remote_enable->setChecked(getSettingBool("xdebug/remote_enable", true));
        ui->lineEdit_xdebug_remote_host->setText(getSettingString("xdebug/remote_host", QString("127.0.0.1")));
        ui->lineEdit_xdebug_remote_port->setText(getSettingString("xdebug/remote_port", QString("9100")));
        ui->checkBox_xdebug_remote_autostart->setChecked(getSettingBool("xdebug/remote_autostart", true));
        ui->lineEdit_xdebug_remote_handler->setText(getSettingString("xdebug/remote_handler", QString("dbgp")));
        ui->comboBox_xdebug_remote_mode->setCurrentText(getSettingString("xdebug/remote_mode", QString("req")));
        // profiler
        ui->checkBox_xdebug_enable_profiler->setChecked(getSettingBool("xdebug/enable_profiler", true));
        ui->checkBox_xdebug_remove_old_logs->setChecked(getSettingBool("xdebug/remove_old_logs", true));

        ui->lineEdit_xdebug_idekey->setText(getSettingString("xdebug/idekey", QString("netbeans-xdebug")));

        //
        // Configuration > Components > MariaDB
        //

        ui->lineEdit_mariadb_port->setText(getSettingString("mariadb/port", QString("3306")));

        //
        // Configuration > Components > MongoDb
        //

        if (isServerInstalled("mongodb")) {
            ui->lineEdit_mongodb_port->setText(getSettingString("mongodb/port", QString("27017")));

            ui->lineEdit_mongodb_dbpath->setText(getSettingString(
                "mongodb/dbpath", QDir::toNativeSeparators(QDir::currentPath() + "/bin/mongodb/data/db")));
        }

        //
        // Configuration > Components > PostgreSQL
        //

        if (isServerInstalled("postgresql")) {
            ui->lineEdit_postgresql_port->setText(getSettingString("postgresql/port", QString("3306")));
        }

        //
        // Configuration > Components > Memcached
        //
        /*
                if (servers->isInstalled("memcached")) {
                    ui->lineEdit_memcached_tcpport->setText(getSettingString("memcached/tcpport",
           QString("11211")));
                    ui->lineEdit_memcached_udpport->setText(getSettingString("memcached/udpport",
           QString("0")));
                    ui->lineEdit_memcached_threads->setText(getSettingString("memcached/threads",
           QString("2"))); ui->lineEdit_memcached_maxconnections->setText(
                        getSettingString("memcached/maxconnections",
           QString("2048")));
                    ui->lineEdit_memcached_maxmemory->setText(getSettingString("memcached/maxmemory",
           QString("2048")));
                }
        */
        //
        // Configuration > Components > Redis
        //

        if (isServerInstalled("redis")) {
            ui->lineEdit_redis_port->setText(getSettingString("redis/port", QString("6379")));
        }
    }

    bool ConfigurationDialog::isServerInstalled(const QString &serverName) const
    {
        foreach (QString server, installedServersList) {
            if (server == serverName) {
                return true;
            }
        }
        return false;
    }

    void ConfigurationDialog::writeSettings()
    {
        // we convert the type "boolean" from isChecked() to "int".
        // because i like having a simple 0/1 in the INI file, instead of true/false.
        // if boolean is saved as string to ini, you'll get "\x1" as value for true.

        //
        // Configuration > Server Control Panel > Tab "Configuration"
        //

        settings->set("global/runonstartup", int(ui->checkbox_runOnStartUp->isChecked()));
        settings->set("global/startminimized", int(ui->checkbox_startMinimized->isChecked()));
        settings->set("global/autostartservers", int(ui->checkbox_autostartServers->isChecked()));

        settings->set("global/clearlogsonstart", int(ui->checkbox_clearLogsOnStart->isChecked()));
        settings->set("global/stopserversonquit", int(ui->checkbox_stopServersOnQuit->isChecked()));

        settings->set("global/onstartallminimize", int(ui->checkbox_onStartAllMinimize->isChecked()));
        settings->set("global/onstartallopenwebinterface", int(ui->checkbox_onStartAllOpenWebinterface->isChecked()));

        settings->set("global/editor", QString(ui->lineEdit_SelectedEditor->text()));

        //
        // Autostart Servers with the Server Control Panel
        //

        settings->set("autostart/nginx", int(ui->checkbox_autostart_Nginx->isChecked()));
        settings->set("autostart/php", int(ui->checkbox_autostart_PHP->isChecked()));
        settings->set("autostart/mariadb", int(ui->checkbox_autostart_MariaDb->isChecked()));

        // checkboxes are automatically removed by hideAutostartCheckboxes...
        if (isServerInstalled("mongodb")) {
            settings->set("autostart/mongodb", int(ui->checkbox_autostart_MongoDb->isChecked()));
        }
        if (isServerInstalled("memcached")) {
            settings->set("autostart/memcached", int(ui->checkbox_autostart_Memcached->isChecked()));
        }
        if (isServerInstalled("postgresql")) {
            settings->set("autostart/postgresql", int(ui->checkbox_autostart_Postgresql->isChecked()));
        }
        if (isServerInstalled("redis")) {
            settings->set("autostart/redis", int(ui->checkbox_autostart_Redis->isChecked()));
        }

        QApplication::processEvents();

        //
        // Configuration > Updater > Update Notification Settings
        //

        settings->set("selfupdater/runonstartup", int(ui->checkBox_SelfUpdater_RunOnStartUp->isChecked()));
        settings->set("selfupdater/autoupdate", int(ui->checkBox_SelfUpdater_AutoUpdate->isChecked()));
        settings->set("selfupdater/autorestart", int(ui->checkBox_SelfUpdater_AutoRestart->isChecked()));

        settings->set("selfupdater/interval", QString("1"));
        // settings->set("selfupdater/last_time_checked", QVariant(0)).toDateTime());

        QApplication::processEvents();

        //
        // Configuration > Components > Nginx > Tab "Upstream"
        //

        saveSettings_Nginx_Upstream();
        QApplication::processEvents();

        //
        // Configuration > Components > XDebug > Tab "Configuration"
        //

        settings->set("xdebug/remote_enable", int(ui->checkBox_xdebug_remote_enable->isChecked()));
        settings->set("xdebug/remote_host", QString(ui->lineEdit_xdebug_remote_host->text()));
        settings->set("xdebug/remote_port", QString(ui->lineEdit_xdebug_remote_port->text()));
        settings->set("xdebug/remote_autostart", int(ui->checkBox_xdebug_remote_autostart->isChecked()));
        settings->set("xdebug/remote_handler", QString(ui->lineEdit_xdebug_remote_handler->text()));
        settings->set("xdebug/remote_mode", QString(ui->comboBox_xdebug_remote_mode->currentText()));

        settings->set("xdebug/idekey", QString(ui->lineEdit_xdebug_idekey->text()));

        QApplication::processEvents();

        //
        // Configuration > Components > MariaDBTab > Tab "Configuration"
        //

        settings->set("mariadb/port", QString(ui->lineEdit_mariadb_port->text()));

        saveSettings_MariaDB_Configuration();

        QApplication::processEvents();

        //
        // Configuration > Components > MongoDB
        //

        // We do not save mongodb values into the wpn-xm.ini file.
        // All setting go directly into mongod.conf yaml file.

        if (isServerInstalled("mongodb")) {
            saveSettings_MongoDB_Configuration();
            QApplication::processEvents();
        }

        //
        // Configuration > Components > PostgreSQL
        //

        if (isServerInstalled("postgresql")) {
            settings->set("postgresql/port", QString(ui->lineEdit_postgresql_port->text()));
            QApplication::processEvents();
        }

        //
        // Configuration > Components > Memcached
        //

        if (isServerInstalled("memcached")) {
            settings->set("memcached/tcpport", QString(ui->lineEdit_memcached_tcpport->text()));
            settings->set("memcached/udpport", QString(ui->lineEdit_memcached_udpport->text()));
            settings->set("memcached/threads", QString(ui->lineEdit_memcached_threads->text()));
            settings->set("memcached/maxconnections", QString(ui->lineEdit_memcached_maxconnections->text()));
            settings->set("memcached/maxmemory", QString(ui->lineEdit_memcached_maxmemory->text()));
            QApplication::processEvents();
        }

        //
        // Configuration > Components > Redis > Tab "Configuration"
        //

        if (isServerInstalled("redis")) {
            settings->set("redis/port", QString(ui->lineEdit_redis_port->text()));

            saveSettings_Redis_Configuration();
            QApplication::processEvents();
        }
    }

    void ConfigurationDialog::saveSettings_PostgreSQL_Configuration()
    {
        QString file = QDir(settings->get("postgresql/config").toString()).absolutePath();

        if (!QFile(file).exists()) {
            qDebug() << "[Error][" << Q_FUNC_INFO << "]" << file << "not found";
        }

        File::INI *ini = new File::INI(file.toLatin1());

        ini->setStringValue("postgresql", "port", ui->lineEdit_postgresql_port->text().toLatin1());

        ini->writeConfigFile();
    }

    /**
     * Redis uses a custom configuration file format
     * with a custom read and write mechanism for the config file: CONFIG GET +
     * CONFIG SET.
     * We read the file as a standard text file and replace lines in the content.
     */
    void ConfigurationDialog::saveSettings_Redis_Configuration()
    {
        // get redis configuration file path
        QString file = QDir(settings->get("redis/config").toString()).absolutePath();

        if (!QFile(file).exists()) {
            qDebug() << "[Error][" << Q_FUNC_INFO << "] redis/config not found";
        }

        // read file
        QString configContent = File::Text::load(file.toLatin1());

        // split linewise by newline command
        QStringList configLines = configContent.split(QRegExp("\n|\r\n|\r"));

        // clear the content (so that we can re-add all the lines)
        configContent.clear();

        // prepare line(s) to replace
        // QString newline_bind = "bind " +
        // ui->lineEdit_redis_bind->text().toLatin1();
        QString newline_port = "port " + ui->lineEdit_redis_port->text().toLatin1();

        // iterate over all lines and replace or re-add lines
        QString line;
        for (int i = 0; i < configLines.size(); i++) {
            line = configLines.at(i);

            // replace line: port
            if (line.startsWith("port", Qt::CaseInsensitive)) {
                configContent.append(QString("%0\n").arg(newline_port));
            }

            // replace line: bind
            /*else if (line.startsWith("bind", Qt::CaseInsensitive)) {
                configContent.append(QString("%0\n").arg(newline_bind));
            }*/

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
        QString file = QDir(settings->get("php/config").toString()).absolutePath();

        if (!QFile(file).exists()) {
            qDebug() << "[Error][" << Q_FUNC_INFO << "]" << file << "not found";
        }

        File::INI *ini = new File::INI(file.toLatin1());

        // remote
        ini->setBoolValue("xdebug", "remote_enable", ui->checkBox_xdebug_remote_enable->isChecked());
        ini->setStringValue("xdebug", "remote_host", ui->lineEdit_xdebug_remote_host->text().toLatin1());
        ini->setStringValue("xdebug", "remote_port", ui->lineEdit_xdebug_remote_port->text().toLatin1());
        ini->setBoolValue("xdebug", "remote_autostart", ui->checkBox_xdebug_remote_autostart->isChecked());
        ini->setStringValue("xdebug", "remote_handler", ui->lineEdit_xdebug_remote_handler->text().toLatin1());
        ini->setStringValue("xdebug", "remote_mode", ui->comboBox_xdebug_remote_mode->currentText().toLatin1());
        // profiler
        ini->setBoolValue("xdebug", "enable_profiler", ui->checkBox_xdebug_enable_profiler->isChecked());
        ini->setBoolValue("xdebug", "remove_old_logs", ui->checkBox_xdebug_remove_old_logs->isChecked());
        ini->setStringValue("xdebug", "idekey", ui->lineEdit_xdebug_idekey->text().toLatin1());

        ini->writeConfigFile();
    }

    void ConfigurationDialog::saveSettings_MariaDB_Configuration()
    {
        QString file = QDir(settings->get("mariadb/config").toString()).absolutePath();

        if (!QFile(file).exists()) {
            qDebug() << "[Error][" << Q_FUNC_INFO << "]" << file << "not found";
        }

        File::INI *ini = new File::INI(file.toLatin1());
        // set port to "[client] port" and "[mysqld] port"
        ini->setStringValue("client", "port", ui->lineEdit_mariadb_port->text().toLatin1());
        ini->setStringValue("mysqld", "port", ui->lineEdit_mariadb_port->text().toLatin1());
        ini->writeConfigFile();
    }

    QString toString(std::string s) { return QString(s.c_str()); }

    void ConfigurationDialog::saveSettings_MongoDB_Configuration()
    {
        QString file = QDir(settings->get("mongodb/config").toString()).absolutePath();

        if (!QFile(file).exists()) {
            qDebug() << "[Error]" << file << "not found";
        }

        auto *yml         = new File::Yml();
        YAML::Node config = yml->load(file);

        config["storage"]["dbPath"] = ui->lineEdit_mongodb_dbpath->text();
        config["net"]["port"]       = ui->lineEdit_mongodb_port->text();

        // qDebug() << YAML::yamlToVariant(config).toMap();

        yml->saveConfig(file, config);

        qDebug() << "[MongoDB][Config] Saved: " << file;
    }

    void ConfigurationDialog::saveSettings_Nginx_Upstream()
    {
        QJsonObject upstreams;
        upstreams.insert("pools", serialize_toJSON_Nginx_Upstream_PoolsTable(ui->tableWidget_Nginx_Upstreams));

        // a) write JSON file
        QJsonDocument jsonDoc;
        jsonDoc.setObject(upstreams);
        File::JSON::save(jsonDoc, "./bin/wpnxm-scp/nginx-upstreams.json");

        // b) update individual Nginx upstream config files
        writeNginxUpstreamConfigs(jsonDoc);
    }

    void ConfigurationDialog::writeNginxUpstreamConfigs(const QJsonDocument &jsonDoc)
    {
        createNginxConfUpstreamFolderIfNotExists_And_clearOldConfigs();

        // build servers string by iterating over all pools

        QJsonObject json      = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools (key)
        for (QJsonObject::Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter) {
            // the "value" object has the key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            QString poolName        = jsonPool["name"].toString();
            QString method          = jsonPool["method"].toString();
            QJsonObject jsonServers = jsonPool["servers"].toObject();

            // build "servers" block for later insertion into the upstream template
            // string
            QString servers;

            // iterate over all servers
            for (int i = 0; i < jsonServers.count(); ++i) {
                // get values for this server
                QJsonObject s = jsonServers.value(QString::number(i)).toObject();

                // use values to build server string
                QString server = QString("    server %1:%2 weight=%3 max_fails=%4 fail_timeout=%5;\n")
                                     .arg(s["address"].toString(), s["port"].toString(), s["weight"].toString(),
                                          s["maxfails"].toString(), s["failtimeout"].toString());

                servers.append(server);
            }

            // upstream template string
            QString upstream(
                "#\n"
                "# Automatically generated Nginx Upstream definition.\n"
                "# Do not edit manually!\n"
                "\n"
                "upstream " +
                poolName +
                " {\n"
                "    " +
                method +
                ";\n"
                "\n" +
                servers + "}\n");

            QString filename("./bin/nginx/conf/upstreams/" + poolName + ".conf");

            QFile file(filename);
            if (file.open(QIODevice::ReadWrite | QFile::Truncate)) {
                QTextStream stream(&file);
                stream << upstream << endl;
            }
            file.close();

            qDebug() << "[Nginx][Upstream Config] Saved: " << filename;
        }
    }

    void ConfigurationDialog::createNginxConfUpstreamFolderIfNotExists_And_clearOldConfigs()
    {
        QDir dir("./bin/nginx/conf/upstreams");

        // create Nginx Conf Upstream Folder If Not Exists
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        // delete old upstream configs
        dir.setNameFilters(QStringList() << "*.conf");
        dir.setFilter(QDir::Files);
        foreach (QString dirFile, dir.entryList()) {
            dir.remove(dirFile);
        }
    }

    QJsonValue ConfigurationDialog::serialize_toJSON_Nginx_Upstream_PoolsTable(QTableWidget *pools)
    {
        QJsonObject jsonPools; // 1..n jsonPool's
        QJsonObject jsonPool; // pool key/value pairs

        int rows = pools->rowCount();

        for (int i = 0; i < rows; ++i) {
            QString poolName = pools->item(i, NginxAddUpstreamDialog::Column::Pool)->text();
            QString method   = pools->item(i, NginxAddUpstreamDialog::Column::Method)->text();

            jsonPool.insert("name", poolName);
            jsonPool.insert("method", method);

            // serialize the currently displayed server table
            if (ui->tableWidget_Nginx_Servers->property("servers_of_pool_name") == poolName) {
                qDebug() << "[Nginx][Config] Serializing the Servers Table of Pool:"
                         << ui->tableWidget_Nginx_Servers->property("servers_of_pool_name").toString() << poolName;

                jsonPool.insert("servers", serialize_toJSON_Nginx_Upstream_ServerTable(ui->tableWidget_Nginx_Servers));
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
        QJsonObject jsonServers; // 1..n jsonServer's
        QJsonObject jsonServer; // server key/value pairs

        int rows = servers->rowCount();

        for (int i = 0; i < rows; ++i) {
            jsonServer.insert("address", servers->item(i, 0 /*NginxAddServerDialog::Column::Address*/)->text());
            jsonServer.insert("port", servers->item(i, 1 /*NginxAddServerDialog::Column::Port*/)->text());
            jsonServer.insert("weight", servers->item(i, 2 /*NginxAddServerDialog::Column::Weight*/)->text());
            jsonServer.insert("maxfails", servers->item(i, 3 /*NginxAddServerDialog::Column::MaxFails*/)->text());
            jsonServer.insert("failtimeout", servers->item(i, 4 /*NginxAddServerDialog::Column::FailTimeout*/)->text());
            jsonServer.insert("phpchildren", servers->item(i, 5 /*NginxAddServerDialog::Column::PHPChildren*/)->text());

            jsonServers.insert(QString::number(i), QJsonValue(jsonServer));
        }

        return QJsonValue(jsonServers);
    }

    void ConfigurationDialog::onClickedButtonBoxOk()
    {
        writeSettings();
        toggleRunOnStartup();
    }

    bool ConfigurationDialog::runOnStartUp() const { return (ui->checkbox_runOnStartUp->checkState() == Qt::Checked); }

    void ConfigurationDialog::setRunOnStartUp(bool run) { ui->checkbox_runOnStartUp->setChecked(run); }

    bool ConfigurationDialog::runAutostartServers() const
    {
        return (ui->checkbox_autostartServers->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setAutostartServers(bool run) { ui->checkbox_autostartServers->setChecked(run); }

    void ConfigurationDialog::toggleAutostartServerCheckboxes(bool run)
    {
        // Note: layout doesn't "inject" itself in the parent-child tree, so
        // findChildren() doesn't work

        // left box
        for (int i = 0; i < ui->autostartServersFormLayout->count(); ++i) {
            ui->autostartServersFormLayout->itemAt(i)->widget()->setEnabled(run);
        }

        // right box
        for (int i = 0; i < ui->autostartServersFormLayout2->count(); ++i) {
            ui->autostartServersFormLayout2->itemAt(i)->widget()->setEnabled(run);
        }
    }

    // show only installed items/childs underneath "Components"
    void ConfigurationDialog::hideComponentsNotInstalledInMenuTree()
    {
        installedServersList = servers->getInstalledServerNames();

        // force append xdebug as installed server to unhide it
        installedServersList << "xdebug";

        // get tree item "Components"
        QList<QTreeWidgetItem *> itemList = ui->configMenuTreeWidget->findItems("Components", Qt::MatchFixedString);
        QTreeWidgetItem *components       = itemList[0];

        // iterate over childs
        for (int i = 0; i < components->childCount(); i++) {
            // get child and hide it
            QTreeWidgetItem *childItem = components->child(i);
            childItem->setHidden(true);

            // iterate over installed server names
            // for (int j=0; j < installed.count(); j++)
            QMutableStringListIterator installedML(installedServersList);
            while (installedML.hasNext()) {
                // get servername
                QString serverName = installedML.next();

                // check, if child name is an installed server name
                if (childItem->text(0).contains(serverName, Qt::CaseInsensitive)) {
                    childItem->setHidden(false); // show the item
                    installedML.remove(); // remove from installed server list
                    // qDebug() << "[ConfigDialog][Components] Show Item " << serverName;
                    break;
                }
            }
        }
    }

    void ConfigurationDialog::hideAutostartCheckboxesOfNotInstalledServers()
    {
        QStringList installed = this->servers->getInstalledServerNames();

        QList<QCheckBox *> boxes = ui->tabWidget->findChildren<QCheckBox *>(QRegExp("checkbox_autostart_\\w"));

        for (auto box : boxes) {
            // return last part of "checkbox_autostart_*"
            QString name      = box->objectName().section("_", -1).toLower();
            QString labelName = this->servers->getCamelCasedServerName(name) + "Label";
            QLabel *label     = ui->tabWidget->findChild<QLabel *>(labelName);

            if (installed.contains(name)) {
                qDebug() << "[" + name + "] Autostart Checkbox visible.";
                box->setVisible(true);
                // label->setVisible(true);
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

    void ConfigurationDialog::setClearLogsOnStart(bool run) { ui->checkbox_clearLogsOnStart->setChecked(run); }

    bool ConfigurationDialog::stopServersOnQuit() const
    {
        return (ui->checkbox_stopServersOnQuit->checkState() == Qt::Checked);
    }

    void ConfigurationDialog::setStopServersOnQuit(bool run) { ui->checkbox_stopServersOnQuit->setChecked(run); }

    void ConfigurationDialog::toggleRunOnStartup()
    {
        // Windows %APPDATA% = Roaming ... Programs\Startup
        const QString startupDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + "\\Startup";

        if (runOnStartUp()) {
            // Add WPN-XM SCP shortcut to the Windows Autostart folder.
            // In Windows terminology "shortcuts" are "shell links".
            WindowsAPI::QtWin::CreateShellLink(QApplication::applicationFilePath(), "",
                                               "WPN-XM Server Control Panel", // app, args, desc
                                               QApplication::applicationFilePath(), 0, // icon path and idx
                                               QApplication::applicationDirPath(), // working dir
                                               startupDir + "\\WPN-XM Server Control Panel.lnk" // filepath of shortcut
            );
        } else {
            // remove link
            QFile::remove(startupDir + "\\WPN-XM Server Control Panel.lnk");
        }
    }

    void ConfigurationDialog::fileOpen()
    {
        QString file = QFileDialog::getOpenFileName(this, tr("Select Editor..."), getenv("PROGRAMFILES"),
                                                    tr("Executables (*.exe);;All Files (*)"));

        file = QDir::toNativeSeparators(file);

        ui->lineEdit_SelectedEditor->setText(file);
    }

    void ConfigurationDialog::on_toolButton_SelectEditor_clicked() { ConfigurationDialog::fileOpen(); }

    void ConfigurationDialog::on_toolButton_ResetEditor_clicked()
    {
        ui->lineEdit_SelectedEditor->setText("notepad.exe");
    }

    void ConfigurationDialog::on_pushButton_MongoDb_Reset_Port_clicked()
    {
        ui->lineEdit_mongodb_port->setText("27017");
    }

    void ConfigurationDialog::on_pushButton_MariaDb_Reset_Port_clicked() { ui->lineEdit_mariadb_port->setText("3306"); }

    void ConfigurationDialog::on_pushButton_PostgreSql_Reset_Port_clicked()
    {
        ui->lineEdit_mariadb_port->setText("5432");
    }

    void ConfigurationDialog::on_pushButton_Memcached_Reset_Port_clicked()
    {
        ui->lineEdit_mariadb_port->setText("11211");
    }

    void ConfigurationDialog::on_pushButton_Redis_Reset_Port_clicked() { ui->lineEdit_mariadb_port->setText("6379"); }

    void ConfigurationDialog::on_pushButton_Nginx_Reset_Upstreams_clicked()
    {
        // reset table content
        ui->tableWidget_Nginx_Upstreams->clearContents();
        ui->tableWidget_Nginx_Upstreams->model()->removeRows(0, ui->tableWidget_Nginx_Upstreams->rowCount());

        // insert default data into row
        int row = ui->tableWidget_Nginx_Upstreams->rowCount();
        ui->tableWidget_Nginx_Upstreams->insertRow(row);
        ui->tableWidget_Nginx_Upstreams->setItem(row, NginxAddUpstreamDialog::Column::Pool,
                                                 new QTableWidgetItem("php_pool"));
        ui->tableWidget_Nginx_Upstreams->setItem(row, NginxAddUpstreamDialog::Column::Method,
                                                 new QTableWidgetItem("ip_hash"));
        ui->tableWidget_Nginx_Upstreams->resizeColumnToContents(0);
    }

    void ConfigurationDialog::on_pushButton_Nginx_Reset_Servers_clicked()
    {
        // reset table content
        ui->tableWidget_Nginx_Servers->clearContents();
        ui->tableWidget_Nginx_Servers->model()->removeRows(0, ui->tableWidget_Nginx_Servers->rowCount());

        // insert default data into row
        int row = ui->tableWidget_Nginx_Servers->rowCount();
        ui->tableWidget_Nginx_Servers->insertRow(row);
        ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::Address,
                                               new QTableWidgetItem("127.0.0.1"));
        ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::Port, new QTableWidgetItem("9100"));
        ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::Weight, new QTableWidgetItem("1"));
        ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::MaxFails, new QTableWidgetItem("1"));
        ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::Timeout, new QTableWidgetItem("1s"));
        ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::PHPChildren,
                                               new QTableWidgetItem("5"));
    }

    void ConfigurationDialog::on_pushButton_searchPHPInstallations_clicked()
    {
        ui->comboBox_PHPVersions->setDisabled(true);

        // find all folders inside "bin" containing a php executable
        QStringList filters = {"php.exe"};
        QString binFolder   = QDir::currentPath() + "/bin/";
        QString currentPHPversion;

        QList<PhpVersions> list;

        QDirIterator it(binFolder, filters, QDir::NoSymLinks | QDir::Files | QDir::Dirs, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString php_exe = it.next();

            QFile f(php_exe);
            QString path        = f.fileName();
            QString originalDir = path.section("/", 0, -2);
            QString phpDirName  = path.remove(binFolder).section("/", 0, -2);

            PhpVersions v;
            v.php_dir = phpDirName;
            v.version = getPHPVersionOfExe(php_exe);

            qDebug() << v.php_dir << v.version;

            // rename folders = versionize PHP folders ("php-version")
            QString expectedPhpDirName = QString("php-").append(v.version);

            if (phpDirName == "php") {
                qDebug() << "Skipping bin/php folder.";
                // set current php version to lineEdit
                currentPHPversion = v.version;
                qDebug() << "Current PHP version: " << currentPHPversion;
                ui->lineEdit_currentPHPVersion->setText(v.version);
                continue;
            } else if (phpDirName == expectedPhpDirName) {
                qDebug() << "PHP Folder already correctly versionized.";
            } else {
                QString dest(binFolder);
                dest.append(expectedPhpDirName);
                qDebug() << "dest: " << dest;
                QDir dir;
                if (!dir.rename(originalDir, dest)) {
                    qDebug() << "Error renaming PHP folder.";
                } else {
                    qDebug() << "PHP Folder renamed: " << originalDir << dest;
                }
            }

            list.append(v);
        }

        // sort the php version list
        int i, j;
        for (i = 0; i < list.count(); ++i) {
            for (j = i + 1; j < list.count(); ++j) {
                QVersionNumber version1 = QVersionNumber::fromString(list[i].version);
                QVersionNumber version2 = QVersionNumber::fromString(list[j].version);
                // qDebug() << version1.toString() << version2.toString();
                // TODO: compare WTF: -1,0,1? but i get -8 to 2. thanks qt
                // qDebug() << QVersionNumber::compare(version1, version2);
                if (QVersionNumber::compare(version1, version2) >= 1) {
                    list.move(i, j);
                    i = 0;
                }
            }
        }

        // populate php version dropdown
        ui->comboBox_PHPVersions->clear();
        for (int i = 0; i < list.count(); ++i) {
            ui->comboBox_PHPVersions->addItem(list[i].version);
        }
        ui->comboBox_PHPVersions->setEnabled(true);
    }

    void ConfigurationDialog::on_pushButton_setPHPVersionForBinFolder_clicked()
    {
        QString selectedPHPVersion = ui->comboBox_PHPVersions->currentText();
        QString currentPHPVersion  = ui->lineEdit_currentPHPVersion->text();

        if (ui->lineEdit_currentPHPVersion->text() == selectedPHPVersion) {
            qDebug() << "This PHP version is already set for bin/php.";
        } else {
            QString binFolder          = QDir::currentPath() + "/bin/";
            QString binPHPFolder       = binFolder + "php";
            QString selectedPhpDirName = QString("php-").append(selectedPHPVersion);
            QString currentPhPDirName  = QString("php-").append(currentPHPVersion);

            // rename (current) "bin/php" to "bin/php-version"

            QString current(binFolder);
            current.append(currentPhPDirName);

            // first, we need to remove the "bin/php-version" folder, if it exists
            if (QDir(current).exists()) {
                QDir(current).removeRecursively();
            }

            // second, rename
            QDir dir;
            if (!dir.rename(binPHPFolder, current)) {
                qDebug() << "Error renaming folder.";
            } else {
                qDebug() << "Folder renamed:" << binPHPFolder << "to" << current;
            }

            // rename (selected) "bin/php-version" to "bin/php"

            QString src(binFolder);
            src.append(selectedPhpDirName);
            QDir dir2;
            if (!dir2.rename(src, binPHPFolder)) {
                qDebug() << "Error renaming folder.";
            } else {
                qDebug() << "Folder renamed:" << src << "to" << binPHPFolder;
            }

            ui->lineEdit_currentPHPVersion->setText(selectedPHPVersion);
        }
    }

    QString ConfigurationDialog::getPHPVersionOfExe(QString pathToPHPExecutable)
    {
        // this happens only during testing
        if (!QFile::exists(pathToPHPExecutable)) {
            return "0.0.0";
        }

        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start(pathToPHPExecutable.append(" -n -v"));

        if (!process.waitForFinished()) {
            qDebug() << "[PHP] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readLine();

        // qDebug() << "[PHP] Version: \n" << p_stdout;

        // - grab inside "PHP x (cli)"
        // - "\\d.\\d.\\d." = grab "1.2.3"
        // - "(\\w+\\d+)?" = grab optional "alpha2" version
        QRegExp regex("PHP\\s(\\d.\\d.\\d.(\\w+\\d+)?)");
        regex.indexIn(p_stdout);

        // qDebug() << regex.capturedTexts();

        return regex.cap(1).trimmed();
    }

    void ConfigurationDialog::on_configMenuTreeWidget_clicked(const QModelIndex &index)
    {
        // a click on a menu item returns the name of the item
        // switches to the matching page in the stacked widget
        QString menuitem = ui->configMenuTreeWidget->model()->data(index).toString().toLower().remove(" ");
        setCurrentStackWidget(menuitem);

        // TODO implement per page config loading/saving
    }

    void ConfigurationDialog::setCurrentStackWidget(const QString &widgetname)
    {
        auto *w = ui->stackedWidget->findChild<QWidget *>(widgetname);
        if (w != nullptr)
            ui->stackedWidget->setCurrentWidget(w);
        else
            qDebug() << "[Config Menu] There is no page " << widgetname << " in the stack widget.";
    }

    void ConfigurationDialog::on_pushButton_Nginx_Upstream_AddUpstream_clicked()
    {
        int result;

        auto *dialog = new NginxAddUpstreamDialog();
        dialog->setWindowTitle("Nginx - Add Pool");

        // ui->tableWidget_pools->setSelectionBehavior(QAbstractItemView::SelectRows);
        // ui->tableWidget_pools->setSelectionMode(QAbstractItemView::SingleSelection);

        result = dialog->exec();

        if (result == QDialog::Accepted) {
            int row = ui->tableWidget_Nginx_Upstreams->rowCount();
            ui->tableWidget_Nginx_Upstreams->insertRow(row);
            ui->tableWidget_Nginx_Upstreams->setItem(row, NginxAddUpstreamDialog::Column::Pool,
                                                     new QTableWidgetItem(dialog->pool()));
            ui->tableWidget_Nginx_Upstreams->setItem(row, NginxAddUpstreamDialog::Column::Method,
                                                     new QTableWidgetItem(dialog->method()));
        }

        delete dialog;
    }

    void ConfigurationDialog::on_pushButton_Nginx_Upstream_AddServer_clicked()
    {
        int result;

        auto *dialog = new NginxAddServerDialog();
        dialog->setWindowTitle("Nginx - Add Server");

        ui->tableWidget_Nginx_Servers->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableWidget_Nginx_Servers->setSelectionMode(QAbstractItemView::SingleSelection);

        result = dialog->exec();

        if (result == QDialog::Accepted) {
            int row = ui->tableWidget_Nginx_Servers->rowCount();
            ui->tableWidget_Nginx_Servers->insertRow(row);
            ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::Address,
                                                   new QTableWidgetItem(dialog->address()));
            ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::Port,
                                                   new QTableWidgetItem(dialog->port()));
            ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::Weight,
                                                   new QTableWidgetItem(dialog->weight()));
            ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::MaxFails,
                                                   new QTableWidgetItem(dialog->maxfails()));
            ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::Timeout,
                                                   new QTableWidgetItem(dialog->timeout()));
            ui->tableWidget_Nginx_Servers->setItem(row, NginxAddServerDialog::Column::PHPChildren,
                                                   new QTableWidgetItem(dialog->phpchildren()));
        }

        delete dialog;
    }

    void ConfigurationDialog::loadNginxUpstreams()
    {
        // clear servers table - clear content and remove all rows
        ui->tableWidget_Nginx_Upstreams->setRowCount(0);
        ui->tableWidget_Nginx_Servers->setRowCount(0);

        // load JSON
        QJsonDocument jsonDoc = File::JSON::load("./bin/wpnxm-scp/nginx-upstreams.json");
        QJsonObject json      = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools
        for (QJsonObject::Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter) {
            // The "value" are the key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            // --- Fill Pools Table ---

            // insert new row
            int insertRow = ui->tableWidget_Nginx_Upstreams->rowCount();
            ui->tableWidget_Nginx_Upstreams->insertRow(insertRow);

            // insert column values
            ui->tableWidget_Nginx_Upstreams->setItem(insertRow, NginxAddUpstreamDialog::Column::Pool,
                                                     new QTableWidgetItem(jsonPool["name"].toString()));
            ui->tableWidget_Nginx_Upstreams->setItem(insertRow, NginxAddUpstreamDialog::Column::Method,
                                                     new QTableWidgetItem(jsonPool["method"].toString()));
        }

        // --- Fill Servers Table ---

        // get the first pool, then the "server" key
        QJsonObject jsonPoolFirst = jsonPools.value(QString::number(0)).toObject();

        updateServersTable(jsonPoolFirst);
    }

    void ConfigurationDialog::on_tableWidget_Upstream_itemSelectionChanged()
    {
        // there is a selection, but its not a row selection
        if (ui->tableWidget_Nginx_Upstreams->selectionModel()->selectedRows(0).size() <= 0) {
            return;
        }

        // get "pool" from selection
        QString selectedUpstreamName =
            ui->tableWidget_Nginx_Upstreams->selectionModel()->selectedRows().first().data().toString();

        // there is a selection, but the selection is already the currently displayed
        // table view
        if (ui->tableWidget_Nginx_Servers->property("servers_of_pool_name") == selectedUpstreamName) {
            return;
        }

        // get the pool and update servers table
        QJsonObject jsonPool = getNginxUpstreamPoolByName(selectedUpstreamName);
        updateServersTable(jsonPool);
    }

    void ConfigurationDialog::updateServersTable(QJsonObject jsonPool)
    {
        // clear servers table - clear content and remove all rows
        ui->tableWidget_Nginx_Servers->setRowCount(0);

        // set new "pool name" as table property (table view identifier)
        ui->tableWidget_Nginx_Servers->setProperty("servers_of_pool_name", jsonPool["name"].toString());

        // key "servers"
        QJsonObject jsonServers = jsonPool["servers"].toObject();

        for (int i = 0; i < jsonServers.count(); ++i) {
            // values for a "server"
            QJsonObject values = jsonServers.value(QString::number(i)).toObject();

            // insert new row
            int insertRow = ui->tableWidget_Nginx_Servers->rowCount();
            ui->tableWidget_Nginx_Servers->insertRow(insertRow);

            // insert column values
            ui->tableWidget_Nginx_Servers->setItem(insertRow, 0 /*NginxAddServerDialog::Column::Address*/,
                                                   new QTableWidgetItem(values["address"].toString()));
            ui->tableWidget_Nginx_Servers->setItem(insertRow, 1 /*NginxAddServerDialog::Column::Port*/,
                                                   new QTableWidgetItem(values["port"].toString()));
            ui->tableWidget_Nginx_Servers->setItem(insertRow, 2 /*NginxAddServerDialog::Column::Weight*/,
                                                   new QTableWidgetItem(values["weight"].toString()));
            ui->tableWidget_Nginx_Servers->setItem(insertRow, 3 /*NginxAddServerDialog::Column::MaxFails*/,
                                                   new QTableWidgetItem(values["maxfails"].toString()));
            ui->tableWidget_Nginx_Servers->setItem(insertRow, 4 /*NginxAddServerDialog::Column::FailTimeout*/,
                                                   new QTableWidgetItem(values["failtimeout"].toString()));
            ui->tableWidget_Nginx_Servers->setItem(insertRow, 5 /*NginxAddServerDialog::Column::PHPChildren*/,
                                                   new QTableWidgetItem(values["phpchildren"].toString()));
        }
    }

    QJsonObject ConfigurationDialog::getNginxUpstreamPoolByName(const QString &poolName)
    {
        // load JSON
        QJsonDocument jsonDoc = File::JSON::load("./bin/wpnxm-scp/nginx-upstreams.json");
        QJsonObject json      = jsonDoc.object();
        QJsonObject jsonPools = json["pools"].toObject();

        // iterate over 1..n pools
        for (QJsonObject::Iterator iter = jsonPools.begin(); iter != jsonPools.end(); ++iter) {
            // "value" is key/value pairs of a pool
            QJsonObject jsonPool = iter.value().toObject();

            // key "name" = poolName
            if (jsonPool["name"].toString() == poolName) {
                return jsonPool;
            }
        }

        return QJsonObject();
    }

    bool ConfigurationDialog::getSettingBool(const QString &key, const QVariant &defaultValue)
    {
        return settings->get(key, defaultValue).toBool();
    }

    int ConfigurationDialog::getSettingInt(const QString &key, const QVariant &defaultValue)
    {
        return settings->get(key, defaultValue).toInt();
    }

    QString ConfigurationDialog::getSettingString(const QString &key, const QVariant &defaultValue)
    {
        return settings->get(key, defaultValue).toString();
    }

    bool ConfigurationDialog::getSettingBool(const QString &key, const bool &defaultValue)
    {
        return settings->get(key, QVariant(defaultValue)).toBool();
    }

    int ConfigurationDialog::getSettingInt(const QString &key, const int &defaultValue)
    {
        return settings->get(key, QVariant(defaultValue)).toInt();
    }

    QString ConfigurationDialog::getSettingString(const QString &key, const QString &defaultValue)
    {
        return settings->get(key, QVariant(defaultValue)).toString();
    }
} // namespace Configuration
