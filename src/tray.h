#ifndef Tray_H
#define Tray_H

#include <QSystemTrayIcon>
#include <QProcess>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QPushButton>
#include <QWidgetAction>
#include <QMessageBox>
#include <QDir>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>

#include <QHostAddress>

#include "version.h"
#include "servers.h"
#include "hostmanager/hostmanagerdialog.h"
#include "settings.h"
#include "networkutils.h"
#include "src/tooltips/TrayToolTip.h"

namespace ServerControlPanel
{
    /// Implements a tray menu with icons.
    /*!
        This class creates a tray menu with icons.
        The tray menu and the mainwindow might be used to control the servers.
        Servers processes are monitored and their process state is displayed in the server status panel.
    */
    class Tray : public QSystemTrayIcon
    {
        Q_OBJECT

        public:
            explicit Tray(QApplication *parent, Servers::Servers *servers);

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

            Settings::SettingsManager *settings;
            Servers::Servers          *servers;
            TrayToolTip               *tooltip = NULL;

            bool        tooltipVisible;
            QString     tooltipTitle;
            QString     tooltipMsg;

        protected:
            void timerEvent(QTimerEvent *event);
    };
}

#endif // Tray_H
