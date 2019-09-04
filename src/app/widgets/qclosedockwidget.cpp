#include "qclosedockwidget.h"

namespace Widgets::DockWidgets
{
    QCloseDockWidget::QCloseDockWidget(const QString &title, QWidget *parent) : QDockWidget(title, parent)
    {
        // constructor
    }

    void QCloseDockWidget::closeEvent(QCloseEvent *event)
    {
        emit closed();

        QDockWidget::closeEvent(event);
    }
} // namespace Widgets::DockWidgets
