#ifndef QCLOSEDOCKWIDGET_H
#define QCLOSEDOCKWIDGET_H

#include <QDockWidget>

namespace Widgets
{

    class QCloseDockWidget : public QDockWidget
    {
        Q_OBJECT

    public:
        QCloseDockWidget(const QString &title = "", QWidget *parent = nullptr);

    protected:
        void closeEvent(QCloseEvent *event);
    signals:
        void closed();
    };

} // namespace Widgets

#endif // QCLOSEDOCKWIDGET_H
