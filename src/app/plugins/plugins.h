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

namespace Plugins
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
        bool hasSettings;

        // bool dependencies;
        // QJsonArray dependenciesPath;

        // QString path;
    };

    class Plugins : public QObject
    {
        Q_OBJECT
    public:
        struct Plugin
        {
            QString id;
            QString libraryPath;
            PluginMetaData metaData;
            PluginInterface *instance = nullptr;

            QPluginLoader *loader = nullptr;

            bool isLoaded() const { return instance; }

            bool operator==(const Plugin &other) const { return this->id == other.id; }
        };

        explicit Plugins(QObject *parent = nullptr);

        QList<Plugin> getAvailablePlugins();

        bool loadPlugin(Plugin *plugin);
        void unloadPlugin(Plugin *plugin);

        void shutdown();

    protected:
        QList<PluginInterface *> loadedPlugins;

    private:
        bool pluginsLoaded = false;

        QList<Plugin> availablePlugins;
        QStringList enabledPlugins;

        void loadAvailablePlugins();
        void refreshLoadedPlugins();

        bool initPlugin(PluginInterface::InitState state, Plugin *plugin);

        static PluginMetaData getMetaData(const QJsonObject &metaData);

    signals:
        void pluginUnloaded(PluginInterface *plugin);
        void refreshedLoadedPlugins();

    public slots:
        void loadSettings();
    };

} // namespace Plugins

Q_DECLARE_METATYPE(Plugins::Plugins::Plugin)

#endif // PLUGINS_H
