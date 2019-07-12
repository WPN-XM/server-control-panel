#ifndef HELLOWORLDPLUGIN_H
#define HELLOWORLDPLUGIN_H

#include "../../app/plugins/plugininterface.h"

#include "configdialog.h"

#include <QPointer>
#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

//! Interface of the Plugin
/*!
 * This is the interface of the plugin.
 * It inherits from both QObject and PluginInterface.
 * The class has to implement all pure virtual methods from PluginInterface.
 *
 * We have to override these functions:
 * - `init()`, which is called when the plugin is loaded
 * - `unload()`, which is called when the plugin is unloaded
 * - `getName()`
 * - `showSettings()`
 */
namespace Plugin_HelloWorld_NS
{
    class Plugin_HelloWorld : public QObject, public PluginsNS::PluginInterface
    {
        Q_OBJECT
        Q_INTERFACES(PluginsNS::PluginInterface)
        Q_PLUGIN_METADATA(IID "WPN-XM.ServerControlPanel.Plugin.HelloWorld" FILE "HelloWorldPlugin.json")

    public:
        /*!
         * Constructor.
         * Don't do anything expensive in the constructor!
         * It will be called, even if the plugin is not on the "active plugins" list.
         */
        Plugin_HelloWorld();

        /*! Deconstructor. */
        ~Plugin_HelloWorld();

        /*!
         * The is called right after the plugin is loaded.
         *
         * State can be either StartupInitState or LateInitState.
         * Tt indicates when the plugin has been loaded.
         * Currently, it can be from the Configuration Dialog "Plugins" or automatically at startup.
         *
         * Automatically at startup means that Plugins are loaded before MainWindow is created.
         *
         * @param state Indicate when the plugin have been loaded (at startup or after).
         */
        void init(InitState state) override;

        /*! This function will be called when unloading plugin. */
        void unload() override;

        /*! This function returns a nice name of the plugin. */
        QString getName() const override;

        QWidget *getConfigDialog();
        QString getConfigDialogTreeMenuEntry();

        /*!
         * This function will be called from the Plugin Configuration Dialog,
         * after clicking on the Settings button.
         * The Settings button will be enabled, when PluginProperties.hasSettings == true.
         *
         * @param parent Parent widget.
         */
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
