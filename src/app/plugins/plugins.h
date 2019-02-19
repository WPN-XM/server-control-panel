#ifndef PLUGINS_H
#define PLUGINS_H

#include <QObject>
#include <QPluginLoader>
//#include <QPixmap>
//#include <QIcon>
#include <QFileInfo>
#include <QDir>

#include "plugininterface.h"

class PluginMetaData
{
public:
    QString iid;
    QString name;
    QString pluginName;
    QString version;
    QString author;
    QString type;
    QString compiler;
    QString operatingSystem;
    bool dependencies;
    // QJsonArray dependenciesPath;
    QString description;
    QString path;
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
    static PluginMetaData getMetaData(const QJsonObject &metaData);

signals:

public slots:
};

Q_DECLARE_METATYPE(Plugins::Plugin)

#endif // PLUGINS_H
