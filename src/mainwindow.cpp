#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "processviewerdialog.h"

namespace ServerControlPanel
{

    MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new ServerControlPanel::Ui::MainWindow)
    {
        ui->setupUi(this);

        // The tray icon is an instance of the QSystemTrayIcon class.
        // check whether a system tray is present on the user's desktop
        if ( false == QSystemTrayIcon::isSystemTrayAvailable()) {
            QMessageBox::critical(0, APP_NAME, tr("You don't have a system tray."));
            QApplication::exit(1);
        }

        setDefaultSettings();
        setWindowTitle(APP_NAME_AND_VERSION);

        this->servers = new Servers::Servers();

        createTrayIcon();

        renderInstalledDaemons();

        createActions();

        // start minimized to tray
        if(settings->get("global/startminimized").toBool()) {
            setWindowState( Qt::WindowMinimized );
        } else {
            // maximize and move window to the top
            setFocus();
            setWindowState( windowState() & ( ~Qt::WindowMinimized | Qt::WindowActive | Qt::WindowMaximized ) );
        }

        // disable Maximize button functionality
        setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);

        checkAlreadyActiveDaemons();

        showPushButtonsOnlyForInstalledTools();

        connect(this, SIGNAL(mainwindow_show()), this, SLOT(MainWindow_ShowEvent()));

        // daemon autostart
        if(settings->get("global/autostartdaemons").toBool()) {
            qDebug() << "[Daemons] Autostart enabled";
            autostartDaemons();
        };

        if(ui->centralWidget->findChild<QLabel*>("label_Nginx_Status")->isEnabled() &&
           ui->centralWidget->findChild<QLabel*>("label_PHP_Status")->isEnabled()) {
          enableToolsPushButtons(true);
        } else {
          enableToolsPushButtons(false);
        }

        // set window size fixed
        setFixedSize(width(), height());

        //ProcessViewerDialog *pvd = new ProcessViewerDialog(this);
        //pvd->exec();

        if(settings->get("selfupdater/runonstartup").toBool()) {
            runSelfUpdate();
        }
    }

    MainWindow::~MainWindow()
    {
        // stop all daemons, when quitting the tray application
        if(settings->get("global/stopdaemonsonquit").toBool()) {
            qDebug() << "[Daemons] Stopping All Daemons on Quit...";
            stopAllDaemons();
        }

        delete ui;
        delete tray;
    }

    void MainWindow::runSelfUpdate()
    {
        selfUpdater = new Updater::SelfUpdater();        
        connect(selfUpdater, SIGNAL(notifyUpdateAvailable(QJsonObject)),
                this, SLOT(show_SelfUpdater_UpdateNotification(QJsonObject)));
        connect(selfUpdater, SIGNAL(notifyRestartNeeded(QJsonObject)),
                this, SLOT(show_SelfUpdater_RestartNeededNotification(QJsonObject)));
        selfUpdater->run();
    }

    // TODO move to Notification Class
    void MainWindow::show_SelfUpdater_UpdateNotification(QJsonObject versionInfo)
    {
        // when autoupdate is
        // - false, ask the user, if he wants to update (dialogbox)
        // - true, show tray notification (that update is in progress)

        if(!settings->get("selfupdater/autoupdate").toBool())
        {
            // the timer is used to wait, until the SplashScreen is gone
            QTimer::singleShot(2000, selfUpdater, SLOT(askForUpdate()));
            return;
        }

        QString softwareNameAndLatestVersion = QString("%1 v%2").arg(
            versionInfo["software_name"].toString(),
            versionInfo["latest_version"].toString()
        );
        QString title("WPN-XM Server Control Panel\nUpdate available:\n");
        QString msg(softwareNameAndLatestVersion);

        tray->showMessage(title, msg, QSystemTrayIcon::Information, 12000);
    }

    // TODO move to Notification Class
    // if "selfupdater/autorestart" is on, just show a tray info
    void MainWindow::show_SelfUpdater_RestartNeededNotification(QJsonObject versionInfo)
    {
        QString title("WPN-XM Server Control Panel\nUpdate successful! Restarting...\n");
        QString msg("Version v" + versionInfo["latest_version"].toString());

        tray->showMessage(title, msg, QSystemTrayIcon::Information, 12000);
    }

    void MainWindow::createTrayIcon()
    {
        tray = new ServerControlPanel::Tray(qApp, settings, servers);

        tray->setIcon(QIcon(":/wpnxm"));

        // handle clicks on the icon
        connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

        // Status Table - Column Status
        // if process state of a server changes, then change the label status in UI::MainWindow too
        connect(servers, SIGNAL(signalMainWindow_ServerStatusChange(QString,bool)),
                this, SLOT(setLabelStatusActive(QString, bool)));

        // if process state of NGINX and PHP changes,
        // then change the disabled/enabled state of pushButtons, too
        connect(servers, SIGNAL(signalMainWindow_EnableToolsPushButtons(bool)),
                this, SLOT(enableToolsPushButtons(bool)));

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

        // Connect Actions for Status Table - Column Action (Start)
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Start_Nginx"), SIGNAL(clicked()), servers, SLOT(startNginx()));
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Start_PHP"), SIGNAL(clicked()), servers, SLOT(startPHP()));
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Start_MariaDb"), SIGNAL(clicked()), servers, SLOT(startMariaDb()));
        QPushButton *buttonStartMongoDb =  ui->centralWidget->findChild<QPushButton*>("pushButton_Start_MongoDb");
        if(buttonStartMongoDb != 0) { connect(buttonStartMongoDb, SIGNAL(clicked()), servers, SLOT(startMongoDb())); }
        QPushButton *buttonStartMemcached =  ui->centralWidget->findChild<QPushButton*>("pushButton_Start_Memcached");
        if(buttonStartMemcached != 0) { connect(buttonStartMemcached, SIGNAL(clicked()), servers, SLOT(startMemcached())); }
        QPushButton *buttonStartPostgreSQL =  ui->centralWidget->findChild<QPushButton*>("pushButton_Start_PostgreSQL");
        if(buttonStartPostgreSQL != 0) { connect(buttonStartPostgreSQL, SIGNAL(clicked()), servers, SLOT(startPostgreSQL())); }
        QPushButton *buttonStartRedis =  ui->centralWidget->findChild<QPushButton*>("pushButton_Start_Redis");
        if(buttonStartRedis != 0) { connect(buttonStartRedis, SIGNAL(clicked()), servers, SLOT(startRedis())); }

        // Connect Actions for Status Table - Column Action (Stop)
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_Nginx"), SIGNAL(clicked()), servers, SLOT(stopNginx()));
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_PHP"), SIGNAL(clicked()), servers, SLOT(stopPHP()));
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_MariaDb"), SIGNAL(clicked()), servers, SLOT(stopMariaDb()));
        QPushButton *buttonStopMongoDb =  ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_MongoDb");
        if(buttonStopMongoDb != 0) { connect(buttonStopMongoDb, SIGNAL(clicked()), servers, SLOT(stopMongoDb())); }
        QPushButton *buttonStopMemcached =  ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_Memcached");
        if(buttonStopMemcached != 0) { connect(buttonStopMemcached, SIGNAL(clicked()), servers, SLOT(stopMemcached())); }
        QPushButton *buttonStopPostgreSQL =  ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_PostgreSQL");
        if(buttonStopPostgreSQL != 0) { connect(buttonStopPostgreSQL, SIGNAL(clicked()), servers, SLOT(stopPostgreSQL())); }
        QPushButton *buttonStopRedis =  ui->centralWidget->findChild<QPushButton*>("pushButton_Stop_Redis");
        if(buttonStopRedis != 0) { connect(buttonStopRedis, SIGNAL(clicked()), servers, SLOT(stopRedis())); }

        // Connect Actions for Status Table - AllDaemons Start, Stop
        connect(ui->pushButton_AllDaemons_Start, SIGNAL(clicked()), this, SLOT(startAllDaemons()));
        connect(ui->pushButton_AllDaemons_Stop,  SIGNAL(clicked()), this, SLOT(stopAllDaemons()));

        // PushButtons:: Website, Mailinglist, ReportBug, Donate
        connect(ui->pushButton_Website,   SIGNAL(clicked()), this, SLOT(goToWebsite()));
        connect(ui->pushButton_Forum,     SIGNAL(clicked()), this, SLOT(goToForum()));
        connect(ui->pushButton_ReportBug, SIGNAL(clicked()), this, SLOT(goToReportIssue()));
        connect(ui->pushButton_Donate,    SIGNAL(clicked()), this, SLOT(goToDonate()));

        // PushButtons: Configuration, Help, About, Close
        connect(ui->pushButton_Console,       SIGNAL(clicked()), this, SLOT(openConsole()));
        connect(ui->pushButton_Configuration, SIGNAL(clicked()), this, SLOT(openConfigurationDialog()));
        connect(ui->pushButton_Help,          SIGNAL(clicked()), this, SLOT(openHelpDialog()));
        connect(ui->pushButton_About,         SIGNAL(clicked()), this, SLOT(openAboutDialog()));
        // clicking Close, does not quit, but closes the window to tray
        connect(ui->pushButton_Close,         SIGNAL(clicked()), this, SLOT(hide()));

        // Actions - Tools
        connect(ui->pushButton_tools_phpinfo,    SIGNAL(clicked()), this, SLOT(openToolPHPInfo()));
        connect(ui->pushButton_tools_phpmyadmin, SIGNAL(clicked()), this, SLOT(openToolPHPMyAdmin()));
        connect(ui->pushButton_tools_webgrind,   SIGNAL(clicked()), this, SLOT(openToolWebgrind()));
        connect(ui->pushButton_tools_adminer,    SIGNAL(clicked()), this, SLOT(openToolAdminer()));
        connect(ui->pushButton_tools_robomongo,  SIGNAL(clicked()), this, SLOT(openToolRobomongo()));

        // Actions - Open Projects Folder
        connect(ui->pushButton_OpenProjects_Browser,  SIGNAL(clicked()), this, SLOT(openProjectFolderInBrowser()));
        connect(ui->pushButton_OpenProjects_Explorer, SIGNAL(clicked()), this, SLOT(openProjectFolderInExplorer()));

        // Actions - Status Table

        // Configuration via Webinterface
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_Nginx"),   SIGNAL(clicked()), this, SLOT(openConfigurationDialogNginx()));
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_PHP"),     SIGNAL(clicked()), this, SLOT(openConfigurationDialogPHP()));
        connect(ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_MariaDb"), SIGNAL(clicked()), this, SLOT(openConfigurationDialogMariaDb()));

        QPushButton *buttonConfigureMongoDb = ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_MongoDb");
        if(buttonConfigureMongoDb != 0) {
            connect(buttonConfigureMongoDb, SIGNAL(clicked()), this, SLOT(openConfigurationDialogMongoDb()));
        }

        QPushButton *buttonConfigurePostgresql = ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_PostgreSQL");
        if(buttonConfigurePostgresql != 0) {
            connect(buttonConfigurePostgresql, SIGNAL(clicked()), this, SLOT(openConfigurationDialogPostgresql()));
        }

        QPushButton *buttonConfigureRedis = ui->centralWidget->findChild<QPushButton*>("pushButton_Configure_Redis");
        if(buttonConfigureRedis != 0) {
            connect(buttonConfigureRedis, SIGNAL(clicked()), this, SLOT(openConfigurationDialogRedis()));
        }
    }

    void MainWindow::changeEvent(QEvent *event)
    {
        if(0 != event)
        {
            switch (event->type())
            {
                case QEvent::WindowStateChange:
                {
                    // minimize to tray (do not minimize to taskbar)
                    if (this->windowState() & Qt::WindowMinimized)
                    {
                        QTimer::singleShot(0, this, SLOT(hide()));
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
        QList<QPushButton *> allShowLogPushButtons = ui->centralWidget->findChildren<QPushButton *>(QRegExp("pushButton_ShowLog_\\w"));

        for(int i = 0; i < allShowLogPushButtons.size(); ++i) {
            allShowLogPushButtons[i]->setEnabled(
                QFile().exists(this->getLogfile(allShowLogPushButtons[i]->objectName()))
            );
        }

        // Set enabled/disabled state for all "pushButton_ShowErrorLog_*" buttons
        QList<QPushButton *> allShowErrorLogPushButtons = ui->centralWidget->findChildren<QPushButton *>(QRegExp("pushButton_ShowErrorLog_\\w"));

        for(int i = 0; i < allShowErrorLogPushButtons.size(); ++i) {
            allShowErrorLogPushButtons[i]->setEnabled(
                QFile().exists(this->getLogfile(allShowErrorLogPushButtons[i]->objectName()))
            );
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

            if(!doNotAskAgainCloseToTray)
            {
                QCheckBox *checkbox = new QCheckBox(tr("Do not show this message again."), this);
                checkbox->setChecked(doNotAskAgainCloseToTray);

                QMessageBox msgbox(this);
                msgbox.setWindowTitle(qApp->applicationName());
                msgbox.setIconPixmap(QMessageBox::standardIcon(QMessageBox::Information));
                msgbox.setText(
                   tr("This program will keep running in the system tray.<br>"
                      "To terminate the program, choose <b>Quit</b> in the context menu of the system tray.")
                );
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
                if( isVisible() )
                    // clicking the tray icon, when the main window is shown, hides it
                    setVisible(false);
                else {
                    // clicking the tray icon, when the main window is hidden, shows the main window
                    show();
                    setFocus();
                    setWindowState( windowState() & ( ~Qt::WindowMinimized | Qt::WindowActive | Qt::WindowMaximized ) );
                    activateWindow();
                }
                break;
            case QSystemTrayIcon::Trigger:
                break;
            case QSystemTrayIcon::MiddleClick:
                break;
            default:
                return;
        }
    }

    void MainWindow::enableToolsPushButtons(bool enabled)
    {
        // get all PushButtons from the Tools GroupBox of MainWindow::UI
        QList<QPushButton *> allPushButtonsButtons = ui->ToolsGroupBox->findChildren<QPushButton *>();

        // set all PushButtons enabled/disabled
        for(int i = 0; i < allPushButtonsButtons.size(); ++i)
        {
            allPushButtonsButtons[i]->setEnabled(enabled);
        }

        // change state of "Open Projects Folder" >> "Browser" button
        ui->pushButton_OpenProjects_Browser->setEnabled(enabled);

        // disable "webinterface" in TrayMenu, if PHP/Nginx is off
        QList<QAction *>actions = tray->contextMenu()->actions();
        const int listSize = actions.size();
        for (int i = 0; i < listSize; ++i) {
           if(actions.at(i)->iconText() == tr("Webinterface")) {
                actions.at(i)->setEnabled(enabled);
           }
        }

        // webinterface configuration is only available, when nginx and php are running
        // disable "pushButton_Configure_*"
        QList<QPushButton *> allConfigurePushButtons = ui->centralWidget->findChildren<QPushButton *>(QRegExp("pushButton_Configure_\\w"));

        // set all PushButtons enabled/disabled
        for(int i = 0; i < allConfigurePushButtons.size(); ++i)
        {
            allConfigurePushButtons[i]->setEnabled(enabled);
        }
    }

    void MainWindow::showPushButtonsOnlyForInstalledTools()
    {
        // get all PushButtons from the Tools GroupBox of MainWindow::UI
        QList<QPushButton *> allPushButtonsButtons = ui->ToolsGroupBox->findChildren<QPushButton *>();

        // set all PushButtons invisible
        for(int i = 0; i < allPushButtonsButtons.size(); ++i) {
            allPushButtonsButtons[i]->setVisible(false);
        }

        // if "component" exists in "tools" or "bin" directory, show pushButtons in the Tools Groupbox
        if(QDir(getProjectFolder() + "/tools/webinterface").exists()) { ui->pushButton_tools_phpinfo->setVisible(true);  }
        if(QDir(getProjectFolder() + "/tools/phpmyadmin").exists())   { ui->pushButton_tools_phpmyadmin->setVisible(true); }
        if(QDir(getProjectFolder() + "/tools/adminer").exists())      { ui->pushButton_tools_adminer->setVisible(true); }
        if(QDir(getProjectFolder() + "/tools/webgrind").exists())     { ui->pushButton_tools_webgrind->setVisible(true); }
        if(QDir(getProjectFolder() + "/bin/robomongo").exists())      { ui->pushButton_tools_robomongo->setVisible(true); }
    }

    void MainWindow::setLabelStatusActive(QString label, bool enabled)
    {
        label = label.toLower();
        if(label == "nginx")                        { ui->centralWidget->findChild<QLabel*>("label_Nginx_Status")->setEnabled(enabled); }
        if(label == "php" || label == "php-cgi")    { ui->centralWidget->findChild<QLabel*>("label_PHP_Status")->setEnabled(enabled); }
        if(label == "mariadb" || label == "mysqld") { ui->centralWidget->findChild<QLabel*>("label_MariaDb_Status")->setEnabled(enabled); }
        if(label == "mongodb")                      { ui->centralWidget->findChild<QLabel*>("label_MongoDb_Status")->setEnabled(enabled); }
        if(label == "memcached")                    { ui->centralWidget->findChild<QLabel*>("label_Memcached_Status")->setEnabled(enabled); }
        if(label == "postgresql" || label == "postgres") { ui->centralWidget->findChild<QLabel*>("label_PostgreSQL_Status")->setEnabled(enabled); }
        if(label == "redis" || label == "redis-server") { ui->centralWidget->findChild<QLabel*>("label_Redis_Status")->setEnabled(enabled); }

        updateTrayIconTooltip();
    }

    void MainWindow::updateTrayIconTooltip()
    {
       QString tip(APP_NAME_AND_VERSION);
               tip.append("\n\n");

       if(ui->centralWidget->findChild<QLabel*>("label_Nginx_Status")->isEnabled())      { tip.append(" - Nginx: running\n"); }
       if(ui->centralWidget->findChild<QLabel*>("label_PHP_Status")->isEnabled())        { tip.append(" - PHP: running\n"); }
       if(ui->centralWidget->findChild<QLabel*>("label_MariaDb_Status")->isEnabled())    { tip.append(" - MariaDb: running\n"); }
       if(ui->centralWidget->findChild<QLabel*>("label_MongoDb_Status") &&
          ui->centralWidget->findChild<QLabel*>("label_MongoDb_Status")->isEnabled())    { tip.append(" - MongoDb: running\n"); }
       if(ui->centralWidget->findChild<QLabel*>("label_Memcached_Status") &&
          ui->centralWidget->findChild<QLabel*>("label_Memcached_Status")->isEnabled())  { tip.append(" - Memcached: running\n"); }
       if(ui->centralWidget->findChild<QLabel*>("label_PostgreSQL_Status") &&
          ui->centralWidget->findChild<QLabel*>("label_PostgreSQL_Status")->isEnabled()) { tip.append(" - PostgreSQL: running\n"); }
       if(ui->centralWidget->findChild<QLabel*>("label_Redis_Status") &&
          ui->centralWidget->findChild<QLabel*>("label_Redis_Status")->isEnabled())      { tip.append(" - Redis: running\n"); }

       tray->setToolTip(tip);
    }

    void MainWindow::quitApplication()
    {
        if(settings->get("global/stopdaemonsonquit").toBool()) {
            qDebug() << "[Daemons] Stopping on Quit...\n";
            stopAllDaemons();
        }
        qApp->quit();
    }

    QString MainWindow::getNginxVersion()
    {
        // this happens only during testing
        if(!QFile().exists("./bin/nginx/nginx.exe")) {
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
        //QString p_stdout = "nginx version: nginx/1.2.1";

        qDebug() << "[Nginx] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout);
    }

    QString MainWindow::getMariaVersion()
    {
        // this happens only during testing
        if(!QFile().exists("./bin/mariadb/bin/mysqld.exe")) {
            return "0.0.0";
        }

        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start("./bin/mariadb/bin/mysql.exe -V"); // upper-case V

        if (!process.waitForFinished()) {
            qDebug() << "[MariaDb] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readAll();

        // string for regexp testing
        //QString p_stdout = "mysql  Ver 15.1 Distrib 5.5.24-MariaDB, for Win32 (x86)";
        qDebug() << "[MariaDb] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout.mid(45, 15));
    }

    QString MainWindow::getPHPVersion()
    {
        // this happens only during testing
        if(!QFile().exists("./bin/php/php.exe")) {
            return "0.0.0";
        }

        QProcess process;
        process.setProcessChannelMode(QProcess::MergedChannels);
        process.start("./bin/php/php.exe -v");

        if (!process.waitForFinished()) {
            qDebug() << "[PHP] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readAll();

        // string for regexp testing
        //QString p_stdout = "PHP 5.4.3 (cli) (built: Feb 29 2012 19:06:50)";
        //QString p_stdout = "PHP 7.0.0alpha2 (cli)  (non semantic version)";

        qDebug() << "[PHP] Version: \n" << p_stdout;

        return p_stdout.mid(4, p_stdout.lastIndexOf("cli)")).replace(" (cli)", "");
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

        qDebug() << "[MongoDb] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout.mid(3)); //21
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

        qDebug() << "[PostgreSQL] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout.mid(2)); //10
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

        qDebug() << "[Memcached] Version: \n" << p_stdout;

        return parseVersionNumber(p_stdout.mid(2)); //10
    }

    QString MainWindow::getRedisVersion()
    {
        QProcess process;
        process.start("./bin/redis/redis-server.exe -v");

        if (!process.waitForFinished()) {
            qDebug() << "[Redis] Version failed:" << process.errorString();
            return "";
        }

        QByteArray p_stdout = process.readLine();

        qDebug() << "[Redis] Version: \n" << p_stdout;

        // Redis server v=2.8.21 sha
        return parseVersionNumber(p_stdout);
    }

    QString MainWindow::parseVersionNumber(QString stringWithVersion)
    {
        //qDebug() << stringWithVersion;

        // This RegExp matches version numbers: (\d+\.)?(\d+\.)?(\d+\.)?(\*|\d+)
        // This is the same, but escaped:
        QRegExp regex("(\\d+\\.)?(\\d+\\.)?(\\d+\\.)?(\\*|\\d+)");

        // match
        regex.indexIn(stringWithVersion);

        return regex.cap(0);

    // Leave this for debugging purposes
    //    int pos = 0;
    //    while((pos = regex.indexIn(stringWithVersion, pos)) != -1)
    //    {
    //        qDebug() << "Match at pos " << pos
    //                 << " with length " << regex.matchedLength()
    //                 << ", captured = " << regex.capturedTexts().at(0).toLatin1().data()
    //                 << ".\n";
    //        pos += regex.matchedLength();
    //    }
    }

    //*
    //* Action slots
    //*
    void MainWindow::startAllDaemons()
    {
        servers->startNginx();
        servers->startPHP();
        servers->startMariaDb();
        servers->startMongoDb();
        servers->startMemcached();
        servers->startPostgreSQL();
        servers->startRedis();

        if(settings->get("global/OnStartAllOpenWebinterface").toBool()) {
            openWebinterface();
        }

        if(settings->get("global/OnStartAllMinimize").toBool()) {
            setWindowState( Qt::WindowMinimized );
        }
    }

    void MainWindow::stopAllDaemons()
    {
        servers->stopMariaDb();
        servers->stopPHP();
        servers->stopNginx();
        servers->stopMongoDb();
        servers->stopMemcached();
        servers->stopPostgreSQL();
        servers->stopRedis();
    }

    void MainWindow::goToWebsite()
    {
        QDesktopServices::openUrl(QUrl("http://wpn-xm.org/"));
    }

    void MainWindow::goToForum()
    {
        QDesktopServices::openUrl(QUrl("http://forum.wpn-xm.org/"));
    }

    void MainWindow::goToReportIssue()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/issues/"));
    }

    void MainWindow::goToDonate()
    {
        QDesktopServices::openUrl(QUrl("http://wpn-xm.org/#donate"));
    }

    void MainWindow::openToolPHPInfo()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=phpinfo"));
    }

    void MainWindow::openToolPHPMyAdmin()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/phpmyadmin/"));
    }

    void MainWindow::openToolWebgrind()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webgrind/"));
    }

    void MainWindow::openToolAdminer()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/adminer/adminer.php"));
    }

    void MainWindow::openToolRobomongo()
    {
        QString command("./bin/robomongo/robomongo.exe");

        if(!QFile().exists(command)) {
            return;
        }

        QProcess::startDetached(command);
    }

    void MainWindow::openWebinterface()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/"));
    }

    void MainWindow::openProjectFolderInBrowser()
    {
        QDesktopServices::openUrl(QUrl("http://localhost"));
    }

    void MainWindow::openProjectFolderInExplorer()
    {
        if(QDir(getProjectFolder()).exists()) {
            // exec explorer with path to projects
            QDesktopServices::openUrl(QUrl("file:///" + getProjectFolder(), QUrl::TolerantMode));
        } else {
            QMessageBox::warning(this, tr("Warning"), tr("The projects folder was not found."));
        }
    }

    void MainWindow::openConsole()
    {
        QString cmd, conemu;

        if(qgetenv("PROCESSOR_ARCHITECTURE") == "x86") {
            conemu = "./bin/conemu/conemu.exe";
        } else {
            conemu = "./bin/conemu/conemu64.exe";
        }

        // prefer "ConEmu", else fallback to "Windows Console"
        if(QFile().exists(conemu)) {
            cmd = conemu + " /LoadCfgFile ./bin/conemu/ConEmu.xml";
        } else {
            cmd = "cmd.exe";
        }

        QProcess::startDetached(cmd);
    }

    QString MainWindow::getProjectFolder() const
    {
        return QDir::toNativeSeparators(QApplication::applicationDirPath() + "/www");
    }

    void MainWindow::openConfigurationDialog()
    {
        Configuration::ConfigurationDialog cfgDlg;
        cfgDlg.setServers(this->servers);
        cfgDlg.setWindowTitle("WPN-XM Server Control Panel - Configuration");
        cfgDlg.exec();
    }

    void MainWindow::openConfigurationDialogNginx()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#nginx"));
    }

    void MainWindow::openConfigurationDialogPHP()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#php"));
    }

    void MainWindow::openConfigurationDialogMariaDb()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#mariadb"));
    }

    void MainWindow::openConfigurationDialogMongoDb()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#mongodb"));
    }

    void MainWindow::openConfigurationDialogPostgresql()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#postgresql"));
    }

    void MainWindow::openConfigurationDialogRedis()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/index.php?page=config#redis"));
    }

    QString MainWindow::getServerNameFromPushButton(QPushButton *button)
    {
        return button->objectName().split("_").last(); // "pushButton_FooBar_Nginx" => "Nginx"
    }

    void MainWindow::openConfigurationInEditor()
    {
        QString serverName = this->getServerNameFromPushButton( (QPushButton *)sender() );

        // fetch config file for server from the ini
        QString cfgFile = QDir(settings->get(serverName + "/config").toString()).absolutePath();

        if(!QFile().exists(cfgFile)) {
            QMessageBox::warning(this, tr("Warning"), tr("Config file not found: \n") + cfgFile, QMessageBox::Yes);
        } else {
           QDesktopServices::setUrlHandler("file", this, "execEditor");
           // if no UrlHandler is set, this executes the OS-dependend scheme handler
           QDesktopServices::openUrl(QUrl::fromLocalFile(cfgFile));
           QDesktopServices::unsetUrlHandler("file");
        }
    }

    QString MainWindow::getLogfile(QString objectName)
    {
        // map objectName to fileName

        QString logsDir = QDir(settings->get("paths/logs").toString()).absolutePath();
        QString logFile = "";

        if(objectName == "pushButton_ShowLog_Nginx")        { logFile = logsDir + "/access.log";}
        if(objectName == "pushButton_ShowErrorLog_Nginx")   { logFile = logsDir + "/error.log";}
        if(objectName == "pushButton_ShowErrorLog_PHP")     { logFile = logsDir + "/php_error.log";}
        if(objectName == "pushButton_ShowErrorLog_MariaDb") { logFile = logsDir + "/mariadb_error.log";}
        if(objectName == "pushButton_ShowLog_MongoDb")      { logFile = logsDir + "/mongodb.log";}
        if(objectName == "pushButton_ShowLog_PostgreSQL")   { logFile = logsDir + "/postgresql.log";}
        if(objectName == "pushButton_ShowLog_Redis")        { logFile = logsDir + "/redis.log";}

        return logFile;
    }

    void MainWindow::openLog()
    {
        // get log file from objectName of the Signal
        QPushButton *button = (QPushButton *)sender();
        QString logfile = this->getLogfile(button->objectName());

        if(!QFile().exists(logfile)) {
            QMessageBox::warning(this, tr("Warning"), tr("Log file not found: \n") + logfile, QMessageBox::Yes);
        } else {
           QDesktopServices::setUrlHandler("file", this, "execEditor");
           // if no UrlHandler is set, this executes the OS-dependend scheme handler
           QDesktopServices::openUrl(QUrl::fromLocalFile(logfile));
           QDesktopServices::unsetUrlHandler("file");
        }
    }

    void MainWindow::execEditor(QUrl logfile)
    {
        QString program = settings->get("global/editor").toString();
        qDebug() << logfile.toLocalFile();
        
        QProcess::startDetached(program, QStringList() << logfile.toLocalFile());
    }

    void MainWindow::openHelpDialog()
    {
        QDesktopServices::openUrl(QUrl("http://wpn-xm.github.io/docs/user-manual/en/#_the_server_control_panel"));
    }

    void MainWindow::openAboutDialog()
    {
        QString year = QDate::currentDate().toString("yyyy");

        QMessageBox about(this);
        about.setWindowTitle(tr("About"));
        about.setText(
            tr(
                "<table border=0>"
                "<tr><td colspan=2><img src=\":/wpnxm-logo-dark-transparent\"></img>&nbsp;"
                "<span style=\"display: inline-block; vertical-align: super; top: -20px; font-weight: bold; font-size: 16px;\">v" APP_VERSION "</span>"
                "</td></tr>"
                "<tr><td colspan=2>&nbsp;&nbsp;</td></tr>"
                "<tr><td align=center><b>Website</b></td><td><a href=\"http://wpn-xm.org/\">http://wpn-xm.org/</a><br></td></tr>"
                "<tr><td align=center><b>Author(s)</b></td><td>Jens-André Koch (c) 2011 - ").append(year).append(", <br>Yann Le Moigne (c) 2010.<br></td></tr>"
                "<tr><td align=center><b>Github</b></td><td>WPN-XM is developed on Github.<br><a href=\"https://github.com/WPN-XM/WPN-XM/\">https://github.com/WPN-XM/WPN-XM/</a><br></td></tr>"
                "<tr><td align=center><b>Icons</b></td><td>We are using Yusukue Kamiyamane's Fugue Icon Set.<br><a href=\"http://p.yusukekamiyamane.com/\">http://p.yusukekamiyamane.com/</a><br></td></tr>"
                "<tr><td align=center><b>+1?</b></td><td>If you like using WPN-XM, consider supporting it:<br><a href=\"http://wpn-xm.org/#donate\">http://wpn-xm.org/#donate</a><br></td></tr>"
                "<tr><td align=center><b>License</b></td><td>GNU/GPL version 3, or any later version.<br></td></tr>"
                "<tr><td align=center><b>Disclaimer</b></td><td>&nbsp;</td></tr>"
                "</td></tr></table>"
                "<br><br>This software is provided by the development team 'as is' and any expressed or implied warranties, including, but not limited to,"
                " the implied warranties of merchantability  and fitness for a particular purpose are disclaimed. In no event shall the development team or its"
                " contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages"
                " (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption)"
                " however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising"
                " in any way out of the use of this software, even if advised of the possibility of such damage.<br>"

        ));
        about.setParent(this);
        about.setAutoFillBackground(true);
        about.exec();
    }

    void MainWindow::autostartDaemons()
    {
        qDebug() << "[Daemons] Autostart...";
        if(settings->get("autostart/nginx").toBool()) servers->startNginx();
        if(settings->get("autostart/php").toBool()) servers->startPHP();
        if(settings->get("autostart/mariadb").toBool()) servers->startMariaDb();
        if(settings->get("autostart/mongodb").toBool()) servers->startMongoDb();
        if(settings->get("autostart/memcached").toBool()) servers->startMemcached();
        if(settings->get("autostart/postgresql").toBool()) servers->startPostgreSQL();
        if(settings->get("autostart/redis").toBool()) servers->startRedis();
    }

    /**
     * @brief MainWindow::checkPorts
     *
     * check for ports, which are already in use
     * based on "netstat -abno | grep "80\|8080\|443""
     * port and service name identification
     */
    void MainWindow::checkPorts()
    {
        QProcess process;
        process.setReadChannel(QProcess::StandardOutput);
        process.setReadChannelMode(QProcess::MergedChannels);
        process.start("cmd", QStringList() << "/c netstat -abno");
        process.waitForFinished();

        QByteArray servicesByteArray = process.readAll();
        QStringList strLines = QString(servicesByteArray).split("\n", QString::SkipEmptyParts);

        qDebug() << "Port check netstat -abno needs higher privileges" << strLines;
    }

    void MainWindow::checkAlreadyActiveDaemons()
    {
        qDebug() << "[Processes Running] Check for already running processes.";

        //checkPorts();

        // Check active processes and report, if processes are already running.
        // We do this to avoid collisions.
        // A modal dialog with checkboxes for running processes is shown.
        // The user might then select the processes to shutdown or continue.

        // fetch processes via tasklist stdout
        QProcess process;
        process.setReadChannel(QProcess::StandardOutput);
        process.setReadChannelMode(QProcess::MergedChannels);
        process.start("cmd", QStringList() << "/c tasklist.exe");
        process.waitForFinished();

        // processList contains the tasklist output
        QByteArray processList = process.readAll();
        //qDebug() << "Read" << processList.length() << "bytes";
        //qDebug() << processList;

        // define processes to look for
        QStringList processesToSearch;
        processesToSearch << "nginx"
                          << "apache"
                          << "memcached"
                          << "mysqld"
                          << "spawn"
                          << "php-cgi"
                          << "mongod"
                          << "postgres"
                          << "redis-server";

        // init a list for found processes
        QStringList processesFoundList;

        // foreach processesToSearch take a look in the processList
        for (int i = 0; i < processesToSearch.size(); ++i)
        {
            //qDebug() << "Searching for process: " << processesToSearch.at(i).toLocal8Bit().constData() << endl;

            if(processList.contains( processesToSearch.at(i).toLatin1().constData() ))
            {
                // process found
                processesFoundList << processesToSearch.at(i).toLatin1().constData();
            }
        }

        //qDebug() << "Already running Processes found : " << processesFoundList;

        // only show the "process shutdown" dialog, when there are processes to shutdown
        if(false == processesFoundList.isEmpty())
        {
            QLabel *labelA = new QLabel(tr("The following processes are already running:"));
            QGroupBox *groupBox = new QGroupBox(tr("Running Processes"));
            QVBoxLayout *vbox = new QVBoxLayout;

            // iterate over proccesFoundList and draw a "process shutdown" checkbox for each one
            int c = processesFoundList.size();
            for(int i = 0; i < c; ++i) {
               // create checkbox
               QCheckBox *checkbox = new QCheckBox(processesFoundList.at(i));
               checkbox->setChecked(true);
               checkbox->setCheckable(true);
               // add checkbox to view
               vbox->addWidget(checkbox);
            }

            groupBox->setLayout(vbox);

            QLabel *labelB = new QLabel(tr("Please select the processes you wish to shutdown.<br><br>"
                                           "Click Shutdown to shut the selected processes down and continue using the server control panel.<br>"
                                           "To proceed without shutting processes down, click Continue.<br>"));

            QPushButton *ShutdownButton = new QPushButton(tr("Shutdown"));
            QPushButton *ContinueButton = new QPushButton(tr("Continue"));
            ShutdownButton->setDefault(true);

            QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
            buttonBox->addButton(ShutdownButton, QDialogButtonBox::ActionRole);
            buttonBox->addButton(ContinueButton, QDialogButtonBox::ActionRole);

            // e) build dialog to inform user about running processes
            QGridLayout *grid = new QGridLayout;
            grid->addWidget(labelA);
            grid->addWidget(groupBox);
            grid->addWidget(labelB);
            grid->addWidget(buttonBox);

            QDialog *dlg = new QDialog(this);
            dlg->setWindowModality(Qt::WindowModal);
            dlg->setLayout(grid);
            dlg->resize(250, 100);
            dlg->setWindowTitle(tr(APP_NAME));
            dlg->setWindowFlags(dlg->windowFlags() | Qt::WindowStaysOnTopHint);

            // Set signal and slot for "Buttons"
            connect(ShutdownButton, SIGNAL(clicked()), dlg, SLOT(accept()));
            connect(ContinueButton, SIGNAL(clicked()), dlg, SLOT(reject()));

            // show modal window
            int dialogCode = dlg->exec();

            // fire modal window event loop and wait for button clicks
            // if shutdown was clicked (accept), execute shutdowns
            // if continue was clicked (reject), do nothing and proceed to mainwindow
            if(dialogCode == QDialog::Accepted)
            {
                // fetch all checkboxes
                QList<QCheckBox *> allCheckBoxes = dlg->findChildren<QCheckBox *>();

                // iterate checkbox values
                c = allCheckBoxes.size();
                for(int i = 0; i < c; ++i) {
                   QCheckBox *cb = allCheckBoxes.at(i);
                   if(cb->isChecked())
                   {
                       qDebug() << "[Process Shutdown]" << cb->text();

                       // handle the PostgreSQL PID file deletion, too
                       if(cb->text() == "postgres") {
                           QString file = QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/data/postmaster.pid");
                           if(QFile().exists(file)) {
                               QFile().remove(file);
                           }
                       }

                       // taskkill parameters:
                       // /f = force shutdown, /t = structure shutdown, /im = the name of the process
                       // nginx and mariadb need a forced shutdown !
                       QProcess process;
                       process.start("cmd.exe", QStringList() <<"/c"<<"taskkill /f /t /im "+cb->text()+".exe");
                       process.waitForFinished();
                       qDebug() << "[Process Shutdown] Result:\n" << process.readAllStandardOutput();
                   }
                   delete cb;
                }
            }

            // if continue was clicked (reject), do update status indicators in mainwindow and tray
            if(dialogCode == QDialog::Rejected)
            {
                int c = processesFoundList.size();
                for(int i = 0; i < c; ++i) {
                    QString procname = processesFoundList.at(i);
                    QString servername = this->servers->getCamelCasedServerName(procname).toLocal8Bit().constData();
                    Servers::Server *server = this->servers->getServer(servername.toLocal8Bit().constData());
                    qDebug() << "[Processes Running] The process" << procname << " has the Server" << server->name;

                    if(server->name != "Not Installed") {
                        // set indicator - main window
                        setLabelStatusActive(servername, true);
                        // set indicator - tray menu
                        server->trayMenu->setIcon(QIcon(":/status_run"));
                    }
                }
            }

            delete vbox;
            delete labelA;
            delete labelB;
            delete ShutdownButton;
            delete ContinueButton;
            delete buttonBox;
            delete groupBox;
            delete grid;
            delete dlg;
        }
    }

    void MainWindow::setDefaultSettings()
    {
        // if the INI is not existing yet, set defaults, they will be written to file
        // if the INI exists, do not set the defaults but read them from file
        if(false == QFile(settings->file()).exists()) {

            settings->set("global/runonstartup",               0);
            settings->set("global/autostartdaemons",           0);
            settings->set("global/startminimized",             0);
            settings->set("global/stopdaemonsonquit",          1);
            settings->set("global/clearlogsonstart",           0);
            settings->set("global/donotaskagainclosetotray",   0);
            settings->set("global/onstartallopenwebinterface", 0);
            settings->set("global/onstartallminimize",         0);
            settings->set("global/editor",                     "notepad.exe");
            //settings->set("global/showballooninfos",         0);

            settings->set("paths/logs",             "./logs");
            settings->set("paths/php",              "./bin/php");
            settings->set("paths/nginx",            "./bin/nginx");
            settings->set("paths/mariadb",          "./bin/mariadb/bin");
            settings->set("paths/mongodb",          "./bin/mongodb/bin");
            settings->set("paths/memcached",        "./bin/memcached");
            settings->set("paths/postgresql",       "./bin/pgsql/bin");
            settings->set("paths/redis",            "./bin/redis");

            settings->set("autostart/nginx",        1);
            settings->set("autostart/php",          1);
            settings->set("autostart/mariadb",      1);
            settings->set("autostart/mongodb",      0);
            settings->set("autostart/memcached",    0);
            settings->set("autostart/postgresql",   0);
            settings->set("autostart/redis",        0);

            settings->set("php/config",             "./bin/php/php.ini");

            settings->set("nginx/config",           "./bin/nginx/conf/nginx.conf");
            settings->set("nginx/sites",            "./www");
            settings->set("nginx/port",             80);

            settings->set("mariadb/config",         "./bin/mariadb/my.ini");
            settings->set("mariadb/port",           3306);
            settings->set("mariadb/password",       "");

            settings->set("memcached/tcpport",        "11211");
            settings->set("memcached/udpport",        "0");
            settings->set("memcached/threads",        "2");
            settings->set("memcached/maxconnections", "2048");
            settings->set("memcached/maxmemory",      "2048");

            settings->set("mongodb/config",         "./bin/mongodb/mongodb.conf");
            settings->set("mongodb/port",           27015);

            settings->set("postgresql/config",      "./bin/pgsql/data/postgresql.conf");
            settings->set("postgresql/port",        5432);

            settings->set("redis/config",           "./bin/redis/redis.windows.conf");
            settings->set("redis/port",             6379);

            settings->set("selfupdater/runonstartup", 1);

            //settings->set("updater/mode",         "manual");
            //settings->set("updater/interval",     "1w");

            qDebug() << "[Settings] Loaded Defaults...\n";
        }
    }

    void MainWindow::renderInstalledDaemons()
    {
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        font1.setPixelSize(11);

        QFont fontNotBold = font1;
        fontNotBold.setBold(false);

        QGroupBox *DaemonStatusGroupBox = new QGroupBox(ui->centralWidget);
        DaemonStatusGroupBox->setObjectName(QStringLiteral("DaemonStatusGroupBox"));
        DaemonStatusGroupBox->setEnabled(true);
        DaemonStatusGroupBox->setGeometry(QRect(10, 70, 471, 121));
        DaemonStatusGroupBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
        DaemonStatusGroupBox->setMinimumSize(QSize(0, 121));
        DaemonStatusGroupBox->setBaseSize(QSize(471, 100));
        DaemonStatusGroupBox->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        DaemonStatusGroupBox->setFlat(false);

        QGridLayout *DaemonsGridLayout = new QGridLayout(DaemonStatusGroupBox);
        DaemonsGridLayout->setSpacing(10);
        DaemonsGridLayout->setObjectName(QStringLiteral("DaemonsGridLayout"));
        DaemonsGridLayout->setSizeConstraint(QLayout::SetMinimumSize);

        /**
         * The DaemonsGrid has the following columns:
         *
         * Status | Port | Daemon | Version | Config | Logs (2) | Actions (2)
         */

        QLabel* label_Status = new QLabel();
        label_Status->setText(QApplication::translate("MainWindow", "Status", 0));
        label_Status->setAlignment(Qt::AlignCenter);
        label_Status->setFont(font1);
        label_Status->setEnabled(false);
        DaemonsGridLayout->addWidget(label_Status, 1, 0);

        QLabel* label_Port = new QLabel();
        label_Port->setText(QApplication::translate("MainWindow", "Port", 0));
        label_Port->setAlignment(Qt::AlignCenter);
        label_Port->setFont(font1);
        label_Port->setEnabled(false);
        DaemonsGridLayout->addWidget(label_Port, 1, 1);

        QLabel* label_Daemon = new QLabel();
        label_Daemon->setText(QApplication::translate("MainWindow", "Daemon", 0));
        label_Daemon->setAlignment(Qt::AlignCenter);
        label_Daemon->setFont(font1);
        label_Daemon->setEnabled(false);
        DaemonsGridLayout->addWidget(label_Daemon, 1, 2);

        QLabel* label_Version = new QLabel();
        label_Version->setText(QApplication::translate("MainWindow", "Version", 0));
        label_Version->setAlignment(Qt::AlignCenter);
        label_Version->setFont(font1);
        label_Version->setEnabled(false);
        DaemonsGridLayout->addWidget(label_Version, 1, 3);

        QLabel* label_Config = new QLabel();
        label_Config->setText(QApplication::translate("MainWindow", "Config", 0));
        label_Config->setAlignment(Qt::AlignCenter);
        label_Config->setFont(font1);
        label_Config->setEnabled(false);
        DaemonsGridLayout->addWidget(label_Config, 1, 4, 1, 2); // two columns (gear and gear-pencil)

        QLabel* label_Logs = new QLabel();
        label_Logs->setText(QApplication::translate("MainWindow", "Logs", 0));
        label_Logs->setAlignment(Qt::AlignCenter);
        label_Logs->setFont(font1);
        label_Logs->setEnabled(false);
        DaemonsGridLayout->addWidget(label_Logs, 1, 6, 1, 2); // two columns (log and log-warning)

        QLabel* label_Actions = new QLabel();
        label_Actions->setText(QApplication::translate("MainWindow", "Actions", 0));
        label_Actions->setAlignment(Qt::AlignCenter);
        label_Actions->setFont(font1);
        label_Actions->setEnabled(false);
        DaemonsGridLayout->addWidget(label_Actions, 1, 8, 1, 2); // two columns

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

        foreach(Servers::Server *server, servers->servers()) {

            /**
             * Columns:
             *
             * Status | Port | Daemon | Version | Config | Logs (2) | Actions (2)
             */

            // Status
            QLabel* labelStatus = new QLabel();
            labelStatus->setObjectName(QString("label_" + server->name + "_Status"));
            labelStatus->setPixmap(QPixmap(QString::fromUtf8(":/status_run_big")));
            labelStatus->setAlignment(Qt::AlignCenter);
            labelStatus->setEnabled(false); // inital state of status leds is disabled
            DaemonsGridLayout->addWidget(labelStatus, rowCounter, 0);

            // Port
            QLabel* labelPort = new QLabel();
            labelPort->setObjectName(QString("label_"+server->name+"_Port"));
            labelPort->setText( settings->get(server->lowercaseName+"/port").toString() );
            labelPort->setAlignment(Qt::AlignCenter);
            labelPort->setFont(fontNotBold);
            DaemonsGridLayout->addWidget(labelPort, rowCounter, 1);

            // Daemon
            QLabel* labelDaemon = new QLabel();
            labelDaemon->setObjectName(QString("label_" + server->name + "_Name"));
            labelDaemon->setAlignment(Qt::AlignCenter);
            labelDaemon->setText(QApplication::translate("MainWindow",
                "<span style=\" font-family:'MS Shell Dlg 2'; font-size: 14px; font-weight: bold;\">" + server->name.toLocal8Bit() + "</span><", 0));
            DaemonsGridLayout->addWidget(labelDaemon, rowCounter, 2);

            // Version
            QLabel* labelVersion = new QLabel();
            labelVersion->setObjectName(QString("label_" + server->name + "_Version"));
            labelVersion->setAlignment(Qt::AlignCenter);
            labelVersion->setText(getVersion(server->lowercaseName));
            labelVersion->setFont(fontNotBold);
            DaemonsGridLayout->addWidget(labelVersion, rowCounter, 3);

            // Config

            if(server->name != "Memcached") {

                // Configuration via Webinterface
                QPushButton* pushButton_Configure = new QPushButton();
                pushButton_Configure->setObjectName(QString("pushButton_Configure_"+ server->name +""));
                pushButton_Configure->setIcon(iconConfig);
                pushButton_Configure->setFlat(true);
                pushButton_Configure->setToolTip(QApplication::translate(
                    "MainWindow", "Open the Webinterface Configuration Tab for "+ server->name.toLocal8Bit() +" ", 0)
                );
                DaemonsGridLayout->addWidget(pushButton_Configure, rowCounter, 4);

                // Configuration via Editor
                QPushButton* pushButton_ConfigureEdit = new QPushButton();
                pushButton_ConfigureEdit->setObjectName(QString("pushButton_ConfigurationEditor_"+ server->name +""));
                pushButton_ConfigureEdit->setIcon(iconConfigEdit);
                pushButton_ConfigureEdit->setFlat(true);
                pushButton_ConfigureEdit->setToolTip(QApplication::translate(
                    "MainWindow", "Edit "+ server->name.toLocal8Bit() +" config file", 0)
                );
                DaemonsGridLayout->addWidget(pushButton_ConfigureEdit, rowCounter, 5);

                connect(pushButton_ConfigureEdit, SIGNAL(clicked()), this, SLOT(openConfigurationInEditor()));
            }

            // Logs
            foreach (QString logfile, server->logFiles)
            {
                if(!logfile.isEmpty()) {
                    // normal log
                    if(!logfile.contains("error"))
                    {
                        QPushButton* pushButton_ShowLog = new QPushButton();
                        pushButton_ShowLog->setObjectName(QString("pushButton_ShowLog_" + server->name + ""));
                        pushButton_ShowLog->setIcon(iconLog);
                        pushButton_ShowLog->setFlat(true);
                        pushButton_ShowLog->setToolTip(QApplication::translate(
                            "MainWindow", "Open "+ server->name.toLocal8Bit() +" Log", 0)
                        );
                        DaemonsGridLayout->addWidget(pushButton_ShowLog, rowCounter, 6);

                        connect(pushButton_ShowLog, SIGNAL(clicked()), this, SLOT(openLog()));
                    }

                    // error log
                    if(logfile.contains("error"))
                    {
                        QPushButton* pushButton_ShowErrorLog = new QPushButton();
                        pushButton_ShowErrorLog->setObjectName(QString("pushButton_ShowErrorLog_"+ server->name +""));
                        pushButton_ShowErrorLog->setIcon(iconErrorLog);
                        pushButton_ShowErrorLog->setFlat(true);
                        pushButton_ShowErrorLog->setToolTip(QApplication::translate(
                            "MainWindow", "Open "+ server->name.toLocal8Bit() +" Error Log", 0)
                        );
                        DaemonsGridLayout->addWidget(pushButton_ShowErrorLog, rowCounter, 7);

                        connect(pushButton_ShowErrorLog, SIGNAL(clicked()), this, SLOT(openLog()));
                    }
                }
            }

            // Actions
            QPushButton* pushButton_Stop = new QPushButton();
            pushButton_Stop->setObjectName(QString("pushButton_Stop_"+ server->name +""));

            pushButton_Stop->setIcon(iconStop);
            pushButton_Stop->setFlat(true);
            pushButton_Stop->setToolTip(QApplication::translate(
                "MainWindow", "Stop "+ server->name.toLocal8Bit() +"", 0)
            );
            DaemonsGridLayout->addWidget(pushButton_Stop, rowCounter, 8);

            QPushButton* pushButton_Start = new QPushButton();
            pushButton_Start->setObjectName(QString("pushButton_Start_"+ server->name +""));
            pushButton_Start->setIcon(iconStart);
            pushButton_Start->setFlat(true);
            pushButton_Start->setToolTip(QApplication::translate(
                "MainWindow", "Start "+ server->name.toLocal8Bit() +"", 0)
            );
            DaemonsGridLayout->addWidget(pushButton_Start, rowCounter, 9);

            rowCounter++;
        }

        /**
         * The DaemonsGridLayout size depends on the number of installed Components.
         * The BottomWidget has to move down (y + height of DaemonsGridLayout + margin)
         * The RightSideWidget moves up, if there are only 3-4 elements,
         * the "Webinterface" PushButton will be on par with the Labels.
         * If there are more then 4 elements, the "webinterface" PushButton
         * is on par with the first server.
         */

        QRect DaemonsBox = DaemonStatusGroupBox->frameGeometry();
        QSize DaemonsSize = DaemonStatusGroupBox->sizeHint();

        // (top left corner y) + (dynamic height y, based on rows for daemons)
        int bottomWidgetY = DaemonsBox.y() + DaemonsSize.height();

        QRect BottomWidgetGeo = ui->BottomWidget->geometry();

        if(BottomWidgetGeo.y() > bottomWidgetY) {
            // 3 or 4 elements - move things from the bottom up
            ui->BottomWidget->move(QPoint(BottomWidgetGeo.x(), bottomWidgetY + 10));
            ui->ToolsGroupBox->move(QPoint(ui->ToolsGroupBox->x(), ui->ToolsGroupBox->y()));
            QRect RightWidgetGeo = ui->RightSideWidget->geometry();
            ui->RightSideWidget->move(QPoint(RightWidgetGeo.x(), RightWidgetGeo.y() - 20));
            this->resize(QSize(this->geometry().width(), bottomWidgetY + BottomWidgetGeo.height() + 20));
        } else {
            // more then 4 elements - auto-expand
            ui->BottomWidget->move(QPoint(BottomWidgetGeo.x(), bottomWidgetY - 10));
            this->resize(QSize(this->geometry().width(), bottomWidgetY + BottomWidgetGeo.height()));
        }
    }

    QString MainWindow::getVersion(QString server)
    {
        QString s = server.toLower();
        if(s == "nginx")     { return getNginxVersion(); }
        if(s == "memcached") { return getMemcachedVersion(); }
        if(s == "mongodb")   { return getMongoVersion(); }
        if(s == "mariadb")   { return getMariaVersion(); }
        if(s == "php")       { return getPHPVersion(); }
        if(s == "postgresql"){ return getPostgresqlVersion(); }
        if(s == "redis")     { return getRedisVersion(); }

        return "The function for fetching the version for " + s + "is not implemented, yet.";
    }

    void MainWindow::updateVersion(QString server)
    {
        QString version = getVersion(server);
        QLabel* label = qApp->activeWindow()->findChild<QLabel *>("label_" + server + "_Version");
        if(label != 0) {
            label->setText(version);
        }
    }

    void MainWindow::on_pushButton_Updater_clicked()
    {
        this->openUpdaterDialog();
    }

    void MainWindow::openUpdaterDialog()
    {
        Updater::UpdaterDialog updaterDialog;
        updaterDialog.setWindowTitle("WPN-XM Server Control Panel - Updater");
        updaterDialog.exec();
    }

}
