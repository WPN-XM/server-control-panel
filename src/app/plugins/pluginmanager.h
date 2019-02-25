#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QWidget>

class QListWidgetItem;

namespace Ui
{
    class PluginList;
}

namespace Plugins
{
    class PluginManager : public QWidget
    {
        Q_OBJECT

    public:
        explicit PluginManager(QWidget *parent = 0);
        ~PluginManager();

        void load();
        void save();

        QStringList getConfigTreeMenuItem();

    private slots:
        void refresh();

    private:
        bool loaded;

        Ui::PluginList *ui;
    };
} // namespace Plugins

#endif // PLUGINMANAGER_H
