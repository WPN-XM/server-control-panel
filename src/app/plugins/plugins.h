#ifndef PLUGINS_H
#define PLUGINS_H

#include <QObject>
#include <QPluginLoader>
//#include <QPixmap>
//#include <QIcon>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QMap>
#include <QCoreApplication>
#include <QDebug>

#include "plugininterface.h"
#include "settings.h"

namespace PluginsNS
{

    class PluginMetaData
    {
    public:
        QString iid;
        QString name;
        QString description;
        QString version;
        QMap<QString, QString> authors;
        QString type;
        QString icon;
        bool core;
        bool hasSettings = false;
        // QPixmap icon;
        // @TODO plugin dependencies?
    };

    class Plugins : public QObject
    {
        Q_OBJECT
    public:
        struct Plugin
        {
            QString pluginId;
            PluginMetaData metaData;
            PluginInterface *instance = nullptr;

            QString libraryPath;
            QPluginLoader *loader = nullptr;

            bool isLoaded() const { return instance; }

            bool operator==(const Plugin &other) const { return this->pluginId == other.pluginId; }
        };

        explicit Plugins(QObject *parent = nullptr);

        QList<Plugin> getAvailablePlugins();

        bool loadPlugin(Plugin *plugin);
        void unloadPlugin(Plugin *plugin);

        void shutdown();

    public slots:
        void loadSettings();
        void loadPlugins();

    protected:
        QList<PluginInterface *> loadedPlugins;

    signals:
        void pluginUnloaded(PluginInterface *plugin);
        void refreshedLoadedPlugins();

    private:
        bool pluginsLoaded = false;

        QList<Plugin> availablePlugins;
        QStringList enabledPlugins{};

        void loadAvailablePlugins();
        void refreshLoadedPlugins();

        bool initPlugin(PluginInterface::InitState state, Plugin *plugin);

        static PluginMetaData getMetaData(const QJsonObject &metaData);
    };

} // namespace PluginsNS

Q_DECLARE_METATYPE(PluginsNS::Plugins::Plugin)

#endif // PLUGINS_H
