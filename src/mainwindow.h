#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QSystemTrayIcon>

#include "servers.h"
#include "settings.h"
#include "tray.h"
#include "config/configurationdialog.h"
#include "updater/updaterdialog.h"
#include "selfupdater.h"
#include "processviewer/processes.h"
#include "processviewer/processviewerdialog.h"
#include "tooltips/BalloonTip.h"
#include "tooltips/LabelWithHoverTooltip.h"

namespace ServerControlPanel
{
    namespace Ui {
        class MainWindow;
    }

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

        public:
            explicit MainWindow(QWidget *parent = 0);
            ~MainWindow();

            void setVisible(bool visible);

            QString getPHPVersion();
            QString getNginxVersion();
            QString getMariaVersion();
            QString getMongoVersion();
            QString getMemcachedVersion();
            QString getPostgresqlVersion();
            QString getRedisVersion();

            QString getPHPPort();
            QString getNginxPort();
            QString getMariaPort();
            QString getMongoPort();
            QString getMemcachedPort();
            QString getPostgresqlPort();
            QString getRedisPort();

            QString parseVersionNumber(QString stringWithVersion);

        public slots:

            // General Action Slots of the MainWindow
            void startAllDaemons();
            void stopAllDaemons();

            void goToWebsite();
            void goToForum();
            void openHelpDialog();
            void openAboutDialog();
            void goToReportIssue();
            void goToDonate();

            void openToolPHPInfo();
            void openToolPHPMyAdmin();
            void openToolWebgrind();
            void openToolAdminer();
            void openToolRobomongo();

            void openProjectFolderInBrowser();
            void openProjectFolderInExplorer();

            void openConsole();
            void openWebinterface();
            void openConfigurationDialog();
            void openProcessViewerDialog();
            void openUpdaterDialog();

            void openConfigurationDialogNginx();
            void openConfigurationDialogPHP();
            void openConfigurationDialogMariaDb();
            void openConfigurationDialogMongoDb();
            void openConfigurationDialogPostgresql();
            void openConfigurationDialogRedis();

            void openLog();
            void openConfigurationInEditor();

            void setLabelStatusActive(QString label, bool enabled);
            void updateVersion(QString server);
            void updatePort(QString port);

            void enableToolsPushButtons(bool enabled);
            void updateTrayIconTooltip();

            void quitApplication();

            void execEditor(QUrl logfile);

            void runSelfUpdate();

            MainWindow *getMainWindow();
            void setup();

            void setProcessesInstance(Processes *oProcesses);
            Processes *getProcessesObject();

        private:
            Ui::MainWindow *ui;

            ServerControlPanel::Tray  *tray;
            Settings::SettingsManager *settings;
            Servers::Servers          *servers;
            Updater::SelfUpdater      *selfUpdater;
            Processes                 *processes;

            QAction *minimizeAction;
            QAction *restoreAction;
            QAction *quitAction;

            void checkAlreadyActiveDaemons();
            void checkPorts();
            void createActions();
            void createTrayIcon();

            void setDefaultSettings();
            void autostartDaemons();

            void renderServerStatusPanel();

            QString getProjectFolder() const;
            void showPushButtonsOnlyForInstalledTools();

            QString getVersion(QString server);
            QString getPort(QString server);

            QString getLogfile(QString objectName);
            QString getServerNameFromPushButton(QPushButton *button);

        signals:
            void mainwindow_show();

        private slots:
            void iconActivated(QSystemTrayIcon::ActivationReason reason);
            //void execEditor(QUrl logfile);

            void on_pushButton_Updater_clicked();
            void MainWindow_ShowEvent();

            void show_SelfUpdater_UpdateNotification(QJsonObject versionInfo);
            void show_SelfUpdater_RestartNeededNotification(QJsonObject versionInfo);           

        protected:
            void closeEvent(QCloseEvent *event);
            void changeEvent(QEvent *event);
            void showEvent(QShowEvent *event);
    };

}

#endif // MAINWINDOW_H
