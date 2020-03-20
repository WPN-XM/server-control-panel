#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>

//#include "../settings.h"

namespace PluginsNS
{

    class PluginInterface
    {

    public:
        enum InitState
        {
            StartupInitState,
            LateInitState
        };

        // PluginInterface() {}
        // PluginInterface()          = delete;
        virtual ~PluginInterface() = default;

        virtual void init(InitState state) = 0;
        virtual void unload()              = 0;

        // slot which should cause emission of `name` signal
        virtual QString getName() const = 0;
        // virtual QString getShortName() const;
        // virtual QString getVersion() const;
        // virtual QString getIcon() const;
        // virtual QString getDescription() const;
        // virtual QString getAuthor() const;
        // virtual QString getEmail() const;

        // settings related
        virtual void showSettings(QWidget *parent = nullptr) = 0;
        // virtual bool loadSettings(Settings::SettingsManager *settings) = 0;
        // virtual bool saveSettings(Settings::SettingsManager *settings) = 0;
        // virtual void onSettingChanged(const QString &setting);
    };

} // namespace PluginsNS

Q_DECLARE_INTERFACE(PluginsNS::PluginInterface, "WPN-XM.ServerControlPanel.PluginInterface/1.0")

#endif // PLUGININTERFACE_H
