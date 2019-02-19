#include "plugins.h"

#include <QDebug>

Plugins::Plugins(QObject *parent) : QObject(parent) {}

PluginMetaData Plugins::getMetaData(const QJsonObject &metaData)
{
    PluginMetaData pluginMetaData;

    QJsonObject metaDataObject = metaData.value("MetaData").toObject();

    pluginMetaData.iid             = metaData.value("IID").toString();
    pluginMetaData.name            = metaDataObject.value("name").toString();
    pluginMetaData.version         = metaDataObject.value("pluginVersion").toString();
    pluginMetaData.author          = metaDataObject.value("author").toString();
    pluginMetaData.type            = metaDataObject.value("type").toString();
    pluginMetaData.compiler        = metaDataObject.value("compiler").toString();
    pluginMetaData.operatingSystem = metaDataObject.value("operatingSystem").toString();
    pluginMetaData.dependencies    = metaDataObject.value("dependencies").toBool();
    pluginMetaData.description     = metaDataObject.value("description").toString();
    // pluginMetaData.dependenciesPath = metaDataObject.value("libPaths").toArray();

    return pluginMetaData;
}
