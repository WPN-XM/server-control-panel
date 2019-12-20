#ifndef MAIN_H
#define MAIN_H

#include "../cli.h"
#include "../mainwindow.h"
#include "../splashscreen.h"
//#include "version.h"

#include "../settings.h"
#include "../processviewer/processes.h"
#include "../plugins/plugins.h"

//#include "../processviewer/processviewerdialog.h"
//#include "../processviewer/AlreadyRunningProcessesDialog.h"

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
