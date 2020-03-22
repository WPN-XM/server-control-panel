#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSystemTrayIcon>
#include <QRegularExpression>

#include "common.h"
#include "tray.h"
#include "servers.h"
#include "settings.h"
#include "selfupdater.h"

#include "file/yml.h"
#include "processes.h"
#include "config/configurationdialog.h"
#include "processviewer/processviewerdialog.h"
#include "updater/updaterdialog.h"
#include "plugins/plugins.h"
#include "widgets/dockwidgets/consoledockwidget.h"
#include "widgets/tooltips/BalloonTip.h"
#include "widgets/tooltips/LabelWithHoverTooltip.h"

namespace ServerControlPanel
{
    namespace Ui
    {
        class MainWindow;
    }

    class APP_CORE_EXPORT MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        explicit MainWindow(QWidget &parent) = delete;
        ~MainWindow() override;

        void setVisible(bool visible) override;

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

        QString parseVersionNumber(const QString &stringWithVersion);

    public slots:

        // General Action Slots of the MainWindow
        void startAllServers();
        void stopAllServers();

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
        void openConfigurationDialogMemcached();

        void openLog();
        void openConfigurationInEditor();

        void updateServerStatusIndicators(const QString &server, bool enabled);

        void updateLabelStatus(const QString &server, bool enabled);
        void updateVersion(const QString &server);
        void updatePort(QString server, bool enabled);

        void enableToolsPushButtons(bool enabled);
        void updateTrayIconTooltip();
        void updateToolsPushButtons();
        void updateServerStatusOnTray(const QString &serverName, bool enabled);

        void quitApplication();

        void execEditor(const QUrl &logfile);

        void runSelfUpdate();

        ServerControlPanel::MainWindow *getMainWindow();

        void setup();

        void setProcessUtil(Processes::ProcessUtil *oProcessesUtil);
        Processes::ProcessUtil *getProcessUtil();

        void setPlugins(PluginsNS::Plugins *oPlugins);
        PluginsNS::Plugins *getPlugins();

        Settings::SettingsManager *getSettings();

    private:
        Ui::MainWindow *ui;

        ServerControlPanel::Tray *tray{};
        Servers::Servers *servers{};
        Updater::SelfUpdater *selfUpdater{};
        Processes::ProcessUtil *processes{};
        PluginsNS::Plugins *plugins{};
        Settings::SettingsManager *settings{};

        QAction *minimizeAction{};
        QAction *restoreAction{};
        QAction *quitAction{};

        void checkPorts();
        void createActions();
        void createTrayIcon();
        void createConsole();

        void setDefaultSettings();
        void autostartServers();

        void renderServerStatusPanel();

        void setMainWindowDefaultSize();

        QString getRootFolder() const;
        QString getProjectFolder() const;
        void showPushButtonsOnlyForInstalledTools();

        QString getVersion(const QString &server);
        QString getPort(const QString &server);

        QString getLogfile(const QString &objectName);
        QString getServerNameFromPushButton(QPushButton *button);

        // MainWindow Default Size
        // int defaultWidth  = 621;
        // int defaultHeight = 424;

    signals:
        void mainwindow_show();

    private slots:
        void iconActivated(QSystemTrayIcon::ActivationReason reason);

        // updater
        void pushButton_Updater_clicked();

        void MainWindow_ShowEvent();

        void show_SelfUpdater_UpdateNotification(QJsonObject versionInfo);
        void show_SelfUpdater_RestartNeededNotification(QJsonObject versionInfo);

        void updateServerStatusIndicatorsForAlreadyRunningServers();

        void handlePlugin(QObject *plugin);

    protected:
        void closeEvent(QCloseEvent *event) override;
        void changeEvent(QEvent *event) override;
        void showEvent(QShowEvent *event) override;
    };
} // namespace ServerControlPanel

#endif // MAINWINDOW_H
