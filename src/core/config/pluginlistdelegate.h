#ifndef PLUGINLISTDELEGATE_H
#define PLUGINLISTDELEGATE_H

#include "common.h"

#include <QApplication>
#include <QListWidget>
#include <QPainter>
#include <QStyledItemDelegate>

namespace PluginsNS
{

    class APP_CORE_EXPORT PluginListDelegate : public QStyledItemDelegate
    {
    public:
        explicit PluginListDelegate(QListWidget *parent);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    private:
        mutable int m_rowHeight;
        mutable int m_padding;
    };

} // namespace PluginsNS
#endif // PLUGINLISTDELEGATE_H
