#include "tray.h"
#include "src/tooltips/TrayToolTip.h"

namespace ServerControlPanel
{
    Tray::Tray(QApplication *parent, Servers::Servers *servers)
        : QSystemTrayIcon(QIcon(":/wpnxm"), parent), servers(servers)
    {
        createTrayMenu();

        /**
         * TrayToolTip:
         *  - we instantiate a QWidget based tooltip
         *  - and set visibility false
         *  - then, we start a timer, which polls the mouse position, see timerEvent()
         *  - then, when the cursor.pos is inside the rectangle of the SystemTrayIcon,
         *    we display the tooltip widget
         *
         * Note: it's not possible to use event() or eventFilter() methods
         *       to wait for the events QEvent:ToolTip or QHelpEvent
         *       to capture the mouse hover event of the tray icon.
         *       It's not supported by Qt, yet (v5.7).
         */
        // tooltip = new TrayToolTip;
        // tooltipVisible = false;
        // startTimer(500);
    }

    void Tray::createTrayMenu()
    {
        qDebug() << "[Tray] Create Tray and add Server menus.\t";

        QMenu *trayMenu = contextMenu();

        if (trayMenu) {
            trayMenu->clear();
        } else {
            trayMenu = new QMenu;
        }

        // add title entry like for WPN-XM in KVirc style (background gray, bold,
        // small font)
        trayMenu->addAction("WPN-XM v" APP_VERSION_SHORT)->setFont(QFont("Arial", 9, QFont::Bold));
        trayMenu->addSeparator();

        // add local IPs to the tray menu
        foreach (const QHostAddress &address, NetworkUtils::getLocalHostIPs()) {
            trayMenu->addAction("Local IP: " + address.toString())->setFont(QFont("Arial", 9, QFont::Bold));
        }
        trayMenu->addSeparator();

        // start and stop all servers; the connection to these actions is made from
        // mainwindow
        trayMenu->addAction(QIcon(":/action_run"), tr("Start All"), this, SLOT(startAllServers()), QKeySequence());
        trayMenu->addAction(QIcon(":/action_stop"), tr("Stop All"), this, SLOT(stopAllServers()), QKeySequence());
        trayMenu->addSeparator();

        // add all server submenus to the tray menu
        foreach (Servers::Server *server, servers->servers()) {
            trayMenu->addMenu(server->trayMenu);
            qDebug() << "[Tray] Added Menu:\t" << server->name;
        }

        trayMenu->addSeparator();
        trayMenu->addAction(QIcon(":/gear"), tr("Manage Hosts"), this, SLOT(openHostManagerDialog()), QKeySequence());
        trayMenu->addAction(QIcon(":/gear"), tr("Webinterface"), this, SLOT(goToWebinterface()), QKeySequence());
        trayMenu->addSeparator();
        trayMenu->addAction(QIcon(":/report_bug"), tr("&Report Bug"), this, SLOT(goToReportIssue()), QKeySequence());
        trayMenu->addAction(QIcon(":/question"), tr("&Help"), this, SLOT(goToWebsiteHelp()), QKeySequence());
        trayMenu->addAction(QIcon(":/quit"), tr("&Quit"), QCoreApplication::instance(), SLOT(quit()), QKeySequence());

        setContextMenu(trayMenu);
    }

    void Tray::goToWebinterface() { QDesktopServices::openUrl(QUrl("http://wpn.xm/")); }

    void Tray::goToReportIssue() { QDesktopServices::openUrl(QUrl("https://github.com/WPN-XM/WPN-XM/issues/")); }

    void Tray::goToWebsiteHelp()
    {
        QDesktopServices::openUrl(QUrl("https://wpn-xm.github.io/docs/user-manual/en/#_the_server_control_panel"));
    }

    void Tray::startAllServers()
    {
        servers->startNginx();
        servers->startPHP();
        servers->startMariaDb();
        servers->startMongoDb();
        servers->startMemcached();
        servers->startPostgreSQL();
        servers->startRedis();
    }

    void Tray::stopAllServers()
    {
        servers->stopMariaDb();
        servers->stopPHP();
        servers->stopNginx();
        servers->stopMongoDb();
        servers->stopMemcached();
        servers->stopPostgreSQL();
        servers->stopRedis();
    }

    void Tray::openHostManagerDialog()
    {
        HostsFileManager::HostsManagerDialog dlg;
        dlg.exec();
    }

    void Tray::timerEvent(QTimerEvent *event)
    {
        Q_UNUSED(event);

        QPoint relativeMousePos = QCursor::pos();

        // hide tooltip
        if (!geometry().contains(relativeMousePos)) {
            if (tooltipVisible) {
                tooltip->hide();
            }
            tooltipVisible = false;
            return;
        }

        // do not show tooltip
        if (tooltipVisible || contextMenu()->isVisible()) {
            return;
        }

        // show tooltip

        // const QPixmap pixmap = QIcon(":/wpnxm").pixmap(QSize(22, 22),
        // QIcon::Normal, QIcon::On);
        // tooltip->showMessage(pixmap, tooltipTitle, tooltipMsg, relativeMousePos);
        tooltip->showMessage(tooltipMsg, relativeMousePos);

        tooltipVisible = true;
    }

    void Tray::setMessage(const QString &title)
    {
        tooltipTitle = "Info";
        tooltipMsg   = title;
    }

    void Tray::setMessage(const QString &title, const QString &msg)
    {
        tooltipTitle = title;
        tooltipMsg   = msg;
    }

    bool Tray::isTooltipVisible()
    {
        // no tooltip object, means the feature isn't active
        if (!tooltip) {
            return false;
        }
        return tooltipVisible;
    }

    void Tray::hideTooltip() { tooltip->hide(); }
} // namespace ServerControlPanel
