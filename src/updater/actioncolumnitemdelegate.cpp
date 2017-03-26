#include "actioncolumnitemdelegate.h"
#include "updaterdialog.h"

namespace Updater
{

    ActionColumnItemDelegate::ActionColumnItemDelegate(QObject *parent)
    {
        Q_UNUSED(parent);

        btn = new QPushButton;
        btn->setVisible(false);
        setPushButtonStyle(btn);

        bar = new QProgressBar;
        bar->setVisible(false);
        setProgressBarStyle(bar);

        currentRow = -1;
    }

    ActionColumnItemDelegate::~ActionColumnItemDelegate()
    {
        delete btn;
        delete bar;
    }

    void ActionColumnItemDelegate::paint(QPainter *painter,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
    {
        int type = index.data(WidgetRole).toInt();

        switch (type) {
        case DownloadPushButton:
            return drawDownloadPushButton(painter, option, index);
        case DownloadProgressBar:
            return drawDownloadProgressBar(painter, option, index);
        case InstallPushButton:
            return drawInstallPushButton(painter, option, index);
        default:
            QStyledItemDelegate::paint(painter, option, index);
        }
    }

    void ActionColumnItemDelegate::drawDownloadPushButton(
        QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionButton opt;
        opt.initFrom(btn);
        opt.rect = option.rect.adjusted(2, 2, -2, -2);
        opt.text = "Download";
        opt.features |= QStyleOptionButton::DefaultButton;
        opt.state =
            QStyle::State_Enabled | QStyle::State_Active | QStyle::State_Raised;

        // change style of button, when clicked. based on currentRow set in
        // editorEvent().
        opt.state |=
            (currentRow == index.row()) ? QStyle::State_Sunken : QStyle::State_Raised;

        // hover on MouseOver
        if (option.state & QStyle::State_MouseOver) {
            painter->fillRect(option.rect.adjusted(+1, +1, -1, -1), Qt::darkGray);
        }

        btn->style()->drawControl(QStyle::CE_PushButton, &opt, painter, btn);
    }

    void ActionColumnItemDelegate::drawInstallPushButton(
        QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionButton opt;
        opt.initFrom(btn);
        opt.rect = option.rect.adjusted(2, 2, -2, -2);
        opt.text = "Install";
        opt.state = QStyle::State_Enabled;
        opt.features |= QStyleOptionButton::DefaultButton;

        // change style of button, when clicked. based on currentRow set in
        // editorEvent().
        opt.state |=
            (currentRow == index.row()) ? QStyle::State_Sunken : QStyle::State_Raised;

        // hover on MouseOver
        if (option.state & QStyle::State_MouseOver) {
            painter->fillRect(option.rect.adjusted(+1, +1, -1, -1), Qt::darkGray);
        }

        btn->style()->drawControl(QStyle::CE_PushButton, &opt, painter, btn);
    }

    void ActionColumnItemDelegate::drawDownloadProgressBar(
        QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionProgressBarV2 opt;
        opt.initFrom(bar);
        opt.rect = option.rect;
        opt.rect.adjust(3, 3, -3, -3);
        opt.textVisible = true;
        opt.textAlignment = Qt::AlignCenter;
        opt.state =
            QStyle::State_Enabled | QStyle::State_Active | QStyle::State_Raised;

        // get progress
        QMap<QString, QVariant> progress = index.model()->data(index).toMap();

        QString text = QString::fromLatin1(" %1 %2 %3 ")
                           .arg(progress["percentage"].toString())
                           .arg(progress["size"].toString())
                           .arg(progress["speed"].toString());
        //.arg(progress["time"].toString());
        //.arg(progress["eta"].toString());

        // set progress
        opt.minimum = 0;
        opt.maximum = progress["bytesTotal"].toFloat();
        opt.progress = progress["bytesReceived"].toFloat();
        opt.text = text;

        bar->style()->drawControl(QStyle::CE_ProgressBar, &opt, painter, bar);
    }

    void ActionColumnItemDelegate::setPushButtonStyle(QPushButton *btn) const
    {
        QString style =
            "QPushButton { border: 1px solid #CCC; background-color: "
            "#EEE; text-align: center;}";
        style +=
            "QPushButton:pressed {background-color: #d1d7da; border-width: 1px; "
            "border-color: #666;}";
        style += "QPushButton:hover:!pressed { border: 1px solid red; }";
        style += "QPushButton:hover { background-color:yellow; }";

        btn->setStyleSheet(style);
    }

    void ActionColumnItemDelegate::setProgressBarStyle(QProgressBar *bar) const
    {
        QString style =
            "QProgressBar {border: 1px solid #CCC; background-color: "
            "#EEE; text-align: center;}";
        style += "QProgressBar::chunk {background-color: #d1d7da; margin: 0.5px;}";

        bar->setStyleSheet(style);

        // bar->setStyleSheet(QString("QProgressBar::chunk:horizontal {background:
        // qlineargradient(x1: 0, y1: 0.5, x2: 1, y2: 0.5, stop: 0 grey, stop: 1
        // grey);}")+QString("QProgressBar::horizontal {border: 1px solid gray;
        // border-radius: 3px; background: yellow; padding: 0px; text-align: left;
        // margin-right: 4ex;}"));
    }

    /**
* Qt makes it really hard to add widgets and events to tableviews
* The cell doesn't contain an editor, but the editorEvent() allows to bind to
* events:
* so it's a hack to intercept the mouseClicks on the table cell.
*/
    bool ActionColumnItemDelegate::editorEvent(QEvent *event,
                                               QAbstractItemModel *model,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index)
    {
        Q_UNUSED(option);

        currentRow = -1;

        if (event->type() != QEvent::MouseButtonRelease &&
            event->type() != QEvent::MouseButtonPress) {
            return false;
        }

        if (event->type() == QEvent::MouseButtonPress) {

            currentRow = index.row();

            if (index.data(WidgetRole).toInt() == DownloadPushButton) {
                emit downloadButtonClicked(index);
                return true;
            }
            if (index.data(WidgetRole).toInt() == InstallPushButton) {
                emit installButtonClicked(index);
                return true;
            }
        }

        if (event->type() == QEvent::MouseButtonRelease) {

            currentRow = -1;

            if (index.data(WidgetRole).toInt() == DownloadPushButton) {
                model->setData(index, DownloadProgressBar, WidgetRole);
                model->dataChanged(index, index);
                return true;
            }
            if (index.data(WidgetRole).toInt() == DownloadProgressBar) {
                model->setData(index, InstallPushButton, WidgetRole);
                model->dataChanged(index, index);
                return true;
            }
        }

        return false;
    }
}
