#ifndef MAIN_H
#define MAIN_H

#include "../cli.h"
#include "../mainwindow.h"
#include "../settings.h"
#include "../splashscreen.h"
#include "../version.h"

#include "../processviewer/processes.h"
//#include "../processviewer/processviewerdialog.h"
#include "../processviewer/AlreadyRunningProcessesDialog.h"

#include "../plugins/pluginmanager.h"

#include <QSharedMemory>
#include <QtCore>
#include <QtWidgets>

namespace ServerControlPanel
{
    class Main : public QObject
    {
        Q_OBJECT

    public:
        explicit Main(QObject *parent = 0);
        static void exitIfAlreadyRunning();
    };
} // namespace ServerControlPanel

#endif // MAIN_H
