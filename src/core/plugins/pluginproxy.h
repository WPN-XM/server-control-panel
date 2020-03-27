#ifndef PLUGINPROXY_H
#define PLUGINPROXY_H

#include "plugins.h"
#include "common.h"

#include <QMenu>

namespace PluginsNS
{

    // class A;

    class APP_CORE_EXPORT PluginProxy : public Plugins
    {
        Q_OBJECT

    public:
        enum EventHandlerType
        {
            MouseDoubleClickHandler,
            MousePressHandler,
            MouseReleaseHandler,
            MouseMoveHandler,
            KeyPressHandler,
            KeyReleaseHandler,
            WheelEventHandler
        };

        explicit PluginProxy(QObject *parent = nullptr);

        void registerAppEventHandler(EventHandlerType type, PluginInterface *obj);

        void populateExtensionsMenu(QMenu *menu);

        bool processMouseDoubleClick(Common::ObjectName type, QObject *obj, QMouseEvent *event);
        bool processMousePress(Common::ObjectName type, QObject *obj, QMouseEvent *event);
        bool processMouseRelease(Common::ObjectName type, QObject *obj, QMouseEvent *event);
        bool processMouseMove(Common::ObjectName type, QObject *obj, QMouseEvent *event);
        bool processWheelEvent(Common::ObjectName type, QObject *obj, QWheelEvent *event);
        bool processKeyPress(Common::ObjectName type, QObject *obj, QKeyEvent *event);
        bool processKeyRelease(Common::ObjectName type, QObject *obj, QKeyEvent *event);

        // void emitACreated(A *a);
        // void emitADeleted(A *a);

    Q_SIGNALS:
        // void ACreated(A *a);
        // void ADeleted(A *a);

    private Q_SLOTS:
        void pluginUnloaded(PluginInterface *plugin);

    private:
        QList<PluginInterface *> mouseDoubleClickHandlers;
        QList<PluginInterface *> mousePressHandlers;
        QList<PluginInterface *> mouseReleaseHandlers;
        QList<PluginInterface *> mouseMoveHandlers;

        QList<PluginInterface *> wheelEventHandlers;

        QList<PluginInterface *> keyPressHandlers;
        QList<PluginInterface *> keyReleaseHandlers;
    };

} // namespace PluginsNS

#endif // PLUGINPROXY_H
