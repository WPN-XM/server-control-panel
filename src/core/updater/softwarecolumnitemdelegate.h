#ifndef SOFTWARECOLUMNITEMDELEGATE_H
#define SOFTWARECOLUMNITEMDELEGATE_H

#include "common.h"

#include <QDesktopServices>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>

namespace Updater
{

    class APP_CORE_EXPORT SoftwareColumnItemDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        explicit SoftwareColumnItemDelegate(QObject *parent = nullptr);
        ~SoftwareColumnItemDelegate() override;

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        bool editorEvent(QEvent *event,
                         QAbstractItemModel *model,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index) override;

    private:
        QLabel *label;

    public slots:
        void onClickedOpenURL(const QModelIndex &index);
    };
} // namespace Updater

#endif // SOFTWARECOLUMNITEMDELEGATE_H
