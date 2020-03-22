#include "pluginsmanager.h"
#include "ui_pluginsmanager.h"

#include "plugins/plugins.h"
#include "plugins/plugininterface.h"
#include "config/pluginlistdelegate.h"

#include "settings.h"

#include <QDebug>

namespace Configuration
{

    PluginsManager::PluginsManager(QWidget *parent) : QWidget(parent), ui(new Configuration::Ui::PluginsManager)
    {
        ui->setupUi(this);
        ui->list->setLayoutDirection(Qt::LeftToRight);

        // ui->butSettings->setIcon(IconProvider::settingsIcon());
        // ui->butRemove->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));

        connect(ui->butSettings, &QAbstractButton::clicked, this, &PluginsManager::settingsClicked);
        connect(ui->butRemove, &QAbstractButton::clicked, this, &PluginsManager::removeClicked);
        connect(ui->list, &QListWidget::currentItemChanged, this, &PluginsManager::currentChanged);
        connect(ui->list, &QListWidget::itemChanged, this, &PluginsManager::itemChanged);
        // connect(mApp->plugins(), &Plugins::availablePluginsChanged, this, &PluginsManager::refresh);

        ui->list->setItemDelegate(new PluginsNS::PluginListDelegate(ui->list));

        load();
    }

    PluginsManager::~PluginsManager() { delete ui; }

    void PluginsManager::load()
    {
        if (!loaded) {
            refresh();
            loaded = true;
        }
    }

    void PluginsManager::save()
    {
        if (!loaded) {
            return;
        }

        QStringList enabledPlugins;

        const int listItems = ui->list->count();

        for (int i = 0; i < listItems; i++) {
            QListWidgetItem *item = ui->list->item(i);

            if (item->checkState() == Qt::Checked) {
                const auto plugin = item->data(Qt::UserRole + 10).value<PluginsNS::Plugins::Plugin>();
                enabledPlugins.append(plugin.pluginId);
            }
        }

        if (enabledPlugins.empty()) {
            QStringList defaultEnabledPlugins = {"HelloWorldPlugin"};
            enabledPlugins << defaultEnabledPlugins;
        }

        Settings::SettingsManager settings;
        settings.set("Plugin-Settings/EnabledPlugins", enabledPlugins);

        qDebug() << "[PluginManager][Settings][Save] Saved EnabledPlugins list to Settings.";
    }

    void PluginsManager::refresh()
    {
        if (blockRefreshPlugins) {
            return;
        }

        const int oldCurrentRow = ui->list->currentRow();

        // stop list handling
        // clear list, deactivate settings button, disconnect item change handling
        ui->list->clear();
        ui->butSettings->setEnabled(false);
        disconnect(ui->list, &QListWidget::itemChanged, this, &PluginsManager::itemChanged);

        // iterate plugins

        auto *plugins = new PluginsNS::Plugins();

        const QList<PluginsNS::Plugins::Plugin> &allPlugins = plugins->getAvailablePlugins();

        for (const PluginsNS::Plugins::Plugin &plugin : allPlugins) {

            auto *item = new QListWidgetItem(ui->list);

            PluginsNS::PluginMetaData metaData = plugin.metaData;

            // QIcon icon = QIcon(desc.icon);
            // item->setIcon(icon);

            QStringList authorsList;
            QMapIterator<QString, QString> i(metaData.authors);
            while (i.hasNext()) {
                authorsList << i.next().key();
            }
            QString authors = authorsList.join(", ");

            QString pluginInfo =
                QString("<b>%1</b> %2<br/><i>%3</i><br/>").arg(metaData.name, metaData.version, authors);
            item->setToolTip(pluginInfo);

            item->setText(metaData.name);
            item->setData(Qt::UserRole, metaData.version);
            item->setData(Qt::UserRole + 1, authors);
            item->setData(Qt::UserRole + 2, metaData.description);

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(plugin.isLoaded() ? Qt::Checked : Qt::Unchecked);
            item->setData(Qt::UserRole + 10, QVariant::fromValue(plugin));

            ui->list->addItem(item);
        }

        sortItems();

        if (oldCurrentRow >= 0) {
            ui->list->setCurrentRow(qMax(0, oldCurrentRow - 1));
            ui->list->setFocus();
        }

        connect(ui->list, &QListWidget::itemChanged, this, &PluginsManager::itemChanged);
    }

    void PluginsManager::sortItems()
    {
        ui->list->sortItems();

        bool itemMoved;
        do {
            itemMoved = false;
            for (int i = 0; i < ui->list->count(); ++i) {
                QListWidgetItem *topItem    = ui->list->item(i);
                QListWidgetItem *bottomItem = ui->list->item(i + 1);
                if (!topItem || !bottomItem) {
                    continue;
                }

                if (topItem->checkState() == Qt::Unchecked && bottomItem->checkState() == Qt::Checked) {
                    QListWidgetItem *item = ui->list->takeItem(i + 1);
                    ui->list->insertItem(i, item);
                    itemMoved = true;
                }
            }
        } while (itemMoved);
    }

    void PluginsManager::currentChanged(QListWidgetItem *item)
    {
        if (!item) {
            return;
        }

        const PluginsNS::Plugins::Plugin plugin = item->data(Qt::UserRole + 10).value<PluginsNS::Plugins::Plugin>();

        ui->butSettings->setEnabled(plugin.isLoaded() && plugin.metaData.hasSettings);
        ui->butRemove->setEnabled(plugin.isRemovable());
    }

    void PluginsManager::itemChanged(QListWidgetItem *item)
    {
        if (!item) {
            return;
        }

        auto *plugins = new PluginsNS::Plugins();

        auto plugin = item->data(Qt::UserRole + 10).value<PluginsNS::Plugins::Plugin>();

        blockRefreshPlugins = true;

        if (item->checkState() == Qt::Checked) {
            plugins->loadPlugin(&plugin);
        } else {
            plugins->unloadPlugin(&plugin);
        }

        blockRefreshPlugins = false;

        disconnect(ui->list, &QListWidget::itemChanged, this, &PluginsManager::itemChanged);

        if (item->checkState() == Qt::Checked && !plugin.isLoaded()) {
            item->setCheckState(Qt::Unchecked);

            QMessageBox::critical(this, tr("Error!"), tr("Cannot load extension!"));
        }

        item->setData(Qt::UserRole + 10, QVariant::fromValue(plugin));

        connect(ui->list, &QListWidget::itemChanged, this, &PluginsManager::itemChanged);

        currentChanged(ui->list->currentItem());
    }

    void PluginsManager::settingsClicked()
    {
        QListWidgetItem *item = ui->list->currentItem();

        if (!item || item->checkState() == Qt::Unchecked) {
            return;
        }

        auto plugin = item->data(Qt::UserRole + 10).value<PluginsNS::Plugins::Plugin>();

        auto *plugins = new PluginsNS::Plugins();

        if (!plugin.isLoaded()) {
            plugins->loadPlugin(&plugin);

            item->setData(Qt::UserRole + 10, QVariant::fromValue(plugin));
        }

        if (plugin.isLoaded() && plugin.metaData.hasSettings) {
            plugin.instance->showSettings(this);
        }
    }

    void PluginsManager::removeClicked()
    {
        QListWidgetItem *item = ui->list->currentItem();
        if (!item) {
            return;
        }

        auto plugin = item->data(Qt::UserRole + 10).value<PluginsNS::Plugins::Plugin>();

        if (plugin.isRemovable()) {
            // mApp->plugins()->removePlugin(&plugin);
        }
    }

} // namespace Configuration
