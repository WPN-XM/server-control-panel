#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>

#include "../settings.h"

class PluginInterface
{

public:
    PluginInterface() {}
    virtual ~PluginInterface() {}

    virtual void onLoad() = 0;

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
    virtual bool loadDefaultSettings(Settings::SettingsManager *settings) = 0;
    virtual bool saveSettings(Settings::SettingsManager *settings)        = 0;
    // virtual bool loadSettings();
    // virtual void onSettingChanged(const QString &setting);

signals:
    // Signal to be emitted in getName()
    // void name(QString);

public slots:
};

#define PluginInterfaceIID "org.wpn-xm.Server-Control-Panel.PluginInterface/1.0"

Q_DECLARE_INTERFACE(PluginInterface, PluginInterfaceIID)

#endif // PLUGININTERFACE_H
