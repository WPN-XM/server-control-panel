#ifndef HELLOWORLDPLUGIN_H
#define HELLOWORLDPLUGIN_H

#include <QObject>
#include <QtPlugin>

#include "../../app/pluginmanager/plugininterface.h"

class HelloWorldPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.wpn-xm.server-control-panel.HelloWorldPlugin")
    Q_INTERFACES(PluginInterface)

public:
    // the class has to implement all pure virtual methods from PluginInterface
    ~HelloWorldPlugin() {}

    void onLoad();
    QString getName() const;

    // QWidget getConfigPage();

    bool loadDefaultSettings(Settings::SettingsManager *settings);
    bool saveSettings(Settings::SettingsManager *settings);

private:
    Settings::SettingsManager *m_settings;
};

#endif // HELLOWORLDPLUGIN_H
