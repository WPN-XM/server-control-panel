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
            QString pluginPath;
            PluginMetaData metaData;
            PluginInterface *instance = nullptr;
            QPluginLoader *loader     = nullptr;

            bool isLoaded() const;
            bool isRemovable() const;
            bool operator==(const Plugin &other) const;
        };

        explicit Plugins(QObject *parent = nullptr);

        QList<Plugin> getAvailablePlugins();

        bool loadPlugin(Plugin *plugin);
        void unloadPlugin(Plugin *plugin);
        void removePlugin(Plugin *plugin);

        bool addPlugin(const QString &fileName);

        void shutdown();

    public slots:
        void loadSettings();
        void loadPlugins();

    protected:
        QList<PluginInterface *> loadedPlugins;

    signals:
        void pluginUnloaded(PluginsNS::PluginInterface *plugin);
        void availablePluginsChanged();

    private:
        bool pluginsLoaded = false;

        QList<Plugin> availablePlugins;
        QStringList enabledPlugins{};

        void registerAvailablePlugin(const Plugin &plugin);

        void refreshLoadedPlugins();
        void loadAvailablePlugins();

        Plugin loadPlugin(const QString &fileName);
        bool initPlugin(PluginInterface::InitState state, Plugin *plugin);

        static PluginMetaData getMetaData(const QJsonObject &metaData);
    };

} // namespace PluginsNS

Q_DECLARE_METATYPE(PluginsNS::Plugins::Plugin)

#endif // PLUGINS_H
