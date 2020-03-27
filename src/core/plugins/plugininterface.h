#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>

#include "../common.h"

class QMenu;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;

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
        virtual void showSettings(QWidget *parent = 0) { Q_UNUSED(parent) }
        // virtual bool loadSettings(Settings::SettingsManager *settings) = 0;
        // virtual bool saveSettings(Settings::SettingsManager *settings) = 0;
        // virtual void onSettingChanged(const QString &setting);

        virtual void populateExtensionsMenu(QMenu *menu) { Q_UNUSED(menu) }

        virtual bool mouseDoubleClick(Common::ObjectName type, QObject *obj, QMouseEvent *event)
        {
            Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false;
        }
        virtual bool mousePress(Common::ObjectName type, QObject *obj, QMouseEvent *event)
        {
            Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false;
        }
        virtual bool mouseRelease(Common::ObjectName type, QObject *obj, QMouseEvent *event)
        {
            Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false;
        }
        virtual bool mouseMove(Common::ObjectName type, QObject *obj, QMouseEvent *event)
        {
            Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false;
        }
        virtual bool wheelEvent(Common::ObjectName type, QObject *obj, QWheelEvent *event)
        {
            Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false;
        }

        virtual bool keyPress(Common::ObjectName type, QObject *obj, QKeyEvent *event)
        {
            Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false;
        }
        virtual bool keyRelease(Common::ObjectName type, QObject *obj, QKeyEvent *event)
        {
            Q_UNUSED(type) Q_UNUSED(obj) Q_UNUSED(event) return false;
        }
    };

} // namespace PluginsNS

Q_DECLARE_INTERFACE(PluginsNS::PluginInterface, "WPN-XM.ServerControlPanel.PluginInterface/1.0")

#endif // PLUGININTERFACE_H
