#include "plugins.h"

namespace Plugins
{

    Plugins::Plugins(QObject *parent) : QObject(parent) { loadSettings(); }

    QList<Plugins::Plugin> Plugins::getAvailablePlugins()
    {
        loadAvailablePlugins();

        return availablePlugins;
    }

    void Plugins::loadAvailablePlugins()
    {
        if (pluginsLoaded) {
            return;
        } else {
            pluginsLoaded = true;
        }

        // find SharedLibrary Plugins
        const QDir pluginsDir(QCoreApplication::applicationDirPath() + QDir::separator() + "plugins");

        QStringList nameFilters;
        nameFilters << "*.dll"; // << "*.so";

        const auto files = pluginsDir.entryInfoList(nameFilters, QDir::Files);

        for (const QFileInfo &file : files) {
            const QString fileName = file.absoluteFilePath();

            QPluginLoader *pluginLoader = new QPluginLoader(fileName);

            if (pluginLoader->metaData().value("MetaData").type() != QJsonValue::Object) {
                qDebug() << "Invalid plugin (metadata json missing):" << fileName << pluginLoader->errorString();
                continue;
            }

            QJsonObject pluginMetaData = pluginLoader->metaData();

            QObject *pluginInstance = pluginLoader->instance();
            if (!pluginInstance) {
                qDebug() << "Error loading plugin:" << fileName << pluginLoader->errorString();
                continue;
            } else {
                qDebug() << "Plugin loaded:" << fileName;
            }

            PluginInterface *pluginObject = qobject_cast<PluginInterface *>(pluginInstance);

            // const QMetaObject *pluginMeta = pluginInstance->metaObject();

            Plugins::Plugin plugin;
            plugin.id          = pluginMetaData.value("name").toString();
            plugin.instance    = pluginObject;
            plugin.libraryPath = fileName;
            plugin.loader      = pluginLoader;
            plugin.metaData    = Plugins::getMetaData(pluginMetaData);

            // add plugin to list
            if (!availablePlugins.contains(plugin)) {
                availablePlugins.append(plugin);
            }
        }
    }

    PluginMetaData Plugins::getMetaData(const QJsonObject &metaData)
    {
        PluginMetaData pluginMetaData;

        QJsonObject metaDataObject = metaData.value("MetaData").toObject();

        pluginMetaData.iid         = metaData.value("IID").toString();
        pluginMetaData.name        = metaDataObject.value("name").toString();
        pluginMetaData.version     = metaDataObject.value("version").toString();
        pluginMetaData.description = metaDataObject.value("description").toString();
        pluginMetaData.type        = metaDataObject.value("type").toString();
        pluginMetaData.core        = metaDataObject.value("core").toBool();
        pluginMetaData.hasSettings = metaDataObject.value("hasSettings").toBool();

        QJsonArray authorsJsonArray = metaDataObject.value("authors").toArray();

        foreach (const QJsonValue &value, authorsJsonArray) {
            QJsonObject obj = value.toObject();
            pluginMetaData.authors.insert(obj["name"].toString(), obj["email"].toString());
        }

        pluginMetaData.icon = metaDataObject.value("icon").toString();

        // pluginMetaData.dependencies     = metaDataObject.value("dependencies").toBool();
        // pluginMetaData.dependenciesPath = metaDataObject.value("libPaths").toArray();

        return pluginMetaData;
    }

    void Plugins::loadSettings()
    {
        QStringList defaultEnabledPlugins = {"HelloWorldPlugin"};

        Settings::SettingsManager settingsManager;
        QSettings settings(settingsManager.file(), QSettings::IniFormat);
        settings.beginGroup("Plugin-Settings");
        enabledPlugins = settings.value("EnabledPlugins", defaultEnabledPlugins).toStringList();
        settings.endGroup();
    }

} // namespace Plugins
