#ifndef ACTIONCOLUMNITEMDELEGATE_H
#define ACTIONCOLUMNITEMDELEGATE_H

#include <QProgressBar>
#include <QPushButton>
#include <QStyledItemDelegate>

#include <QApplication>
#include <QPainter>
#include <QStylePainter>

#include <QSortFilterProxyModel>
#include <QTableView>

namespace Updater
{
    class ActionColumnItemDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        explicit ActionColumnItemDelegate(QObject *parent = 0);
        ~ActionColumnItemDelegate();

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        bool editorEvent(QEvent *event,
                         QAbstractItemModel *model,
                         const QStyleOptionViewItem &option,
                         const QModelIndex &index);

    signals:
        /**
   *  Signal that is emitted when the button has been clicked.
   *  @param index - index whose button was clicked
   */
        void downloadButtonClicked(const QModelIndex &index);
        void installButtonClicked(const QModelIndex &index);

    private:
        void drawDownloadPushButton(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const;
        void drawDownloadProgressBar(QPainter *painter,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const;
        void drawInstallPushButton(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const;
        QPushButton *btn;
        QProgressBar *bar;
        void setPushButtonStyle(QPushButton *btn) const;
        void setProgressBarStyle(QProgressBar *bar) const;
        int currentRow;

    public:
        enum WidgetRole
        {
            WidgetRole = Qt::UserRole + 1
        };
        enum WidgetTypes
        {
            DownloadPushButton = 1,
            DownloadProgressBar,
            InstallPushButton
        };
    };
}

#endif // ACTIONCOLUMNITEMDELEGATE_H
