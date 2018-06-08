#ifndef CONFIGURATIONDIALOG_H
#define CONFIGURATIONDIALOG_H

#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>

#include <QTableWidget>
#include <QTreeWidgetItem>

#include "../servers.h"
#include "../settings.h"
#include "../windowsapi.h"

namespace Configuration
{
    namespace Ui
    {
        class ConfigurationDialog;
    }

    class ConfigurationDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit ConfigurationDialog(QWidget *parent = 0);

        ~ConfigurationDialog();

        void setRunOnStartUp(bool run = true);
        bool runOnStartUp() const;

        void setAutostartServers(bool run = true);
        bool runAutostartServers() const;

        void setClearLogsOnStart(bool run = true);
        bool runClearLogsOnStart() const;

        void setStopServersOnQuit(bool run = true);
        bool stopServersOnQuit() const;

        void fileOpen();

        void setServers(Servers::Servers *servers);
        void hideAutostartCheckboxesOfNotInstalledServers();

        void setCurrentStackWidget(QString widgetname);

        // TODO move config stuff into own class per component

        void saveSettings_MariaDB_Configuration();
        void saveSettings_MongoDB_Configuration();
        void saveSettings_Redis_Configuration();
        void saveSettings_PostgreSQL_Configuration();
        void saveSettings_Xdebug_Configuration();

        // TODO move nginx stuff into a "nginx config class"

        void saveSettings_Nginx_Upstream();
        QJsonValue serialize_toJSON_Nginx_Upstream_ServerTable(QTableWidget *servers);
        QJsonValue serialize_toJSON_Nginx_Upstream_PoolsTable(QTableWidget *pools);

        void writeNginxUpstreamConfigs(QJsonDocument jsonDoc);
        QJsonObject getNginxUpstreamPoolByName(QString requestedUpstreamPoolName);
        void updateServersTable(QJsonObject jsonPool);

        void createNginxConfUpstreamFolderIfNotExists_And_clearOldConfigs();

    private slots:
        void toggleAutostartServerCheckboxes(bool run = true);
        void onClickedButtonBoxOk();

        void on_toolButton_SelectEditor_clicked();
        void on_toolButton_ResetEditor_clicked();

        void on_configMenuTreeWidget_clicked(const QModelIndex &index);

        // bind PushButtons of Nginx Config Tab
        void on_pushButton_Nginx_Upstream_AddUpstream_clicked();
        void on_pushButton_Nginx_Upstream_AddServer_clicked();
        void on_pushButton_Nginx_Reset_Upstreams_clicked();
        void on_pushButton_Nginx_Reset_Servers_clicked();
        void on_tableWidget_Upstream_itemSelectionChanged();

        // bind PushButtons for resetting ports
        void on_pushButton_MariaDb_Reset_Port_clicked();
        void on_pushButton_MongoDb_Reset_Port_clicked();
        void on_pushButton_Memcached_Reset_Port_clicked();
        void on_pushButton_PostgreSql_Reset_Port_clicked();
        void on_pushButton_Redis_Reset_Port_clicked();

        void on_configMenuSearchLineEdit_textChanged(const QString &string);


    private:
        Ui::ConfigurationDialog *ui;

        QSortFilterProxyModel *configMenuFilterProxyModel;

        Settings::SettingsManager *settings;
        Servers::Servers *servers;

        QCheckBox *checkbox_runOnStartUp;
        QCheckBox *checkbox_autostartServers;
        QCheckBox *checkbox_clearLogsOnStart;
        QCheckBox *checkbox_stopServersOnQuit;

        QCheckBox *checkbox_autostart_MariaDb;
        QCheckBox *checkbox_autostart_MongoDb;
        QCheckBox *checkbox_autostart_PHP;
        QCheckBox *checkbox_autostart_Nginx;
        QCheckBox *checkbox_autostart_Memcached;
        QCheckBox *checkbox_autostart_PostgreSQL;
        QCheckBox *checkbox_autostart_Redis;

        void readSettings();
        void writeSettings();

        void toggleRunOnStartup();

        void loadNginxUpstreams();

        bool getSettingBool(const QString &key, const QVariant &defaultValue);
        bool getSettingBool(const QString &key, const bool &defaultValue);

        int getSettingInt(const QString &key, const QVariant &defaultValue);
        int ConfigurationDialog::getSettingInt(const QString &key, const int &defaultValue);

        QString getSettingString(const QString &key, const QVariant &defaultValue);
        QString getSettingString(const QString &key, const QString &defaultValue);

    };
}

#endif // CONFIGURATIONDIALOG_H
