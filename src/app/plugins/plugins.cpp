#include "plugins.h"

namespace PluginsNS
{
    bool Plugins::Plugin::isLoaded() const { return instance; }

    bool Plugins::Plugin::isRemovable() const { return !pluginPath.isEmpty() && QFileInfo(pluginPath).isWritable(); }

    bool Plugins::Plugin::operator==(const Plugin &other) const
    {
        return /*type == other.type &&*/ pluginId == other.pluginId;
    }

    Plugins::Plugins(QObject *parent) : QObject(parent), pluginsLoaded(false)
    {
        // load Settings
        loadSettings();
    }

    QList<Plugins::Plugin> Plugins::getAvailablePlugins()
    {
        loadAvailablePlugins();

        return availablePlugins;
    }
    void Plugins::loadPlugins()
    {
        foreach (const QString &pluginName, enabledPlugins) {
            Plugin plugin = loadPlugin(pluginName);
            if (!initPlugin(PluginInterface::StartupInitState, &plugin)) {
                qWarning() << "Failed to init" << pluginName << "plugin";
                continue;
            }
            registerAvailablePlugin(plugin);
        }

        refreshLoadedPlugins();

        qDebug() << "[Plugins]" << loadedPlugins.count() << " extensions loaded";
    }

    bool Plugins::loadPlugin(Plugins::Plugin *plugin)
    {
        if (plugin->isLoaded()) {
            return true;
        }

        if (!initPlugin(PluginInterface::LateInitState, plugin)) {
            return false;
        }

        availablePlugins.removeOne(*plugin);
        availablePlugins.prepend(*plugin);

        refreshLoadedPlugins();

        return plugin->isLoaded();
    }

    void Plugins::unloadPlugin(Plugins::Plugin *plugin)
    {
        if (!plugin->isLoaded()) {
            return;
        }

        plugin->loader->unload();
        emit pluginUnloaded(plugin->instance);
        plugin->instance = nullptr;

        availablePlugins.removeOne(*plugin);
        availablePlugins.append(*plugin);

        refreshLoadedPlugins();
    }

    bool Plugins::addPlugin(const QString &fileName)
    {
        Plugin plugin = loadPlugin(fileName);
        registerAvailablePlugin(plugin);
        emit availablePluginsChanged();
        return true;
    }

    Plugins::Plugin Plugins::loadPlugin(const QString &fileName)
    {
        QString file = fileName;

#ifdef QT_DEBUG
        if (!file.contains("d.dll")) {
            file.append("d.dll"); // "ADebugPlugind.dll"
        }
#endif

#ifdef QT_NO_DEBUG
        if (!fileName.contains(".dll")) {
            file.append(".dll"); // "AReleasePlugin.dll"
        }
#endif

        const QDir pluginsDir(QCoreApplication::applicationDirPath() + QDir::separator() + "plugins");

        const QString fullPath = QDir(pluginsDir).absoluteFilePath(file);

        if (QFileInfo::exists(fullPath)) {
            QString file = fullPath;
        } else {
            qDebug() << "[Plugins] Plugin file not found:" << file;
        }

        QPluginLoader *pluginLoader = new QPluginLoader(file);

        /*if (pluginLoader->metaData().value("MetaData").type() != QJsonValue::Object) {
            qDebug() << "Invalid plugin (metadata json missing):" << fileName << pluginLoader->errorString();
            continue;
        }*/

        QJsonObject pluginMetaData = pluginLoader->metaData();

        Plugin plugin;
        plugin.pluginId   = pluginMetaData.value("MetaData").toObject().value("name").toString();
        plugin.pluginPath = file;
        plugin.loader     = new QPluginLoader(file);
        plugin.metaData   = Plugins::getMetaData(pluginMetaData);

        return plugin;
    }

    void Plugins::removePlugin(Plugins::Plugin *plugin)
    {
        if (!plugin->isRemovable()) {
            return;
        }

        if (plugin->isLoaded()) {
            unloadPlugin(plugin);
        }

        bool result = false;

        QFileInfo info(plugin->pluginPath);
        if (info.isDir()) {
            result = QDir(plugin->pluginPath).removeRecursively();
        } else if (info.isFile()) {
            result = QFile::remove(plugin->pluginPath);
        }

        if (!result) {
            qWarning() << "Failed to remove" << plugin->metaData.name;
            return;
        }

        availablePlugins.removeOne(*plugin);
        emit availablePluginsChanged();
    }

    void Plugins::loadAvailablePlugins()
    {
        if (pluginsLoaded) {
            return;
        }

        pluginsLoaded = true;

        // find SharedLibrary Plugins
        const QDir pluginsDir(QCoreApplication::applicationDirPath() + QDir::separator() + "plugins");

        QStringList nameFilters;
        nameFilters << "*.dll"; // << "*.so";

        const auto files = pluginsDir.entryInfoList(nameFilters, QDir::Files);

        for (const QFileInfo &file : files) {
            const QString fileName = file.absoluteFilePath();

            if (!QLibrary::isLibrary(fileName)) {
                qDebug() << "Invalid plugin (not a library):" << fileName;
                continue;
            }

            Plugin plugin = loadPlugin(fileName);

            registerAvailablePlugin(plugin);
        }
    }

    void Plugins::registerAvailablePlugin(const Plugin &plugin)
    {
        if (!availablePlugins.contains(plugin)) {
            availablePlugins.append(plugin);
        }
    }

    void Plugins::refreshLoadedPlugins()
    {
        loadedPlugins.clear();

        foreach (const Plugin &plugin, availablePlugins) {
            if (plugin.isLoaded()) {
                loadedPlugins.append(plugin.instance);
            }
        }

        emit availablePluginsChanged();
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

        Settings::SettingsManager settings;
        enabledPlugins = settings.get("Plugin-Settings/EnabledPlugins", defaultEnabledPlugins).toStringList();
    }

    bool Plugins::initPlugin(PluginInterface::InitState state, Plugins::Plugins::Plugin *plugin)
    {
        if (!plugin) {
            return false;
        }

        plugin->instance = qobject_cast<PluginInterface *>(plugin->loader->instance());

        if (!plugin->instance) {
            qDebug() << "[Plugins] Error loading plugin:" << plugin->pluginPath << "(" << plugin->loader->errorString()
                     << ")";

            return false;
        }

        plugin->instance->init(state);

        qDebug() << "Plugin loaded:" << plugin->pluginPath;

        return true;
    }

    void Plugins::shutdown()
    {
        foreach (PluginInterface *iPlugin, loadedPlugins) {
            iPlugin->unload();
        }
    }

} // namespace PluginsNS
