#ifndef QCLOSEDOCKWIDGET_H
#define QCLOSEDOCKWIDGET_H

#include <QDockWidget>

namespace Widgets::DockWidgets
{

    class QCloseDockWidget : public QDockWidget
    {
        Q_OBJECT

    public:
        QCloseDockWidget(const QString &title = "", QWidget *parent = nullptr);

    protected:
        void closeEvent(QCloseEvent *event) override;
    signals:
        void closed();
    };

} // namespace Widgets::DockWidgets

#endif // QCLOSEDOCKWIDGET_H