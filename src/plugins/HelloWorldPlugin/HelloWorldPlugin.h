#ifndef HELLOWORLDPLUGIN_H
#define HELLOWORLDPLUGIN_H

#include "../../app/plugins/plugininterface.h"

#include "configdialog.h"

#include <QPointer>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

namespace Plugin_HelloWorld_NS
{
    class Plugin_HelloWorld : public QObject, public Plugins::PluginInterface
    {
        Q_OBJECT
        Q_INTERFACES(Plugins::PluginInterface)
        Q_PLUGIN_METADATA(IID "WPN-XM.ServerControlPanel.Plugin.HelloWorld" FILE "HelloWorldPlugin.json")

    public:
        // the class has to implement all pure virtual methods from PluginInterface
        ~Plugin_HelloWorld() {}

        void init(InitState state) override;
        void unload() override;

        QString getName() const override;

        QWidget *getConfigDialog();
        QString getConfigDialogTreeMenuEntry();

        void showSettings(QWidget *parent = nullptr) override;

        // bool loadDefaultSettings(Settings::SettingsManager *settings) override;

        // when the custom config dialog is closed with ok. you can read the parameters from your custom dialog.
        // bool saveSettings(Settings::SettingsManager *settings) override;

    private:
        QPointer<QDialog> settings;
        // Settings::SettingsManager *m_settings;
    };

} // namespace Plugin_HelloWorld_NS

#endif // HELLOWORLDPLUGIN_H
