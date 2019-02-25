#include "pluginmanager.h"
#include "ui_pluginlist.h"

#include "plugins.h"
#include "plugininterface.h"
#include "pluginlistdelegate.h"

namespace Plugins
{

    PluginManager::PluginManager(QWidget *parent) : QWidget(parent), ui(new Ui::PluginList), loaded(false)
    {
        ui->setupUi(this);
        ui->list->setLayoutDirection(Qt::LeftToRight);
        // ui->butSettings->setIcon(IconProvider::settingsIcon());

        // Application Extensions
        /*Settings settings;
        settings.beginGroup("Plugin-Settings");
        bool appPluginsEnabled = settings.value("EnablePlugins", true).toBool();
        settings.endGroup();*/

        // ui->list->setEnabled(appPluginsEnabled);

        /*connect(ui->butSettings, &QAbstractButton::clicked, this, &PluginsManager::settingsClicked);
        connect(ui->list, &QListWidget::currentItemChanged, this, &PluginsManager::currentChanged);
        connect(ui->list, &QListWidget::itemChanged, this, &PluginsManager::itemChanged);
    */
        ui->list->setItemDelegate(new PluginListDelegate(ui->list));
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

        QStringList allowedPlugins;
        for (int i = 0; i < ui->list->count(); i++) {
            QListWidgetItem *item = ui->list->item(i);

            if (item->checkState() == Qt::Checked) {
                const Plugins::Plugin plugin = item->data(Qt::UserRole + 10).value<Plugins::Plugin>();
                allowedPlugins.append(plugin.id);
            }
        }

        /*Settings settings;
        settings.beginGroup("Plugin-Settings");
        settings.setValue("AllowedPlugins", allowedPlugins);
        settings.endGroup();*/
    }

    void PluginManager::refresh()
    {
        auto *plugins = new Plugins();

        const auto &allPlugins = plugins->getAvailablePlugins();

        foreach (const auto &plugin, allPlugins) {

            QListWidgetItem *item = new QListWidgetItem(ui->list);

            // QIcon icon = QIcon(desc.icon);
            // item->setIcon(icon);

            QStringList authorsList;
            QMapIterator<QString, QString> i(plugin.metaData.authors);
            while (i.hasNext()) {
                authorsList << i.next().key();
            }
            QString authors = authorsList.join(", ");

            QString pluginInfo =
                QString("<b>%1</b> %2<br/><i>%3</i><br/>").arg(plugin.metaData.name, plugin.metaData.version, authors);
            item->setToolTip(pluginInfo);

            item->setText(plugin.metaData.name);
            item->setData(Qt::UserRole, plugin.metaData.version);
            item->setData(Qt::UserRole + 1, authors);
            item->setData(Qt::UserRole + 2, plugin.metaData.description);

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(plugin.isLoaded() ? Qt::Checked : Qt::Unchecked);
            item->setData(Qt::UserRole + 10, QVariant::fromValue(plugin));

            ui->list->addItem(item);
        }
    }

    QStringList PluginManager::getConfigTreeMenuItem()
    {
        QStringList list;
        list << "Plugins";
        list << this->objectName();

        return list;
    }

} // namespace Plugins
