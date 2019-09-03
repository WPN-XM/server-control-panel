#ifndef CONSOLEDOCKWIDGET_H
#define CONSOLEDOCKWIDGET_H

#include "widgets/qclosedockwidget.h"
#include "mainwindow.h"

#include <QPlainTextEdit>
#include <QStatusBar>
#include <QToolButton>
#include <QLabel>
#include <QTimer>

namespace Widgets
{

    class ConsoleDockWidget : public QCloseDockWidget
    {
        Q_OBJECT

    public:
        ConsoleDockWidget(QMainWindow *parent = nullptr);

    private:
        void createStatusBar();
        void createDockWidget();
        void setMainWindowDefaultSize();

        QMainWindow *mainWindow;
        QToolButton *toolButton{};
        QPlainTextEdit *plainTextEdit{};

        // MainWindow Default Size
        int defaultWidth  = 621;
        int defaultHeight = 424;

    private slots:
        // console dockWidget related
        void toolButton_Console_clicked();
        void dockWidgetCloseClicked();
        void dockWidgetTopLevelChanged(bool);
        void dockWidgetVisibilityChanged(bool);
        void restoreDock();
    };

} // namespace Widgets

#endif // CONSOLEDOCKWIDGET_H
