#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QWidget>
#include <QMessageBox>

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
        void settingsClicked();
        void currentChanged(QListWidgetItem *item);
        void itemChanged(QListWidgetItem *item);
        void refresh();

    private:
        void sortItems();
        bool loaded;
        Ui::PluginList *ui;
    };
} // namespace Plugins

#endif // PLUGINMANAGER_H
