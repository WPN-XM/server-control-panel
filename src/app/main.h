#ifndef MAIN_H
#define MAIN_H

#include "../version.h"
#include "../mainwindow.h"
#include "../settings.h"
#include "../splashscreen.h"
#include "../cli.h"

#include "../processviewer/processes.h"
//#include "../processviewer/processviewerdialog.h"
#include "../processviewer/AlreadyRunningProcessesDialog.h"

#include <QtWidgets>
#include <QtCore>
#include <QSharedMemory>

namespace ServerControlPanel
{
    class Main : public QObject
    {
        Q_OBJECT

        public:
            explicit Main(QObject *parent = 0);
            static void exitIfAlreadyRunning();
    };
}

#endif // MAIN_H
