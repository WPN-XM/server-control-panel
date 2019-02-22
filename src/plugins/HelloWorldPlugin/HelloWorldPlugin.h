#ifndef HELLOWORLDPLUGIN_H
#define HELLOWORLDPLUGIN_H

#include "../../app/plugins/plugininterface.h"

class HelloWorldPlugin : public QObject, public Plugins::PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(Plugins::PluginInterface)
    Q_PLUGIN_METADATA(IID "WPN-XM.ServerControlPanel.Plugin.HelloWorld" FILE "HelloWorldPlugin.json")

public:
    // the class has to implement all pure virtual methods from PluginInterface
    ~HelloWorldPlugin() {}

    void onLoad() override;
    QString getName() const override;

    void showSettings(QWidget *parent = nullptr) override;
    // bool loadDefaultSettings(Settings::SettingsManager *settings) override;
    // bool saveSettings(Settings::SettingsManager *settings) override;

private:
    // Settings::SettingsManager *m_settings;
};

#endif // HELLOWORLDPLUGIN_H
