#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>

//#include "../settings.h"

namespace Plugins
{

    class PluginInterface
    {

    public:
        PluginInterface() {}
        virtual ~PluginInterface() {}

        enum InitState
        {
            StartupInitState,
            LateInitState
        };

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

        // virtual bool enable();
        // virtual bool disable();

        // settings related
        virtual void showSettings(QWidget *parent = nullptr) = 0;
        // virtual bool loadSettings(Settings::SettingsManager *settings) = 0;
        // virtual bool saveSettings(Settings::SettingsManager *settings) = 0;
        // virtual void onSettingChanged(const QString &setting);

    signals:
        // Signal to be emitted in getName()
        // void name(QString);

    public slots:
    };

} // namespace Plugins

Q_DECLARE_INTERFACE(Plugins::PluginInterface, "WPN-XM.ServerControlPanel.PluginInterface/1.0")

#endif // PLUGININTERFACE_H
