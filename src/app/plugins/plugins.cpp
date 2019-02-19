#include "plugins.h"

#include <QDebug>

Plugins::Plugins(QObject *parent) : QObject(parent) {}

PluginMetaData Plugins::getMetaData(const QJsonObject &metaData)
{
    PluginMetaData pluginMetaData;

    QJsonObject metaDataObject = metaData.value("MetaData").toObject();

    pluginMetaData.iid          = metaData.value("IID").toString();
    pluginMetaData.name         = metaDataObject.value("name").toString();
    pluginMetaData.version      = metaDataObject.value("version").toString();
    pluginMetaData.description  = metaDataObject.value("description").toString();
    pluginMetaData.type         = metaDataObject.value("type").toString();
    pluginMetaData.core         = metaDataObject.value("core").toBool();
    QJsonArray authorsJsonArray = metaDataObject.value("authors").toArray();

    foreach (const QJsonValue &value, authorsJsonArray) {
        QJsonObject obj = value.toObject();
        pluginMetaData.authors.insert(obj["name"].toString(), obj["email"].toString());
    }

    // pluginMetaData.dependencies     = metaDataObject.value("dependencies").toBool();
    // pluginMetaData.dependenciesPath = metaDataObject.value("libPaths").toArray();

    return pluginMetaData;
}
