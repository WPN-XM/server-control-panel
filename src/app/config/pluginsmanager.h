#ifndef PLUGINSMANAGER_H
#define PLUGINSMANAGER_H

#include <QWidget>
#include <QMessageBox>

class QListWidgetItem;

namespace Configuration
{
    namespace Ui
    {
        class PluginsManager;
    }

    class PluginsManager : public QWidget
    {
        Q_OBJECT

    public:
        explicit PluginsManager(QWidget *parent = 0);
        ~PluginsManager();

        void load();
        void save();

        QString menuName = "Plugins";

    private slots:
        void settingsClicked();
        void currentChanged(QListWidgetItem *item);
        void itemChanged(QListWidgetItem *item);

        void refresh();

    private:
        void sortItems();

        Ui::PluginsManager *ui;
        bool loaded;
    };
} // namespace Configuration

#endif // PLUGINSMANAGER_H
