#ifndef SOFTWARECOLUMNITEMDELEGATE_H
#define SOFTWARECOLUMNITEMDELEGATE_H

#include <QDesktopServices>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTextDocument>

namespace Updater
{

    class SoftwareColumnItemDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        explicit SoftwareColumnItemDelegate(QObject *parent = 0);
        ~SoftwareColumnItemDelegate();

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        bool editorEvent(QEvent *event,
                         QAbstractItemModel *model,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index);

    private:
        QLabel *label;

    public slots:
        void onClickedOpenURL(const QModelIndex &index);
    };
}

#endif // SOFTWARECOLUMNITEMDELEGATE_H
