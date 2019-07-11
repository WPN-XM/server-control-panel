#include "pluginmanager.h"
#include "ui_pluginlist.h"

#include "plugins.h"
#include "plugininterface.h"
#include "pluginlistdelegate.h"

#include "settings.h"

#include <QDebug>

namespace Plugins
{

    PluginManager::PluginManager(QWidget *parent) : QWidget(parent), ui(new Ui::PluginList), loaded(false)
    {
        ui->setupUi(this);
        ui->list->setLayoutDirection(Qt::LeftToRight);
        // ui->butSettings->setIcon(IconProvider::settingsIcon());

        connect(ui->butSettings, &QAbstractButton::clicked, this, &PluginManager::settingsClicked);
        connect(ui->list, &QListWidget::currentItemChanged, this, &PluginManager::currentChanged);
        connect(ui->list, &QListWidget::itemChanged, this, &PluginManager::itemChanged);

        ui->list->setItemDelegate(new PluginListDelegate(ui->list));

        load();
    }

    PluginManager::~PluginManager() { delete ui; }

    void PluginManager::load()
    {
        if (!loaded) {
            refresh();
            loaded = true;
        }
    }

    void PluginManager::save()
    {
        if (!loaded) {
            return;
        }

        QStringList enabledPlugins;

        for (int i = 0; i < ui->list->count(); i++) {
            QListWidgetItem *item = ui->list->item(i);

            if (item->checkState() == Qt::Checked) {
                const Plugins::Plugin plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();
                enabledPlugins.append(plugin.pluginId);
            }
        }

        if (enabledPlugins.empty()) {
            enabledPlugins << "HelloWorldPlugin";
        }

        Settings::SettingsManager settings;
        // settings.set("Plugin-Settings/LoadPlugins", true);
        settings.set("Plugin-Settings/EnabledPlugins", enabledPlugins);

        qDebug() << "[PluginManager][Settings][Save] Saved EnabledPlugins list to Settings.";
    }

    void PluginManager::refresh()
    {
        // stop list handling
        // clear list, deactivate settings button, disconnect item change handling
        ui->list->clear();
        ui->butSettings->setEnabled(false);
        disconnect(ui->list, &QListWidget::itemChanged, this, &PluginManager::itemChanged);

        // iterate plugins

        auto *plugins = new Plugins();

        const QList<Plugins::Plugin> &allPlugins = plugins->getAvailablePlugins();

        foreach (const Plugins::Plugin &plugin, allPlugins) {

            QListWidgetItem *item = new QListWidgetItem(ui->list);

            PluginMetaData metaData = plugin.metaData;

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

        connect(ui->list, &QListWidget::itemChanged, this, &PluginManager::itemChanged);
    }

    void PluginManager::sortItems()
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

    void PluginManager::currentChanged(QListWidgetItem *item)
    {
        if (!item) {
            return;
        }

        const Plugins::Plugin plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();

        bool showSettings = plugin.metaData.hasSettings;

        if (!plugin.isLoaded()) {
            showSettings = false;
        }

        ui->butSettings->setEnabled(showSettings);
    }

    void PluginManager::itemChanged(QListWidgetItem *item)
    {
        if (!item) {
            return;
        }

        Plugins *plugins = new Plugins();

        Plugins::Plugin plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();

        if (item->checkState() == Qt::Checked) {
            plugins->loadPlugin(&plugin);
        } else {
            plugins->unloadPlugin(&plugin);
        }

        disconnect(ui->list, &QListWidget::itemChanged, this, &PluginManager::itemChanged);

        if (item->checkState() == Qt::Checked && !plugin.isLoaded()) {
            item->setCheckState(Qt::Unchecked);

            QMessageBox::critical(this, tr("Error!"), tr("Cannot load extension!"));
        }

        item->setData(Qt::UserRole + 10, QVariant::fromValue(plugin));

        connect(ui->list, &QListWidget::itemChanged, this, &PluginManager::itemChanged);

        currentChanged(ui->list->currentItem());
    }

    void PluginManager::settingsClicked()
    {
        QListWidgetItem *item = ui->list->currentItem();

        if (!item || item->checkState() == Qt::Unchecked) {
            return;
        }

        Plugins::Plugin plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();

        Plugins *plugins = new Plugins;

        if (!plugin.isLoaded()) {
            plugins->loadPlugin(&plugin);

            item->setData(Qt::UserRole + 10, QVariant::fromValue(plugin));
        }

        if (plugin.isLoaded() && plugin.metaData.hasSettings) {
            plugin.instance->showSettings(this);
        }
    }

} // namespace Plugins
