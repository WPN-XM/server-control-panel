#ifndef PLUGINLISTDELEGATE_H
#define PLUGINLISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QListWidget>

namespace Plugins
{

    class PluginListDelegate : public QStyledItemDelegate
    {
    public:
        explicit PluginListDelegate(QListWidget *parent);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
        mutable int m_rowHeight;
        mutable int m_padding;
    };

} // namespace Plugins
#endif // PLUGINLISTDELEGATE_H
