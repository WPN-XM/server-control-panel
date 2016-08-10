#include "tray.h"

namespace ServerControlPanel
{
    Tray::Tray(QApplication *parent, Settings::SettingsManager *settings, Servers::Servers *servers) :
        QSystemTrayIcon(QIcon(":/wpnxm.png"), parent),
        settings(settings),
        servers(servers)
    {
        createTrayMenu();
    }

    void Tray::createTrayMenu()
    {
        QMenu *trayMenu = contextMenu();

        if (trayMenu) {
            trayMenu->clear();
        } else {
            trayMenu = new QMenu;
            setContextMenu(trayMenu);
        }

        // set tray menu icon
        trayMenu->setIcon(QIcon(QPixmap(":/wpnxm.png")));

        // add title entry like for WPN-XM in KVirc style (background gray, bold, small font)
        trayMenu->addAction("WPN-XM v" APP_VERSION_SHORT)->setFont(QFont("Arial", 9, QFont::Bold));
        trayMenu->addSeparator();

        // add local IPs to the tray menu
        foreach(const QHostAddress &address, NetworkUtils::getLocalHostIPs()) {
            trayMenu->addAction("IP: "+address.toString())->setFont(QFont("Arial", 9, QFont::Bold));
        }
        trayMenu->addSeparator();

        // start and stop all daemons; the connection to these actions is made from mainwindow
        trayMenu->addAction(QIcon(":/action_run"), tr("Start All"), this, SLOT(startAllDaemons()), QKeySequence());
        trayMenu->addAction(QIcon(":/action_stop"), tr("Stop All"), this, SLOT(stopAllDaemons()), QKeySequence());
        trayMenu->addSeparator();

        // add all server submenus to the tray menu
        foreach(Servers::Server *server, servers->servers()) {
            trayMenu->addMenu(server->trayMenu);
            qDebug() << "[" + server->name + "] SubMenu was added to the TrayMenu.";
        }

        trayMenu->addSeparator();
        trayMenu->addAction(QIcon(":/gear"), tr("Manage Hosts"), this, SLOT(openHostManagerDialog()), QKeySequence());
        trayMenu->addAction(QIcon(":/gear"), tr("Webinterface"), this, SLOT(goToWebinterface()), QKeySequence());
        trayMenu->addSeparator();
        trayMenu->addAction(QIcon(":/report_bug"), tr("&Report Bug"), this, SLOT(goToReportIssue()), QKeySequence());
        trayMenu->addAction(QIcon(":/question"),tr("&Help"), this, SLOT(goToWebsiteHelp()), QKeySequence());
        trayMenu->addAction(QIcon(":/quit"),tr("&Quit"), qApp, SLOT(quit()), QKeySequence());
    }

    void Tray::goToWebinterface()
    {
        QDesktopServices::openUrl(QUrl("http://localhost/tools/webinterface/"));
    }

    void Tray::goToReportIssue()
    {
        QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/issues/"));
    }

    void Tray::goToWebsiteHelp()
    {
        QDesktopServices::openUrl(QUrl("http://wpn-xm.github.io/docs/user-manual/en/#_the_server_control_panel"));
    }

    void Tray::startAllDaemons()
    {
        servers->startNginx();
        servers->startPHP();
        servers->startMariaDb();
        servers->startMongoDb();
        servers->startMemcached();
    }

    void Tray::stopAllDaemons()
    {
        servers->stopMariaDb();
        servers->stopPHP();
        servers->stopNginx();
        servers->stopMongoDb();
        servers->stopMemcached();
    }

    void Tray::openHostManagerDialog()
    {
        HostsFileManager::HostsManagerDialog dlg;
        dlg.exec();
    }   
}
