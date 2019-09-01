#include "consoledockwidget.h"

namespace Widgets
{

    ConsoleDockWidget::ConsoleDockWidget(QMainWindow *parent)
    {
        mainWindow = parent;

        createDockWidget();

        createStatusBar();

        connect(this, &Widgets::QCloseDockWidget::closed, this, &ConsoleDockWidget::dockWidgetCloseClicked);

        // let the floating dockwidget window return to it's mainwindow position, when clicking close
        connect(this, &QDockWidget::visibilityChanged, this, &ConsoleDockWidget::dockWidgetVisibilityChanged);

        // resize mainwindow, when the dockwidget goes into floating mode
        connect(this, &QDockWidget::topLevelChanged, this, &ConsoleDockWidget::dockWidgetTopLevelChanged);
    }

    void ConsoleDockWidget::createDockWidget()
    {
        /** Create a box for the debuglog text and add it to a BottomDockWidget. **/

        auto *plainTextEdit = new QPlainTextEdit();
        plainTextEdit->setPlainText("Debug Log Stream");
        plainTextEdit->setObjectName("DebugConsolePlainTextEdit");
        plainTextEdit->setReadOnly(true);
        plainTextEdit->setFixedHeight(100);
        plainTextEdit->setFixedWidth(621);

        /*auto *dockWidget = new Widgets::QCloseDockWidget("Debug Console", this);
        dockWidget->setObjectName("DebugConsole");
        dockWidget->setWidget(plainTextEdit);
        dockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
        dockWidget->setFloating(false);*/

        setObjectName("DebugConsole");
        setWidget(plainTextEdit);
        setAllowedAreas(Qt::BottomDockWidgetArea);
        setFloating(false);

        // hide console when starting
        hide();

        // resize mainwindow
        setMainWindowDefaultSize();
    }

    void ConsoleDockWidget::createStatusBar()
    {
        /** Create a StatusBar with a toggle button for a debug console. **/

        auto *toolButton = new QToolButton();
        toolButton->setObjectName("Console");
        toolButton->setArrowType(Qt::DownArrow);
        toolButton->setToolTip("Open Debug Console");

        auto *labelPlaceholderRightSide = new QLabel();
        labelPlaceholderRightSide->setText(" ");

        auto *statusBar = new QStatusBar();
        statusBar->setSizeGripEnabled(false);
        statusBar->addPermanentWidget(toolButton);
        statusBar->addPermanentWidget(labelPlaceholderRightSide);
        statusBar->setStyleSheet("QStatusBar { border-top: 1px solid #ccc; } QStatusBar::item { border: 0px; } ");

        mainWindow->setStatusBar(statusBar);

        // Toggle Console with ToolButton in the StatusBar
        connect(toolButton, SIGNAL(clicked()), this, SLOT(toolButton_Console_clicked()));
    }

    void ConsoleDockWidget::dockWidgetCloseClicked()
    {
        setMainWindowDefaultSize();

        // disallow dockwidget window to be resized
        auto *textedit = mainWindow->findChild<QPlainTextEdit *>("DebugConsolePlainTextEdit");
        textedit->setMaximumHeight(100);

        // after clicking close on the dockwidget, switch to "open" on the toggle button
        auto *toolButton = mainWindow->findChild<QToolButton *>("Console");
        toolButton->setArrowType(Qt::DownArrow);
        toolButton->setToolTip("Open Debug Console");
    }

    void ConsoleDockWidget::setMainWindowDefaultSize()
    {
        // make the mainwindow smaller again, after dock was closed
        mainWindow->setFixedSize(defaultWidth, defaultHeight);
    }

    void ConsoleDockWidget::dockWidgetTopLevelChanged(bool topLevelChanged)
    {
        auto *dockWidget = qobject_cast<QDockWidget *>(sender());

        if (dockWidget->isFloating() && topLevelChanged) {
            setMainWindowDefaultSize();

            // allow dockwidget window to be resized (expand policy)
            dockWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

            // set new max height only on the inner textedit widget
            auto *textedit = mainWindow->findChild<QPlainTextEdit *>("DebugConsolePlainTextEdit");
            textedit->setMinimumHeight(100);
            textedit->setMaximumHeight(600);
        }
    }

    void ConsoleDockWidget::dockWidgetVisibilityChanged(bool visible)
    {
        auto *dockWidget = qobject_cast<QDockWidget *>(sender());

        if (dockWidget->isFloating() && !visible) {
            QTimer::singleShot(100, this, &ConsoleDockWidget::restoreDock);
        }
    }

    void ConsoleDockWidget::restoreDock()
    {
        // auto *dockWidget = qobject_cast<QDockWidget *>(sender());
        auto *dockWidget = mainWindow->findChild<QDockWidget *>("DebugConsole");

        dockWidget->setFloating(false);
        dockWidget->setVisible(false);
    }

    void ConsoleDockWidget::toolButton_Console_clicked()
    {
        auto *toolButton = mainWindow->findChild<QToolButton *>("Console");
        auto *dockWidget = mainWindow->findChild<QDockWidget *>("DebugConsole");

        if (toolButton->arrowType() == Qt::DownArrow) {
            // when open console was clicked, do this:
            dockWidget->show();
            mainWindow->setFixedSize(defaultWidth, defaultHeight + dockWidget->height() - 5);
            toolButton->setArrowType(Qt::UpArrow);
            toolButton->setToolTip("Close Debug Console");
        } else {
            // when close console was clicked, do this:
            if (dockWidget->isFloating()) {
                dockWidget->setFloating(false);
            }
            dockWidget->hide();
            setMainWindowDefaultSize();
            toolButton->setArrowType(Qt::DownArrow);
            toolButton->setToolTip("Open Debug Console");
        }
    }
} // namespace Widgets
