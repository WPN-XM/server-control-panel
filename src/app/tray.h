#ifndef Tray_H
#define Tray_H

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QUrl>
#include <QWidgetAction>

#include <QHostAddress>

#include "hostmanager/hostmanagerdialog.h"
#include "networkutils.h"
#include "servers.h"
#include "settings.h"
#include "tooltips/TrayToolTip.h"
#include "version.h"

namespace ServerControlPanel
{
    /// Implements a tray menu with icons.
    /*!
    This class creates a tray menu with icons.
    The tray menu and the mainwindow might be used to control the servers.
    Servers processes are monitored and their process state is displayed in the
   server status panel.
*/
    class Tray : public QSystemTrayIcon
    {
        Q_OBJECT

    public:
        explicit Tray(QCoreApplication *parent, Servers::Servers *servers);

        void setMessage(const QString &title);
        void setMessage(const QString &title, const QString &msg);
        bool isTooltipVisible();
        void hideTooltip();

    public slots:
        void startAllServers();
        void stopAllServers();
        void goToWebsiteHelp();
        void goToReportIssue();
        void goToWebinterface();
        void openHostManagerDialog();

    private:
        void createTrayMenu();

        Settings::SettingsManager *settings{};
        Servers::Servers *servers;
        TrayToolTip *tooltip = nullptr;

        bool tooltipVisible{};
        QString tooltipTitle;
        QString tooltipMsg;

    protected:
        void timerEvent(QTimerEvent *event) override;
    };
} // namespace ServerControlPanel

#endif // Tray_H
