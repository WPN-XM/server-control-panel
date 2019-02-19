#include "pluginmanager.h"

#include <QString>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QPluginLoader>

PluginManager::PluginManager() {}

QList<Plugins::Plugin> PluginManager::getAvailablePlugins()
{
    loadAvailablePlugins();

    return availablePlugins;
}

void PluginManager::loadAvailablePlugins()
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
        qWarning() << fileName;

        QPluginLoader *pluginLoader = new QPluginLoader(fileName);

        if (pluginLoader->metaData().value("MetaData").type() != QJsonValue::Object) {
            qDebug() << "Invalid plugin : " << fileName << " config missing " << fileName
                     << pluginLoader->errorString();
            continue;
        }

        QObject *pluginInstance = pluginLoader->instance();
        if (!pluginInstance) {
            qDebug() << "Error loading plugin : " << fileName << pluginLoader->errorString();
            continue;
        } else {
            qDebug() << "Plugin loaded : " << fileName;
        }

        QJsonObject pluginMetaData = pluginLoader->metaData().value("MetaData").toObject();
        QString pluginName         = pluginMetaData.value("name").toString();

        PluginInterface *pluginObject = qobject_cast<PluginInterface *>(pluginInstance);

        // const QMetaObject *pluginMeta = pluginInstance->metaObject();

        Plugins::Plugin plugin;
        plugin.id          = pluginName;
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
