#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "plugins.h"
#include "plugininterface.h"

class PluginManager
{
public:
    PluginManager();
    QList<Plugins::Plugin> getAvailablePlugins();

private:
    bool pluginsLoaded = false;
    QList<Plugins::Plugin> availablePlugins;

    void loadAvailablePlugins();
    Plugins::Plugin loadSharedLibraryPlugin(const QString &name);
};

#endif // PLUGINMANAGER_H
