#include "plugins.h"

Plugins::Plugins(QObject *parent) : QObject(parent) {}

PluginMetaData Plugins::getMetaData(const QJsonObject &metaData)
{
    PluginMetaData pluginMetaData;

    pluginMetaData.iid             = metaData.value("IID").toString();
    pluginMetaData.name            = metaData.value("MetaData").toObject().value("name").toString();
    pluginMetaData.version         = metaData.value("MetaData").toObject().value("pluginVersion").toString();
    pluginMetaData.author          = metaData.value("MetaData").toObject().value("author").toString();
    pluginMetaData.compiler        = metaData.value("MetaData").toObject().value("compiler").toString();
    pluginMetaData.operatingSystem = metaData.value("MetaData").toObject().value("operatingSystem").toString();
    pluginMetaData.dependencies    = metaData.value("MetaData").toObject().value("dependencies").toBool();
    pluginMetaData.description     = metaData.value("MetaData").toObject().value("description").toString();
    // pluginMetaData.dependenciesPath = metaData.value("MetaData").toObject().value("libPaths").toArray();

    return pluginMetaData;
}
