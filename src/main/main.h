#ifndef MAIN_H
#define MAIN_H

#include "../core/cli.h"
#include "../core/mainwindow.h"
#include "../core/splashscreen.h"

#include "../core/settings.h"
#include "../core/processviewer/processes.h"
#include "../core/plugins/plugins.h"

//#include "../core/processviewer/processviewerdialog.h"
//#include "../core/processviewer/AlreadyRunningProcessesDialog.h"

#include <QSharedMemory>
#include <QtCore>
#include <QtWidgets>

namespace ServerControlPanel
{
    class Main : public QObject
    {
        Q_OBJECT

    public:
        explicit Main(QObject *parent = nullptr);
        static void exitIfAlreadyRunning();
        static void exitIfNotInAppFolder();
    };
} // namespace ServerControlPanel

#endif // MAIN_H
