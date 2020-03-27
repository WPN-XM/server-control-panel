#include "pluginproxy.h"
#include "plugininterface.h"
//#include "mainapplication.h"
//#include "settings.h"

#include <QMenu>

namespace PluginsNS
{

    PluginProxy::PluginProxy(QObject *parent) : Plugins(parent)
    {
        connect(this, &PluginProxy::pluginUnloaded, this, &PluginProxy::pluginUnloaded);
    }

    void PluginProxy::registerAppEventHandler(PluginProxy::EventHandlerType type, PluginInterface *obj)
    {
        switch (type) {
        case MouseDoubleClickHandler:
            if (!mouseDoubleClickHandlers.contains(obj)) {
                mouseDoubleClickHandlers.append(obj);
            }
            break;

        case MousePressHandler:
            if (!mousePressHandlers.contains(obj)) {
                mousePressHandlers.append(obj);
            }
            break;

        case MouseReleaseHandler:
            if (!mouseReleaseHandlers.contains(obj)) {
                mouseReleaseHandlers.append(obj);
            }
            break;

        case MouseMoveHandler:
            if (!mouseMoveHandlers.contains(obj)) {
                mouseMoveHandlers.append(obj);
            }
            break;

        case KeyPressHandler:
            if (!keyPressHandlers.contains(obj)) {
                keyPressHandlers.append(obj);
            }
            break;

        case KeyReleaseHandler:
            if (!keyReleaseHandlers.contains(obj)) {
                keyReleaseHandlers.append(obj);
            }
            break;

        case WheelEventHandler:
            if (!wheelEventHandlers.contains(obj)) {
                wheelEventHandlers.append(obj);
            }
            break;

        default:
            qWarning("PluginProxy::registerAppEventHandler registering unknown event handler type");
            break;
        }
    }

    void PluginProxy::pluginUnloaded(PluginInterface *plugin)
    {
        mousePressHandlers.removeOne(plugin);
        mouseReleaseHandlers.removeOne(plugin);
        mouseMoveHandlers.removeOne(plugin);
        wheelEventHandlers.removeOne(plugin);

        keyPressHandlers.removeOne(plugin);
        keyReleaseHandlers.removeOne(plugin);
    }

    void PluginProxy::populateExtensionsMenu(QMenu *menu)
    {
        if (!menu) {
            return;
        }

        for (PluginInterface *iPlugin : qAsConst(loadedPlugins)) {
            iPlugin->populateExtensionsMenu(menu);
        }
    }

    bool PluginProxy::processMouseDoubleClick(Common::ObjectName type, QObject *obj, QMouseEvent *event)
    {
        bool accepted = false;

        for (PluginInterface *iPlugin : qAsConst(mouseDoubleClickHandlers)) {
            if (iPlugin->mouseDoubleClick(type, obj, event)) {
                accepted = true;
            }
        }

        return accepted;
    }

    bool PluginProxy::processMousePress(Common::ObjectName type, QObject *obj, QMouseEvent *event)
    {
        bool accepted = false;

        for (PluginInterface *iPlugin : qAsConst(mousePressHandlers)) {
            if (iPlugin->mousePress(type, obj, event)) {
                accepted = true;
            }
        }

        return accepted;
    }

    bool PluginProxy::processMouseRelease(Common::ObjectName type, QObject *obj, QMouseEvent *event)
    {
        bool accepted = false;

        for (PluginInterface *iPlugin : qAsConst(mouseReleaseHandlers)) {
            if (iPlugin->mouseRelease(type, obj, event)) {
                accepted = true;
            }
        }

        return accepted;
    }

    bool PluginProxy::processMouseMove(Common::ObjectName type, QObject *obj, QMouseEvent *event)
    {
        bool accepted = false;

        for (PluginInterface *iPlugin : qAsConst(mouseMoveHandlers)) {
            if (iPlugin->mouseMove(type, obj, event)) {
                accepted = true;
            }
        }

        return accepted;
    }

    bool PluginProxy::processWheelEvent(Common::ObjectName type, QObject *obj, QWheelEvent *event)
    {
        bool accepted = false;

        for (PluginInterface *iPlugin : qAsConst(wheelEventHandlers)) {
            if (iPlugin->wheelEvent(type, obj, event)) {
                accepted = true;
            }
        }

        return accepted;
    }

    bool PluginProxy::processKeyPress(Common::ObjectName type, QObject *obj, QKeyEvent *event)
    {
        bool accepted = false;

        for (PluginInterface *iPlugin : qAsConst(keyPressHandlers)) {
            if (iPlugin->keyPress(type, obj, event)) {
                accepted = true;
            }
        }

        return accepted;
    }

    bool PluginProxy::processKeyRelease(Common::ObjectName type, QObject *obj, QKeyEvent *event)
    {
        bool accepted = false;

        for (PluginInterface *iPlugin : qAsConst(keyReleaseHandlers)) {
            if (iPlugin->keyRelease(type, obj, event)) {
                accepted = true;
            }
        }

        return accepted;
    }

    // void PluginProxy::emitACreated(A *a) { emit ACreated(a); }

    // void PluginProxy::emitADeleted(A *a) { emit ADeleted(a); }

} // namespace PluginsNS
