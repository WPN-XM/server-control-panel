#include "softwarecolumnitemdelegate.h"

#include "updaterdialog.h"

#include <QPalette>

namespace Updater
{

    SoftwareColumnItemDelegate::SoftwareColumnItemDelegate(QObject *parent)
    {
        Q_UNUSED(parent);

        label = new QLabel();
        label->setAlignment(Qt::AlignLeft);
    }

    SoftwareColumnItemDelegate::~SoftwareColumnItemDelegate() { delete label; }

    void SoftwareColumnItemDelegate::paint(QPainter *painter,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
    {
        painter->save();

        QString softwareName =
            index.model()
                ->data(index.model()->index(
                    index.row(), UpdaterDialog::Columns::SoftwareComponent))
                .toString();
        QString websiteURL = index.model()
                                 ->data(index.model()->index(
                                     index.row(), UpdaterDialog::Columns::WebsiteURL))
                                 .toString();

        QString link = "<img src=\":/home.png\"><a href=\"" + websiteURL + "\">" +
                       softwareName + "</a>";

        QTextDocument document;

        if (option.state & QStyle::State_MouseOver) {
            // draw stuff which appears on mouse over
            document.setDefaultStyleSheet(
                "a { text-decoration: none; color: darkblue; }");
        } else {
            // draw stuff that appears when mouse is not over control
            document.setDefaultStyleSheet("a { text-decoration: none; color: black; }");
        }

        document.setTextWidth(option.rect.width());
        document.setHtml(link);
        painter->translate(option.rect.topLeft());
        document.drawContents(painter);

        painter->restore();
        return;
    }

    /**
* Qt makes it really hard to add widgets and events to tableviews
* The cell doesn't contain an editor, but this allows to bind to events...
* so its a hack to intercept the mouseclick on the table cell.
*/
    bool SoftwareColumnItemDelegate::editorEvent(QEvent *event,
                                                 QAbstractItemModel *model,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index)
    {
        Q_UNUSED(model);
        Q_UNUSED(option);

        if (event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseButtonDblClick) {
            emit onClickedOpenURL(index);
        }
        return 0;
    }

    void SoftwareColumnItemDelegate::onClickedOpenURL(const QModelIndex &index)
    {
        QString websiteURL = index.model()
                                 ->data(index.model()->index(
                                     index.row(), UpdaterDialog::Columns::WebsiteURL))
                                 .toString();
        QDesktopServices::openUrl(QUrl(websiteURL));
    }
}
