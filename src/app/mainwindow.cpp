#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace ServerControlPanel
{

    MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new ServerControlPanel::Ui::MainWindow)
    {
        ui->setupUi(this);

        // The tray icon is an instance of the QSystemTrayIcon class.
        // Check, whether a system tray is present on the user's desktop.
        if (!QSystemTrayIcon::isSystemTrayAvailable()) {
            QMessageBox::critical(nullptr, APP_NAME, tr("You don't have a system tray."));
            QApplication::exit(1);
        }

        setWindowTitle(APP_NAME_AND_VERSION);

        setDefaultSettings();

        // start minimized to tray
        if (settings->get("global/startminimized").toBool()) {
            setWindowState(Qt::WindowMinimized);
        } else {
            // maximize and move window to the top
            setFocus();
            setWindowState(windowState() & (~Qt::WindowMinimized | Qt::WindowActive | Qt::WindowMaximized));
        }

        // disable Maximize button functionality
        setWindowFlags((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

        // set window size fixed
        setFixedSize(width(), height());

        // TODO unhide scheduler - feature not ready yet. hide button for now
        ui->pushButton_Scheduler->hide();

        // TODO unhide updater - feature not ready yet. hide button for now
        ui->pushButton_Updater->hide();

        connect(this, SIGNAL(mainwindow_show()), this, SLOT(MainWindow_ShowEvent()));
    }

    void MainWindow::setup()
    {
        servers = new Servers::Servers();

        createTrayIcon();

        renderServerStatusPanel();

        updateServerStatusIndicatorsForAlreadyRunningServers();

        createActions();

        showPushButtonsOnlyForInstalledTools();

        // Status Table - Column Status
        // if process state of a server changes, then change the label status in
        // UI::MainWindow too
        connect(servers, SIGNAL(signalMainWindow_ServerStatusChange(QString, bool)), this,
                SLOT(updateServerStatusIndicators(QString, bool)));

        // server autostart
        if (settings->get("global/autostartservers").toBool()) {
            qDebug() << "[Servers] Autostart enabled";
            autostartServers();
        };

        updateTrayIconTooltip();
        updateToolsPushButtons();

#ifdef QT_DEBUG
        if (settings->get("selfupdater/runonstartup").toBool()) {
            runSelfUpdate();
        }

// ui->pushButton_Updater->setEnabled(true);
#endif
    }

    MainWindow::~MainWindow()
    {
        // stop all servers, when quitting the tray application
        if (settings->get("global/stopserversonquit").toBool()) {
            qDebug() << "[Servers] Stopping All Servers on Quit...";
            stopAllServers();
        }

        delete ui;
        delete tray;
    }

    MainWindow *MainWindow::getMainWindow() { return this; }

    void MainWindow::setProcessesInstance(Processes::Processes *oProcesses) { processes = oProcesses; }

    Processes::Processes *MainWindow::getProcessesObject() { return processes; }

    void MainWindow::setPluginManagerInstance(Plugins::PluginManager *oPluginManager)
    {
        pluginManager = oPluginManager;
    }

    Plugins::PluginManager *MainWindow::getPluginManager() { return pluginManager; }

    void MainWindow::updateServerStatusIndicatorsForAlreadyRunningServers()
    {
        QStringList installedServers = servers->getInstalledServerNames();
        installedServers << "php-cgi";

        QStringList alreadyUpdated;

        foreach (const Processes::Processes::Process &process, processes->monitoredProcessesList) {
            QString processName = process.name.section(".", 0, 0);
            // qDebug() << Q_FUNC_INFO << processName;

            if (installedServers.contains(processName) && !alreadyUpdated.contains(processName)) {
                qDebug() << "[Processes Running][updateServerStatusIndicators]"
                            ""
                         << "for Process" << processName;

                alreadyUpdated << processName;
                updateServerStatusIndicators(processName, true);
            }
        }
    }

    void MainWindow::runSelfUpdate()
    {
        selfUpdater = new Updater::SelfUpdater();
        connect(selfUpdater, SIGNAL(notifyUpdateAvailable(QJsonObject)), this,
                SLOT(show_SelfUpdater_UpdateNotification(QJsonObject)));
        connect(selfUpdater, SIGNAL(notifyRestartNeeded(QJsonObject)), this,
                SLOT(show_SelfUpdater_RestartNeededNotification(QJsonObject)));
        selfUpdater->run();
    }

    // TODO move to Notification Class
    void MainWindow::show_SelfUpdater_UpdateNotification(QJsonObject versionInfo)
    {
        // when autoupdate is
        // - false, ask the user, if he wants to update (dialogbox)
        // - true, show tray notification (that update is in progress)

        if (!settings->get("selfupdater/autoupdate").toBool()) {
            // the timer is used to wait, until the SplashScreen is gone
            QTimer::singleShot(2000, selfUpdater, SLOT(askForUpdate()));
            return;
        }

        QString softwareNameAndLatestVersion =
            QString("%1 v%2").arg(versionInfo["software_name"].toString(), versionInfo["latest_version"].toString());
        QString title("Update available:\n");
        QString msg(softwareNameAndLatestVersion);

        tray->showMessage(title, msg);
    }

    // TODO move to Notification Class
    // if "selfupdater/autorestart" is on, just show a tray info
    void MainWindow::show_SelfUpdater_RestartNeededNotification(QJsonObject versionInfo)
    {
        QString title("Update successful! Restarting...\n");
        QString msg("Version v" + versionInfo["latest_version"].toString());
        tray->showMessage(title, msg);
    }

    void MainWindow::createTrayIcon()
    {
        tray = new ServerControlPanel::Tray(qApp, servers);

        // handle clicks on the tray icon
        connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
                SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

        tray->show();
    }

    void MainWindow::createActions()
    {
        // title bar - minimize
        minimizeAction = new QAction(tr("Mi&nimize"), this);
        connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

        // title bar - restore
        restoreAction = new QAction(tr("&Restore"), this);
        connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

        // title bar - close
        // Note that this action is intercepted by MainWindow::closeEvent()
        // Its modified from "quit" to "close to tray" with a msgbox
        quitAction = new QAction(tr("&Quit"), this);
        connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApplication()));

        QWidget *cWidget = ui->centralWidget;

        // Connect Actions for Status Table - Column Action (Start)
        connect(cWidget->findChild<QPushButton *>("pushButton_Start_Nginx"), SIGNAL(clicked()), servers,
                SLOT(startNginx()));
        connect(cWidget->findChild<QPushButton *>("pushButton_Start_PHP"), SIGNAL(clicked()), servers,
                SLOT(startPHP()));
        connect(cWidget->findChild<QPushButton *>("pushButton_Start_MariaDb"), SIGNAL(clicked()), servers,
                SLOT(startMariaDb()));
        QPushButton *buttonStartMongoDb = cWidget->findChild<QPushButton *>("pushButton_Start_MongoDb");
        if (buttonStartMongoDb != nullptr) {
            connect(buttonStartMongoDb, SIGNAL(clicked()), servers, SLOT(startMongoDb()));
        }
        QPushButton *buttonStartMemcached = cWidget->findChild<QPushButton *>("pushButton_Start_Memcached");
        if (buttonStartMemcached != nullptr) {
            connect(buttonStartMemcached, SIGNAL(clicked()), servers, SLOT(startMemcached()));
        }
        QPushButton *buttonStartPostgreSQL = cWidget->findChild<QPushButton *>("pushButton_Start_PostgreSQL");
        if (buttonStartPostgreSQL != nullptr) {
            connect(buttonStartPostgreSQL, SIGNAL(clicked()), servers, SLOT(startPostgreSQL()));
        }
        QPushButton *buttonStartRedis = cWidget->findChild<QPushButton *>("pushButton_Start_Redis");
        if (buttonStartRedis != nullptr) {
            connect(buttonStartRedis, SIGNAL(clicked()), servers, SLOT(startRedis()));
        }

        // Connect Actions for Status Table - Column Action (Stop)
        connect(cWidget->findChild<QPushButton *>("pushButton_Stop_Nginx"), SIGNAL(clicked()), servers,
                SLOT(stopNginx()));
        connect(cWidget->findChild<QPushButton *>("pushButton_Stop_PHP"), SIGNAL(clicked()), servers, SLOT(stopPHP()));
        connect(cWidget->findChild<QPushButton *>("pushButton_Stop_MariaDb"), SIGNAL(clicked()), servers,
                SLOT(stopMariaDb()));
        QPushButton *buttonStopMongoDb = cWidget->findChild<QPushButton *>("pushButton_Stop_MongoDb");
        if (buttonStopMongoDb != nullptr) {
            connect(buttonStopMongoDb, SIGNAL(clicked()), servers, SLOT(stopMongoDb()));
        }
        QPushButton *buttonStopMemcached = cWidget->findChild<QPushButton *>("pushButton_Stop_Memcached");
        if (buttonStopMemcached != nullptr) {
            connect(buttonStopMemcached, SIGNAL(clicked()), servers, SLOT(stopMemcached()));
        }
        QPushButton *buttonStopPostgreSQL = cWidget->findChild<QPushButton *>("pushButton_Stop_PostgreSQL");
        if (buttonStopPostgreSQL != nullptr) {
            connect(buttonStopPostgreSQL, SIGNAL(clicked()), servers, SLOT(stopPostgreSQL()));
        }
        QPushButton *buttonStopRedis = cWidget->findChild<QPushButton *>("pushButton_Stop_Redis");
        if (buttonStopRedis != nullptr) {
            connect(buttonStopRedis, SIGNAL(clicked()), servers, SLOT(stopRedis()));
        }

        // Connect Actions for Status Table - AllServers Start, Stop
        connect(ui->pushButton_AllServers_Start, SIGNAL(clicked()), this, SLOT(startAllServers()));
        connect(ui->pushButton_AllServers_Stop, SIGNAL(clicked()), this, SLOT(stopAllServers()));

        // PushButtons: Website, Forum, Help, About, ReportBug, Donate
        connect(ui->pushButton_Website, SIGNAL(clicked()), this, SLOT(goToWebsite()));
        connect(ui->pushButton_Forum, SIGNAL(clicked()), this, SLOT(goToForum()));
        connect(ui->pushButton_Help, SIGNAL(clicked()), this, SLOT(openHelpDialog()));
        connect(ui->pushButton_About, SIGNAL(clicked()), this, SLOT(openAboutDialog()));
        connect(ui->pushButton_ReportBug, SIGNAL(clicked()), this, SLOT(goToReportIssue()));
        connect(ui->pushButton_Donate, SIGNAL(clicked()), this, SLOT(goToDonate()));

        // PushButtons: Configuration, Help, About, Close
        connect(ui->pushButton_Console, SIGNAL(clicked()), this, SLOT(openConsole()));
        connect(ui->pushButton_Configuration, SIGNAL(clicked()), this, SLOT(openConfigurationDialog()));
        connect(ui->pushButton_Processes, SIGNAL(clicked()), this, SLOT(openProcessViewerDialog()));

        // clicking Close, does not quit, but closes the window to tray
        connect(ui->pushButton_Close, SIGNAL(clicked()), this, SLOT(hide()));

        // Actions - Tools
        connect(ui->pushButton_tools_phpinfo, SIGNAL(clicked()), this, SLOT(openToolPHPInfo()));
        connect(ui->pushButton_tools_phpmyadmin, SIGNAL(clicked()), this, SLOT(openToolPHPMyAdmin()));
        connect(ui->pushButton_tools_webgrind, SIGNAL(clicked()), this, SLOT(openToolWebgrind()));
        connect(ui->pushButton_tools_adminer, SIGNAL(clicked()), this, SLOT(openToolAdminer()));
        connect(ui->pushButton_tools_robomongo, SIGNAL(clicked()), this, SLOT(openToolRobomongo()));

        // Actions - Open Projects Folder
        connect(ui->pushButton_OpenProjects_Browser, SIGNAL(clicked()), this, SLOT(openProjectFolderInBrowser()));
        connect(ui->pushButton_OpenProjects_Explorer, SIGNAL(clicked()), this, SLOT(openProjectFolderInExplorer()));

        // Actions - Status Table

        // Connect Configure Buttons
        // clicking the configure icon opens the config tab of this server
        connect(cWidget->findChild<QPushButton *>("pushButton_Configure_Nginx"), SIGNAL(clicked()), this,
                SLOT(openConfigurationDialogNginx()));
        connect(cWidget->findChild<QPushButton *>("pushButton_Configure_PHP"), SIGNAL(clicked()), this,
                SLOT(openConfigurationDialogPHP()));
        connect(cWidget->findChild<QPushButton *>("pushButton_Configure_MariaDb"), SIGNAL(clicked()), this,
                SLOT(openConfigurationDialogMariaDb()));

        QPushButton *btnConfigureMongoDb = cWidget->findChild<QPushButton *>("pushButton_Configure_MongoDb");
        if (btnConfigureMongoDb != nullptr) {
            connect(btnConfigureMongoDb, SIGNAL(clicked()), this, SLOT(openConfigurationDialogMongoDb()));
        }

        QPushButton *btnConfigureMemcached = cWidget->findChild<QPushButton *>("pushButton_Configure_Memcached");
        if (btnConfigureMemcached != nullptr) {
            connect(btnConfigureMemcached, SIGNAL(clicked()), this, SLOT(openConfigurationDialogMemcached()));
        }

        QPushButton *btnConfigurePostgresql = cWidget->findChild<QPushButton *>("pushButton_Configure_PostgreSQL");
        if (btnConfigurePostgresql != nullptr) {
            connect(btnConfigurePostgresql, SIGNAL(clicked()), this, SLOT(openConfigurationDialogPostgresql()));
        }

        QPushButton *btnConfigureRedis = cWidget->findChild<QPushButton *>("pushButton_Configure_Redis");
        if (btnConfigureRedis != nullptr) {
            connect(btnConfigureRedis, SIGNAL(clicked()), this, SLOT(openConfigurationDialogRedis()));
        }
    }

    void MainWindow::changeEvent(QEvent *event)
    {
        if (nullptr != event) {
            switch (event->type()) {
            case QEvent::WindowStateChange: {
                // minimize to tray (do not minimize to taskbar)
                if ((this->windowState() & Qt::WindowMinimized) && !this->isHidden()) {
                    this->hide();
                }

                break;
            }
            default:
                break;
            }
        }

        QMainWindow::changeEvent(event);
    }

    void MainWindow::MainWindow_ShowEvent()
    {
        /**
         * Only show the log file icons/buttons, if the respective file exists.
         */

        // Set enabled/disabled state for all "pushButton_ShowLog_*" buttons
        QList<QPushButton *> allShowLogPushButtons =
            ui->centralWidget->findChildren<QPushButton *>(QRegExp("pushButton_ShowLog_\\w"));

        for (auto &allShowLogPushButton : allShowLogPushButtons) {
            allShowLogPushButton->setEnabled(QFile::exists(this->getLogfile(allShowLogPushButton->objectName())));
        }

        // Set enabled/disabled state for all "pushButton_ShowErrorLog_*" buttons
        QList<QPushButton *> allShowErrorLogPushButtons =
            ui->centralWidget->findChildren<QPushButton *>(QRegExp("pushButton_ShowErrorLog_\\w"));

        for (auto &allShowErrorLogPushButton : allShowErrorLogPushButtons) {
            allShowErrorLogPushButton->setEnabled(
                QFile::exists(this->getLogfile(allShowErrorLogPushButton->objectName())));
        }
    }

    void MainWindow::showEvent(QShowEvent *event)
    {
        QMainWindow::showEvent(event);
        emit mainwindow_show();
    }

    void MainWindow::closeEvent(QCloseEvent *event)
    {
        if (tray->isVisible()) {

            bool doNotAskAgainCloseToTray = settings->get("global/donotaskagainclosetotray").toBool();

            if (!doNotAskAgainCloseToTray) {
                QCheckBox *checkbox = new QCheckBox(tr("Do not show this message again."), this);
                checkbox->setChecked(doNotAskAgainCloseToTray);

                QMessageBox msgbox(this);
                msgbox.setWindowTitle(QApplication::applicationName());
                msgbox.setIconPixmap(QMessageBox::standardIcon(QMessageBox::Information));
                msgbox.setText(
                    tr("This program will keep running in the system tray.<br>"
                       "To terminate the program, choose <b>Quit</b> in the "
                       "context menu of the system tray."));
                msgbox.setCheckBox(checkbox);
                msgbox.exec();

                settings->set("global/donotaskagainclosetotray", int(msgbox.checkBox()->isChecked()));
            }

            // hide mainwindow
            hide();

            // do not propagate the event to the base class
            event->ignore();
        }
        event->accept();
    }

    void MainWindow::setVisible(bool visible)
    {
        minimizeAction->setEnabled(visible);
        restoreAction->setEnabled(isMaximized() || !visible);
        QMainWindow::setVisible(visible);
    }

    void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
    {
        switch (reason) {
        // Double click toggles dialog display
        case QSystemTrayIcon::DoubleClick:
            if (isVisible())
                // clicking the tray icon, when the main window is shown, hides it
                setVisible(false);
            else {
                // clicking the tray icon, when the main window is hidden, shows the main
                // window
                show();
                setFocus();
                setWindowState(windowState() & (~Qt::WindowMinimized | Qt::WindowActive | Qt::WindowMaximized));
                activateWindow();
            }
            break;
        case QSystemTrayIcon::Context:
            if (tray->isTooltipVisible()) {
                tray->hideTooltip();
            }
            break;
        default:
            return;
        }
    }

    void MainWindow::updateServerStatusIndicators(const QString &server, bool enabled)
    {
        updateVersion(server);

        QString srv = server.toLower();

        updateLabelStatus(srv, enabled);

        updateTrayIconTooltip();

        if (srv == "nginx" || srv == "php" || srv == "php-cgi") {
            updateToolsPushButtons();
        }

        updateServerStatusOnTray(srv, enabled);

        updatePort(srv, enabled);
    }

    void MainWindow::enableToolsPushButtons(bool enabled)
    {
        // get all PushButtons from the Tools GroupBox of MainWindow::UI
        QList<QPushButton *> buttons = ui->ToolsGroupBox->findChildren<QPushButton *>();

        // set all PushButtons enabled/disabled
        for (auto &button : buttons) {
            button->setEnabled(enabled);
        }

        // change state of "Open Projects Folder" >> "Browser" button
        ui->pushButton_OpenProjects_Browser->setEnabled(enabled);

        // disable Action "Webinterface" in TrayMenu, when PHP+Nginx are off
        QList<QAction *> actions = tray->contextMenu()->actions();
        const int listSize       = actions.size();
        for (int i = 0; i < listSize; ++i) {
            if (actions.at(i)->iconText() == tr("Webinterface")) {
                actions.at(i)->setEnabled(enabled);
            }
        }
    }

    void MainWindow::showPushButtonsOnlyForInstalledTools()
    {
        // get all PushButtons from the Tools GroupBox of MainWindow::UI
        QList<QPushButton *> buttons = ui->ToolsGroupBox->findChildren<QPushButton *>();

        // set all PushButtons invisible
        for (auto &button : buttons) {
            button->setVisible(false);
        }

        // if "component" exists in "tools" or "bin" directory, show pushButtons in
        // the Tools Groupbox
        if (QDir(getProjectFolder() + "/tools/webinterface").exists()) {
            ui->pushButton_tools_phpinfo->setVisible(true);
        }
        if (QDir(getProjectFolder() + "/tools/phpmyadmin").exists()) {
            ui->pushButton_tools_phpmyadmin->setVisible(true);
        }
        if (QDir(getProjectFolder() + "/tools/adminer").exists()) {
            ui->pushButton_tools_adminer->setVisible(true);
        }
        if (QDir(getProjectFolder() + "/tools/webgrind").exists()) {
            ui->pushButton_tools_webgrind->setVisible(true);
        }
        if (QDir(getProjectFolder() + "/bin/robomongo").exists()) {
            ui->pushButton_tools_robomongo->setVisible(true);
        }
    }

    void MainWindow::updateLabelStatus(const QString &server, bool enabled)
    {
        if (server == "nginx") {
            ui->centralWidget->findChild<QLabel *>("label_Nginx_Status")->setEnabled(enabled);
        }
        if (server == "php" || server == "php-cgi") {
            ui->centralWidget->findChild<QLabel *>("label_PHP_Status")->setEnabled(enabled);
        }
        if (server == "mariadb" || server == "mysqld") {
            ui->centralWidget->findChild<QLabel *>("label_MariaDb_Status")->setEnabled(enabled);
        }
        if (server == "mongodb") {
            ui->centralWidget->findChild<QLabel *>("label_MongoDb_Status")->setEnabled(enabled);
        }
        if (server == "memcached") {
            ui->centralWidget->findChild<QLabel *>("label_Memcached_Status")->setEnabled(enabled);
        }
        if (server == "postgresql" || server == "postgres") {
            ui->centralWidget->findChild<QLabel *>("label_PostgreSQL_Status")->setEnabled(enabled);
        }
        if (server == "redis" || server == "redis-server") {
            ui->centralWidget->findChild<QLabel *>("label_Redis_Status")->setEnabled(enabled);
        }
    }

    void MainWindow::updateServerStatusOnTray(const QString &serverName, bool enabled)
    {
        QIcon icon;
        if (enabled) {
            icon = QIcon(":/status_run");
        } else {
            icon = QIcon(":/status_stop");
        }

        QList<QAction *> actions = tray->contextMenu()->actions();

        const int listSize = actions.size();
        for (int i = 0; i < listSize; ++i) {
            QAction *action = actions.at(i);
            if (action->iconText().toLower() == serverName) {
                action->setIcon(icon);
            }
        }
    }

    void MainWindow::updateTrayIconTooltip()
    {
        QString tip     = "";
        QWidget *widget = ui->centralWidget;

        if (widget->findChild<QLabel *>("label_Nginx_Status")->isEnabled()) {
            tip.append("Nginx: running\n");
        }
        if (widget->findChild<QLabel *>("label_PHP_Status")->isEnabled()) {
            tip.append("PHP: running\n");
        }
        if (widget->findChild<QLabel *>("label_MariaDb_Status")->isEnabled()) {
            tip.append("MariaDb: running\n");
        }
        if (widget->findChild<QLabel *>("label_MongoDb_Status") &&
            widget->findChild<QLabel *>("label_MongoDb_Status")->isEnabled()) {
            tip.append("MongoDb: running\n");
        }
        if (widget->findChild<QLabel *>("label_Memcached_Status") &&
            widget->findChild<QLabel *>("label_Memcached_Status")->isEnabled()) {
            tip.append("Memcached: running\n");
        }
        if (widget->findChild<QLabel *>("label_PostgreSQL_Status") &&
            widget->findChild<QLabel *>("label_PostgreSQL_Status")->isEnabled()) {
            tip.append("PostgreSQL: running\n");
        }
        if (widget->findChild<QLabel *>("label_Redis_Status") &&
            widget->findChild<QLabel *>("label_Redis_Status")->isEnabled()) {
            tip.append("Redis: running\n");
        }

        tray->setMessage(tip);
    }

    void MainWindow::updateToolsPushButtons()
    {
        QWidget *widget = ui->centralWidget;

        if (widget->findChild<QLabel *>("label_Nginx_Status")->isEnabled() &&
            widget->findChild<QLabel *>("label_PHP_Status")->isEnabled()) {
            enableToolsPushButtons(true);
        } else {
            enableToolsPushButtons(false);
        }
    }

    void MainWindow::quitApplication()
    {
        if (settings->get("global/stopserversonquit").toBool()) {
            qDebug() << "[Servers] Stopping on Quit...\n";
            stopAllServers();
        }
        QApplication::quit();
    }

    QString MainWindow::getNginxVersion()
    {
        // this happens only during testing
        if (!QFile::exists("./bin/nginx/nginx.exe")) {
            return "0.0.0";
        }

        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start("./bin/nginx/nginx.exe -v");

        if (!process.waitForFinished()) {
            qDebug() << "[Nginx] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readAll();

        // string for regexp testing
        // QString p_stdout = "nginx version: nginx/1.2.1";

        // qDebug() << "[Nginx] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout);
    }

    QString MainWindow::getMariaVersion()
    {
        // this happens only during testing
        if (!QFile::exists("./bin/mariadb/bin/mysqld.exe") || !QFile::exists("./bin/mariadb/bin/mysqlcheck.exe")) {
            return "0.0.0";
        }

        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start("./bin/mariadb/bin/mysqlcheck.exe -V"); // upper-case V

        if (!process.waitForFinished()) {
            qDebug() << "[MariaDb] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readAll();

        // string for regexp testing
        // QString p_stdout = ".\\bin\\mariadb\\bin\\mysqlcheck.exe  Ver 15.1 Distrib
        // 5.5.24-MariaDB, for Win32 (x86)";
        // QString p_stdout = ".\\bin\\mariadb\\bin\\mysqlcheck.exe  Ver 2.7.4-MariaDB
        // Distrib 10.1.6-MariaDB, for Win64 (AMD64)\r\n";

        // qDebug() << "[MariaDb] Version: \n" << p_stdout;

        // scrape second version number
        return parseVersionNumber(p_stdout.mid(p_stdout.lastIndexOf("Distrib "), 15));
    }

    QString MainWindow::getPHPVersion()
    {
        // this happens only during testing
        if (!QFile::exists("./bin/php/php.exe")) {
            return "0.0.0";
        }

        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start("./bin/php/php.exe -n -v");

        if (!process.waitForFinished()) {
            qDebug() << "[PHP] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readLine();

        // string for regexp testing
        // QString p_stdout = "PHP 5.4.3 (cli) (built: Feb 29 2012 19:06:50)";
        // QString p_stdout = "PHP 7.0.0alpha2 (cli)  (non semantic version)";
        // QString p_stdout = "PHP 7.1.1 (cli) (built: Jan 18 2017 18:50:48)";

        // qDebug() << "[PHP] Version: \n" << p_stdout;

        // - grab inside "PHP x (cli)"
        // - "\\d.\\d.\\d." = grab "1.2.3"
        // - "(\\w+\\d+)?" = grab optional "alpha2" version
        QRegExp regex("PHP\\s(\\d.\\d.\\d.(\\w+\\d+)?)");
        regex.indexIn(p_stdout);

        // qDebug() << regex.capturedTexts();

        return regex.cap(1);
    }

    QString MainWindow::getMongoVersion()
    {
        QProcess process;
        process.start("./bin/mongodb/bin/mongod.exe --version");

        if (!process.waitForFinished()) {
            qDebug() << "[MongoDB] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readLine();

        // qDebug() << "[MongoDb] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout.mid(3)); // 21
    }

    QString MainWindow::getPostgresqlVersion()
    {
        QProcess process;
        process.start("./bin/pgsql/bin/pg_ctl.exe -V");

        if (!process.waitForFinished()) {
            qDebug() << "[PostgreSQL] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readAll();

        // qDebug() << "[PostgreSQL] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout.mid(2)); // 10
    }

    QString MainWindow::getMemcachedVersion()
    {
        QProcess process;
        process.start("./bin/memcached/memcached.exe -h");

        if (!process.waitForFinished()) {
            qDebug() << "[Memcached] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readLine();

        // qDebug() << "[Memcached] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout.mid(2)); // 10
    }

    QString MainWindow::getRedisVersion()
    {
        // notes: --version doesn't work
        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        QString redisExe = getRootFolder() + "\\bin\\redis\\redis-cli.exe --help";
        process.start(redisExe);

        if (!process.waitForFinished()) {
            qDebug() << "[Redis] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readLine();

        // qDebug() << "[Redis] Version: \n" << p_stdout;

        // Redis server v=2.8.21 sha
        return parseVersionNumber(p_stdout);
    }

    QString MainWindow::parseVersionNumber(const QString &stringWithVersion)
    {
        // This RegExp matches version numbers: (\d+\.)?(\d+\.)?(\d+\.)?(\*|\d+)
        // This is the same, but escaped:
        QRegExp regex("(\\d+\\.)?(\\d+\\.)?(\\d+\\.)?(\\*|\\d+)");

        regex.indexIn(stringWithVersion);

        return regex.cap(0);

        // Leave this for debugging purposes
        //    int pos = 0;
        //    while((pos = regex.indexIn(stringWithVersion, pos)) != -1)
        //    {
        //        qDebug() << "Match at pos " << pos
        //                 << " with length " << regex.matchedLength()
        //                 << ", captured = " <<
        //                 regex.capturedTexts().at(0).toLatin1().data()
        //                 << ".\n";
        //        pos += regex.matchedLength();
        //    }
    }

    //*
    //* Action slots
    //*
    void MainWindow::startAllServers()
    {
        servers->startNginx();
        servers->startPHP();
        servers->startMariaDb();
        servers->startMongoDb();
        servers->startMemcached();
        servers->startPostgreSQL();
        servers->startRedis();

        if (settings->get("global/OnStartAllOpenWebinterface").toBool()) {
            openWebinterface();
        }

        if (settings->get("global/OnStartAllMinimize").toBool()) {
            setWindowState(Qt::WindowMinimized);
        }
    }

    void MainWindow::stopAllServers()
    {
        servers->stopNginx();
        servers->stopPHP();
        servers->stopMariaDb();
        servers->stopMongoDb();
        servers->stopMemcached();
        servers->stopPostgreSQL();
        servers->stopRedis();
    }

    void MainWindow::goToWebsite() { QDesktopServices::openUrl(QUrl("https://wpn-xm.org/")); }

    void MainWindow::goToForum() { QDesktopServices::openUrl(QUrl("https://forum.wpn-xm.org/")); }

    void MainWindow::goToReportIssue() { QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/issues/")); }

    void MainWindow::goToDonate() { QDesktopServices::openUrl(QUrl("https://wpn-xm.org/#donate")); }

    void MainWindow::openToolPHPInfo() { QDesktopServices::openUrl(QUrl("https://wpn.xm/index.php?page=phpinfo")); }

    void MainWindow::openToolPHPMyAdmin() { QDesktopServices::openUrl(QUrl("http://localhost/tools/phpmyadmin/")); }

    void MainWindow::openToolWebgrind() { QDesktopServices::openUrl(QUrl("http://localhost/tools/webgrind/")); }

    void MainWindow::openToolAdminer()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/adminer/adminer.php"));
    }

    void MainWindow::openToolRobomongo()
    {
        QString command("./bin/robomongo/robomongo.exe");

        if (!QFile::exists(command)) {
            return;
        }

        QProcess::startDetached(command);
    }

    void MainWindow::openWebinterface() { QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/")); }

    void MainWindow::openProjectFolderInBrowser()
    {
        // we have to take the NGINX port into account
        QString nginxPort = settings->get("nginx/port").toString();

        QString url = "http://localhost";

        if (nginxPort != "80") {
            url.append(":" + nginxPort + "/");
        }

        QDesktopServices::openUrl(QUrl(url));
    }

    void MainWindow::openProjectFolderInExplorer()
    {
        if (QDir(getProjectFolder()).exists()) {
            // exec explorer with path to projects
            QDesktopServices::openUrl(QUrl("file:///" + getProjectFolder(), QUrl::TolerantMode));
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("The projects folder was not found."));
        }
    }

    void MainWindow::openConsole()
    {
        QString cmd, conemu;

        if (qgetenv("PROCESSOR_ARCHITECTURE") == "x86") {
            conemu = "./bin/conemu/conemu.exe";
        } else {
            conemu = "./bin/conemu/conemu64.exe";
        }

        // prefer "ConEmu", else fallback to "Windows Console"
        if (QFile::exists(conemu)) {
            cmd = conemu + " /LoadCfgFile ./bin/conemu/ConEmu.xml";
        } else {
            cmd = "cmd.exe";
        }

        QProcess::startDetached(cmd);
    }

    QString MainWindow::getRootFolder() const { return QDir::toNativeSeparators(QDir::currentPath()); }
    QString MainWindow::getProjectFolder() const { return QDir::toNativeSeparators(QDir::currentPath() + "/www"); }

    void MainWindow::openConfigurationDialog()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(servers);
        cfgDlg.exec();
    }

    void MainWindow::openConfigurationDialogNginx()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(servers);
        cfgDlg.setCurrentStackWidget("nginx");
        cfgDlg.exec();
    }

    void MainWindow::openConfigurationDialogPHP()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(this->servers);
        cfgDlg.setCurrentStackWidget("php");
        cfgDlg.exec();
    }

    void MainWindow::openConfigurationDialogMariaDb()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(this->servers);
        cfgDlg.setCurrentStackWidget("mariadb");
        cfgDlg.exec();
    }

    void MainWindow::openConfigurationDialogMongoDb()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(this->servers);
        cfgDlg.setCurrentStackWidget("mongodb");
        cfgDlg.exec();
    }

    void MainWindow::openConfigurationDialogPostgresql()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(this->servers);
        cfgDlg.setCurrentStackWidget("postgresql");
        cfgDlg.exec();
    }

    void MainWindow::openConfigurationDialogRedis()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(this->servers);
        cfgDlg.setCurrentStackWidget("redis");
        cfgDlg.exec();
    }

    void MainWindow::openConfigurationDialogMemcached()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(this->servers);
        cfgDlg.setCurrentStackWidget("memcached");
        cfgDlg.exec();
    }

    QString MainWindow::getServerNameFromPushButton(QPushButton *button)
    {
        return button->objectName().split("_").last(); // "pushButton_FooBar_Nginx" => "Nginx"
    }

    void MainWindow::openConfigurationInEditor()
    {
        QPushButton *button = static_cast<QPushButton *>(sender());
        QString serverName  = this->getServerNameFromPushButton(button);

        // fetch config file for server from the ini
        QString cfgFile = QDir(settings->get(serverName + "/config").toString()).absolutePath();

        if (!QFile::exists(cfgFile)) {
            QMessageBox::warning(this, tr("Warning"), tr("Config file not found: \n") + cfgFile, QMessageBox::Yes);
        } else {
            QDesktopServices::setUrlHandler("file", this, "execEditor");
            // if no UrlHandler is set, this executes the OS-dependend scheme handler
            QDesktopServices::openUrl(QUrl::fromLocalFile(cfgFile));
            QDesktopServices::unsetUrlHandler("file");
        }
    }

    QString MainWindow::getLogfile(const QString &objectName)
    {
        // map objectName to fileName

        QString logsDir = QDir(settings->get("paths/logs").toString()).absolutePath();
        QString logFile = "";

        if (objectName == "pushButton_ShowLog_Nginx") {
            logFile = logsDir + "/access.log";
        }
        if (objectName == "pushButton_ShowErrorLog_Nginx") {
            logFile = logsDir + "/error.log";
        }
        if (objectName == "pushButton_ShowErrorLog_PHP") {
            logFile = logsDir + "/php_error.log";
        }
        if (objectName == "pushButton_ShowErrorLog_MariaDb") {
            logFile = logsDir + "/mariadb_error.log";
        }
        if (objectName == "pushButton_ShowLog_MongoDb") {
            logFile = logsDir + "/mongodb.log";
        }
        if (objectName == "pushButton_ShowLog_PostgreSQL") {
            logFile = logsDir + "/postgresql.log";
        }
        if (objectName == "pushButton_ShowLog_Redis") {
            logFile = logsDir + "/redis.log";
        }

        return logFile;
    }

    void MainWindow::openLog()
    {
        // get log file from objectName of the Signal
        QPushButton *button = static_cast<QPushButton *>(sender());
        QString logfile     = this->getLogfile(button->objectName());

        if (!QFile::exists(logfile)) {
            QMessageBox::warning(this, tr("Warning"), tr("Log file not found: \n") + logfile, QMessageBox::Yes);
        } else {
            QDesktopServices::setUrlHandler("file", this, "execEditor");
            // if no UrlHandler is set, this executes the OS-dependend scheme handler
            QDesktopServices::openUrl(QUrl::fromLocalFile(logfile));
            QDesktopServices::unsetUrlHandler("file");
        }
    }

    void MainWindow::execEditor(const QUrl &logfile)
    {
        QString program = settings->get("global/editor").toString();
        qDebug() << logfile.toLocalFile();

        QProcess::startDetached(program, QStringList() << logfile.toLocalFile());
    }

    void MainWindow::openHelpDialog()
    {
        QDesktopServices::openUrl(QUrl("https://wpn-xm.github.io/docs/user-manual/en/#_the_server_control_panel"));
    }

    void MainWindow::openAboutDialog()
    {
        QString year = QDate::currentDate().toString("yyyy");

        QMessageBox about(this);
        about.setWindowTitle(tr("About"));
        about.setText(tr("<table border=0>"
                         "<tr><td colspan=2><img "
                         "src=\":/wpnxm-logo-dark-transparent\"></img>&nbsp;"
                         "<span style=\"display: inline-block; vertical-align: super; top: "
                         "-20px; font-weight: bold; font-size: 16px;\">v" APP_VERSION "</span>"
                         "</td></tr>"
                         "<tr><td colspan=2>&nbsp;&nbsp;</td></tr>"
                         "<tr><td align=center><b>Website</b></td><td><a "
                         "href=\"https://wpn-xm.org/\">https://wpn-xm.org/</a><br></td></tr>"
                         "<tr><td align=center><b>Author</b></td><td>Jens A. Koch (c) 2010 "
                         "- ")
                          .append(year)
                          .append(".<br></td></tr>"
                                  "<tr><td align=center><b>Github</b></td><td>WPN-XM is developed "
                                  "on Github.<br><a "
                                  "href=\"https://github.com/WPN-XM/WPN-XM/\">https://github.com/"
                                  "WPN-XM/WPN-XM/</a><br></td></tr>"
                                  "<tr><td align=center><b>Icons</b></td><td>We are using Yusukue "
                                  "Kamiyamane's Fugue Icon Set.<br><a "
                                  "href=\"https://p.yusukekamiyamane.com/\">http://"
                                  "p.yusukekamiyamane.com/</a><br></td></tr>"
                                  "<tr><td align=center><b>+1?</b></td><td>If you like using "
                                  "WPN-XM, consider supporting it:<br><a "
                                  "href=\"https://wpn-xm.org/#donate\">https://wpn-xm.org/#donate</"
                                  "a><br></td></tr>"
                                  "<tr><td align=center><b>License</b></td><td>GNU/GPL version 3, "
                                  "or any later version.<br></td></tr>"
                                  "<tr><td align=center><b>Disclaimer</b></td><td>&nbsp;</td></tr>"
                                  "</td></tr></table>"
                                  "<br><br>This software is provided by the development team 'as "
                                  "is' and any expressed or implied warranties, including, but not "
                                  "limited to,"
                                  " the implied warranties of merchantability  and fitness for a "
                                  "particular purpose are disclaimed. In no event shall the "
                                  "development team or its"
                                  " contributors be liable for any direct, indirect, incidental, "
                                  "special, exemplary, or consequential damages"
                                  " (including, but not limited to, procurement of substitute "
                                  "goods or services; loss of use, data, or profits; or business "
                                  "interruption)"
                                  " however caused and on any theory of liability, whether in "
                                  "contract, strict liability, or tort (including negligence or "
                                  "otherwise) arising"
                                  " in any way out of the use of this software, even if advised of "
                                  "the possibility of such damage.<br>"

                                  ));
        about.setParent(this);
        about.setAutoFillBackground(true);
        about.exec();
    }

    void MainWindow::autostartServers()
    {
        qDebug() << "[Servers] Autostarting...";
        if (settings->get("autostart/nginx").toBool())
            servers->startNginx();
        if (settings->get("autostart/php").toBool())
            servers->startPHP();
        if (settings->get("autostart/mariadb").toBool())
            servers->startMariaDb();
        if (settings->get("autostart/mongodb").toBool())
            servers->startMongoDb();
        if (settings->get("autostart/memcached").toBool())
            servers->startMemcached();
        if (settings->get("autostart/postgresql").toBool())
            servers->startPostgreSQL();
        if (settings->get("autostart/redis").toBool())
            servers->startRedis();
    }

    void MainWindow::setDefaultSettings()
    {
        // if the INI is not existing yet, set defaults, they will be written to file
        // if the INI exists, do not set the defaults but read them from file
        if (!QFile(settings->file()).exists()) {

            settings->set("global/runonstartup", 0);
            settings->set("global/autostartservers", 0);
            settings->set("global/startminimized", 0);
            settings->set("global/stopserversonquit", 1);
            settings->set("global/clearlogsonstart", 0);
            settings->set("global/donotaskagainclosetotray", 0);
            settings->set("global/onstartallopenwebinterface", 0);
            settings->set("global/onstartallminimize", 0);
            settings->set("global/editor", "notepad.exe");

            settings->set("paths/logs", "./logs");

            settings->set("autostart/nginx", 1);
            settings->set("autostart/php", 1);
            settings->set("autostart/mariadb", 1);
            if (servers->isInstalled("mongodb")) {
                settings->set("autostart/mongodb", 0);
            }
            if (servers->isInstalled("memcached")) {
                settings->set("autostart/memcached", 0);
            }
            if (servers->isInstalled("postgresql")) {
                settings->set("autostart/postgresql", 0);
            }
            if (servers->isInstalled("redis")) {
                settings->set("autostart/redis", 0);
            }

            settings->set("php/config", "./bin/php/php.ini");

            settings->set("nginx/config", "./bin/nginx/conf/nginx.conf");
            settings->set("nginx/sites", "./www");
            settings->set("nginx/port", 80);

            settings->set("mariadb/config", "./bin/mariadb/my.ini");
            settings->set("mariadb/port", 3306);
            settings->set("mariadb/password", "");
            if (servers->isInstalled("memcached")) {
                settings->set("memcached/tcpport", "11211");
                settings->set("memcached/udpport", "0");
                settings->set("memcached/threads", "2");
                settings->set("memcached/maxconnections", "2048");
                settings->set("memcached/maxmemory", "2048");
            }
            if (servers->isInstalled("mongodb")) {
                settings->set("mongodb/config", "./bin/mongodb/mongod.conf");
            }
            if (servers->isInstalled("postgresql")) {
                settings->set("postgresql/config", "./bin/pgsql/data/postgresql.conf");
                settings->set("postgresql/port", 5432);
            }
            if (servers->isInstalled("redis")) {
                settings->set("redis/config", "./bin/redis/redis.windows.conf");
                settings->set("redis/port", 6379);
            }

            settings->set("selfupdater/runonstartup", 1);
            settings->set("selfupdater/autoupdate", 0);
            settings->set("selfupdater/autorestart", 0);
            settings->set("selfupdater/interval", 7);
            settings->set("selfupdater/last_time_checked", 0);

            qDebug() << "[Settings] Loaded Defaults...\n";
        }
    }

    void MainWindow::renderServerStatusPanel()
    {
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        font1.setPixelSize(11);

        QFont fontNotBold = font1;
        fontNotBold.setBold(false);

        QGroupBox *ServerStatusGroupBox = new QGroupBox(ui->centralWidget);
        ServerStatusGroupBox->setObjectName(QStringLiteral("ServerStatusGroupBox"));
        ServerStatusGroupBox->setEnabled(true);
        ServerStatusGroupBox->setGeometry(QRect(10, 70, 471, 121));
        ServerStatusGroupBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
        ServerStatusGroupBox->setMinimumSize(QSize(471, 121)); // 3 server rows added
        ServerStatusGroupBox->setBaseSize(QSize(471, 121));
        ServerStatusGroupBox->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
        ServerStatusGroupBox->setFlat(false);

        QGridLayout *ServersGridLayout = new QGridLayout(ServerStatusGroupBox);
        ServersGridLayout->setSpacing(10);
        ServersGridLayout->setObjectName(QStringLiteral("ServersGridLayout"));
        // ServersGridLayout->setSizeConstraint(QLayout::SetMinimumSize);

        /**
         * The ServersGrid has the following columns:
         *
         * Status | Port | Server | Version | Config | Logs (2) | Actions (2)
         */

        QLabel *label_Status = new QLabel();
        label_Status->setText(QApplication::translate("MainWindow", "Status", nullptr));
        label_Status->setAlignment(Qt::AlignCenter);
        label_Status->setFont(font1);
        label_Status->setEnabled(false);
        ServersGridLayout->addWidget(label_Status, 1, 0);

        QLabel *label_Port = new QLabel();
        label_Port->setText(QApplication::translate("MainWindow", "Port", nullptr));
        label_Port->setMinimumWidth(38);
        label_Port->setAlignment(Qt::AlignCenter);
        label_Port->setFont(font1);
        label_Port->setEnabled(false);
        ServersGridLayout->addWidget(label_Port, 1, 1);

        QLabel *label_Server = new QLabel();
        label_Server->setText(QApplication::translate("MainWindow", "Server", nullptr));
        label_Server->setAlignment(Qt::AlignCenter);
        label_Server->setFont(font1);
        label_Server->setEnabled(false);
        ServersGridLayout->addWidget(label_Server, 1, 2);

        QLabel *label_Version = new QLabel();
        label_Version->setText(QApplication::translate("MainWindow", "Version", nullptr));
        label_Version->setAlignment(Qt::AlignCenter);
        label_Version->setFont(font1);
        label_Version->setEnabled(false);
        ServersGridLayout->addWidget(label_Version, 1, 3);

        QLabel *label_Config = new QLabel();
        label_Config->setText(QApplication::translate("MainWindow", "Config", nullptr));
        label_Config->setAlignment(Qt::AlignCenter);
        label_Config->setFont(font1);
        label_Config->setEnabled(false);
        ServersGridLayout->addWidget(label_Config, 1, 4, 1, 2); // two columns (gear and gear-pencil)

        QLabel *label_Logs = new QLabel();
        label_Logs->setText(QApplication::translate("MainWindow", "Logs", nullptr));
        label_Logs->setAlignment(Qt::AlignCenter);
        label_Logs->setFont(font1);
        label_Logs->setEnabled(false);
        ServersGridLayout->addWidget(label_Logs, 1, 6, 1, 2); // two columns (log and log-warning)

        QLabel *label_Actions = new QLabel();
        label_Actions->setText(QApplication::translate("MainWindow", "Actions"));
        label_Actions->setAlignment(Qt::AlignCenter);
        label_Actions->setFont(font1);
        label_Actions->setEnabled(false);
        ServersGridLayout->addWidget(label_Actions, 1, 8, 1, 2); // two columns

        /**
         * Define Icons
         */

        QIcon iconConfig;
        iconConfig.addFile(QStringLiteral(":/gear.png"), QSize(), QIcon::Normal, QIcon::Off);

        QIcon iconConfigEdit;
        iconConfigEdit.addFile(QStringLiteral(":/gear--pencil.png"), QSize(), QIcon::Normal, QIcon::Off);

        QIcon iconLog;
        iconLog.addFile(QStringLiteral(":/report.png"), QSize(), QIcon::Normal, QIcon::Off);

        QIcon iconErrorLog;
        iconErrorLog.addFile(QStringLiteral(":/report--exclamation.png"), QSize(), QIcon::Normal, QIcon::Off);

        QIcon iconStop;
        iconStop.addFile(QStringLiteral(":/action_stop"), QSize(), QIcon::Normal, QIcon::Off);

        QIcon iconStart;
        iconStart.addFile(QStringLiteral(":/action_run"), QSize(), QIcon::Normal, QIcon::Off);

        int rowCounter = 2;

        foreach (Servers::Server *server, servers->servers()) {

            /**
             * Columns:
             *
             * Status | Port | Server | Version | Config | Logs (2) | Actions (2)
             */

            // Status
            QLabel *labelStatus = new QLabel();
            labelStatus->setObjectName(QString("label_" + server->name + "_Status"));
            labelStatus->setPixmap(QPixmap(QString::fromUtf8(":/status_run_big")));
            labelStatus->setAlignment(Qt::AlignCenter);
            labelStatus->setEnabled(false); // inital state of status leds is disabled
            ServersGridLayout->addWidget(labelStatus, rowCounter, 0);

            // Port
            if (server->name == "PHP") {
                Tooltips::LabelWithHoverTooltip *labelPort = new Tooltips::LabelWithHoverTooltip();
                labelPort->setObjectName(QString("label_PHP_Port"));
                // labelPort->setTooltipText(getPort(server->lowercaseName));
                labelPort->setFont(fontNotBold);
                ServersGridLayout->addWidget(labelPort, rowCounter, 1);
            } else {
                QLabel *labelPort = new QLabel();
                labelPort->setObjectName(QString("label_" + server->name + "_Port"));
                // labelPort->setText(getPort(server->lowercaseName));
                labelPort->setFont(fontNotBold);
                ServersGridLayout->addWidget(labelPort, rowCounter, 1);
            }

            // Server
            QLabel *labelServer = new QLabel();
            labelServer->setObjectName(QString("label_" + server->name + "_Name"));
            labelServer->setAlignment(Qt::AlignCenter);
            labelServer->setText(QApplication::translate("MainWindow",
                                                         "<span style=\" font-family:'MS Shell Dlg 2'; font-size: "
                                                         "14px; font-weight: bold;\">" +
                                                             server->name.toLocal8Bit() + "</span><"));
            ServersGridLayout->addWidget(labelServer, rowCounter, 2);

            // Version
            QLabel *labelVersion = new QLabel();
            labelVersion->setObjectName(QString("label_" + server->name + "_Version"));
            labelVersion->setAlignment(Qt::AlignCenter);
            labelVersion->setText(getVersion(server->lowercaseName));
            labelVersion->setFont(fontNotBold);
            ServersGridLayout->addWidget(labelVersion, rowCounter, 3);

            // Config

            // Configuration via Webinterface
            QPushButton *pushButton_Configure = new QPushButton();
            pushButton_Configure->setObjectName(QString("pushButton_Configure_" + server->name + ""));
            pushButton_Configure->setIcon(iconConfig);
            pushButton_Configure->setFlat(true);
            pushButton_Configure->setToolTip(QApplication::translate(
                "MainWindow", "Open Configuration Tab for " + server->name.toLocal8Bit() + " "));
            ServersGridLayout->addWidget(pushButton_Configure, rowCounter, 4);

            if (server->name != "Memcached") { // memcached doesn't have a config file

                // Configuration via Editor
                QPushButton *pushButton_ConfigureEdit = new QPushButton();
                pushButton_ConfigureEdit->setObjectName(QString("pushButton_ConfigurationEditor_" + server->name + ""));
                pushButton_ConfigureEdit->setIcon(iconConfigEdit);
                pushButton_ConfigureEdit->setFlat(true);
                pushButton_ConfigureEdit->setToolTip(
                    QApplication::translate("MainWindow", "Edit " + server->name.toLocal8Bit() + " config file"));
                ServersGridLayout->addWidget(pushButton_ConfigureEdit, rowCounter, 5);

                connect(pushButton_ConfigureEdit, SIGNAL(clicked()), this, SLOT(openConfigurationInEditor()));
            }

            // Logs
            foreach (QString logfile, server->logFiles) {
                if (!logfile.isEmpty()) {
                    // normal log
                    if (!logfile.contains("error")) {
                        QPushButton *pushButton_ShowLog = new QPushButton();
                        pushButton_ShowLog->setObjectName(QString("pushButton_ShowLog_" + server->name + ""));
                        pushButton_ShowLog->setIcon(iconLog);
                        pushButton_ShowLog->setFlat(true);
                        pushButton_ShowLog->setToolTip(
                            QApplication::translate("MainWindow", "Open " + server->name.toLocal8Bit() + " Log"));
                        ServersGridLayout->addWidget(pushButton_ShowLog, rowCounter, 6);

                        connect(pushButton_ShowLog, SIGNAL(clicked()), this, SLOT(openLog()));
                    }

                    // error log
                    if (logfile.contains("error")) {
                        QPushButton *pushButton_ShowErrorLog = new QPushButton();
                        pushButton_ShowErrorLog->setObjectName(QString("pushButton_ShowErrorLog_" + server->name + ""));
                        pushButton_ShowErrorLog->setIcon(iconErrorLog);
                        pushButton_ShowErrorLog->setFlat(true);
                        pushButton_ShowErrorLog->setToolTip(
                            QApplication::translate("MainWindow", "Open " + server->name.toLocal8Bit() + " Error Log"));
                        ServersGridLayout->addWidget(pushButton_ShowErrorLog, rowCounter, 7);

                        connect(pushButton_ShowErrorLog, SIGNAL(clicked()), this, SLOT(openLog()));
                    }
                }
            }

            // Actions
            QPushButton *pushButton_Stop = new QPushButton();
            pushButton_Stop->setObjectName(QString("pushButton_Stop_" + server->name + ""));

            pushButton_Stop->setIcon(iconStop);
            pushButton_Stop->setFlat(true);
            pushButton_Stop->setToolTip(
                QApplication::translate("MainWindow", "Stop " + server->name.toLocal8Bit() + ""));
            ServersGridLayout->addWidget(pushButton_Stop, rowCounter, 8);

            QPushButton *pushButton_Start = new QPushButton();
            pushButton_Start->setObjectName(QString("pushButton_Start_" + server->name + ""));
            pushButton_Start->setIcon(iconStart);
            pushButton_Start->setFlat(true);
            pushButton_Start->setToolTip(
                QApplication::translate("MainWindow", "Start " + server->name.toLocal8Bit() + ""));
            ServersGridLayout->addWidget(pushButton_Start, rowCounter, 9);

            rowCounter++;
        }

        /**
         * The ServersGridLayout size depends on the number of installed Components.
         * The BottomWidget has to move down (y + height of ServersGridLayout +
         * margin)
         * The RightSideWidget moves up, if there are only 3-4 elements,
         * the "Webinterface" PushButton will be on par with the Labels.
         * If there are more then 4 elements, the "console" PushButton
         * is on par with the first server.
         */

        // 0) we added a lot of widgets dynamically,
        // now we need to adjust the size of the widgets to fit its contents.
        // in other words: let's update all size/geometry data,
        // before we do additional calculations and start moving things around.
        ServerStatusGroupBox->adjustSize();

        // 1) get rectangle of the status panel
        QRect ServersBox = ServerStatusGroupBox->geometry();
        // 2) get bottom Y of status panel (bottom = top edge Y + height + 1)
        int ServersBoxBottomY = ServersBox.bottom() + 5;

        /*qDebug() << ServerStatusGroupBox->frameGeometry();
  qDebug() << ServerStatusGroupBox->geometry().x();
  qDebug() << ServerStatusGroupBox->geometry().y();
  qDebug() << ServerStatusGroupBox->geometry().height();
  qDebug() << ServerStatusGroupBox->geometry().bottom();
  qDebug() << ServersBoxBottomY;

  qDebug() << ui->BottomWidget->frameGeometry();
  qDebug() << ui->ToolsGroupBox->contentsRect();*/

        QRect BottomWidget = ui->BottomWidget->frameGeometry();

        // 3) re-position bottom widget
        ui->BottomWidget->move(QPoint(BottomWidget.x(), ServersBoxBottomY));

        // 4) finally, re-size the window accordingly
        this->setFixedHeight(ServersBoxBottomY + BottomWidget.height() + 10);
    }

    QString MainWindow::getVersion(const QString &server)
    {
        QString s = server.toLower();
        if (s == "nginx") {
            return getNginxVersion();
        }
        if (s == "memcached") {
            return getMemcachedVersion();
        }
        if (s == "mongodb") {
            return getMongoVersion();
        }
        if (s == "mariadb") {
            return getMariaVersion();
        }
        if (s == "php") {
            return getPHPVersion();
        }
        if (s == "postgresql") {
            return getPostgresqlVersion();
        }
        if (s == "redis") {
            return getRedisVersion();
        }

        qDebug() << "The function for fetching the version for " + s + " is not implemented, yet.";

        return ":(";
    }

    void MainWindow::updateVersion(const QString &server)
    {
        QLabel *label = QApplication::activeWindow()->findChild<QLabel *>("label_" + server + "_Version");
        if (label != nullptr) {
            QString version = getVersion(server);
            label->setText(version);
        }
    }

    QString MainWindow::getPort(const QString &server)
    {
        QString s = server.toLower();
        if (s == "nginx") {
            return getNginxPort();
        }
        if (s == "memcached") {
            return getMemcachedPort();
        }
        if (s == "mongodb") {
            return getMongoPort();
        }
        if (s == "mariadb") {
            return getMariaPort();
        }
        if (s == "php" || s == "php-cgi") {
            return getPHPPort();
        }
        if (s == "postgresql") {
            return getPostgresqlPort();
        }
        if (s == "redis") {
            return getRedisPort();
        }

        qDebug() << "The function for fetching the port for " + s + " is not implemented, yet.";

        return ":(";
    }

    void MainWindow::updatePort(QString server, bool enabled)
    {
        // qDebug() << server << enabled;

        if (server == "php" || server == "php-cgi") {
            if (enabled) {
                // show Label Text to indicate that a HoverTooltip is available
                auto *label = ui->centralWidget->findChild<QLabel *>("label_PHP_Port");
                if (label != nullptr) {
                    label->setText("Pool*");
                }
                // enable hover tooltip + show PHP ports and childs text
                auto *tip = ui->centralWidget->findChild<Tooltips::LabelWithHoverTooltip *>("label_PHP_Port");
                if (tip != nullptr) {
                    tip->enableToolTip(true);
                    tip->setTooltipText(getPHPPort());
                }
            } else {
                // clear label
                auto *label = ui->centralWidget->findChild<QLabel *>("label_PHP_Port");
                if (label != nullptr) {
                    label->setText("");
                }
                // deactivate hover tooltip
                auto *tip = ui->centralWidget->findChild<Tooltips::LabelWithHoverTooltip *>("label_PHP_Port");
                if (tip != nullptr) {
                    tip->enableToolTip(false);
                    tip->setTooltipText("");
                }
            }
        } else {
            QString srvname = servers->getCamelCasedServerName(server);
            auto *label     = ui->centralWidget->findChild<QLabel *>("label_" + srvname + "_Port");
            if (label != nullptr) {
                if (enabled) {
                    QString port = getPort(server);
                    label->setText(port);
                } else {
                    label->setText(""); // clear
                }
            }
        }
    }

    QString MainWindow::getNginxPort() { return settings->get("nginx/port").toString(); }

    QString MainWindow::getMemcachedPort() { return settings->get("memcached/tcpport").toString(); }

    QString MainWindow::getMongoPort()
    {
        QString file = QDir(settings->get("mongodb/config").toString()).absolutePath();

        if (!QFile(file).exists()) {
            qDebug() << "[Error]" << file << "not found";
        }

        File::Yml *yml    = new File::Yml();
        YAML::Node config = yml->load(file);

        return QString::fromStdString(config["net"]["port"].as<std::string>());
    }

    QString MainWindow::getMariaPort() { return settings->get("mariadb/port").toString(); }

    QString MainWindow::getPHPPort()
    {
        QVariantMap PHPServers = servers->getPHPServersFromNginxUpstreamConfig();

        // string template used during iteration
        const QString portStringTemplate("[%1] Port: %2 (%3 Childs) \n");

        QString result;

        auto end = PHPServers.cend();
        for (auto item = PHPServers.cbegin(); item != end; ++item) {
            // item.key   = pool_name
            QString poolName = item.key();

            // item.value = QMap (port, num childs)
            QMap<QString, QVariant> m = item.value().toMap();
            auto end2                 = m.cend();
            for (auto item2 = m.cbegin(); item2 != end2; ++item2) {
                QString port   = item2.key();
                QString childs = item2.value().toString();
                result += portStringTemplate.arg(poolName, port, childs);
            }
        }

        return result;
    }

    QString MainWindow::getPostgresqlPort() { return settings->get("postgresql/port").toString(); }

    QString MainWindow::getRedisPort() { return settings->get("redis/port").toString(); }

    void MainWindow::on_pushButton_Updater_clicked() { this->openUpdaterDialog(); }

    void MainWindow::openUpdaterDialog()
    {
        Updater::UpdaterDialog updaterDialog;
        updaterDialog.exec();
    }

    void MainWindow::openProcessViewerDialog()
    {
        auto *pvd = new ProcessViewerDialog(this);
        pvd->exec();
    }
} // namespace ServerControlPanel
