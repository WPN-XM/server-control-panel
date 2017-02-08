#ifndef ALREADYRUNNINGPROCESSESDIALOG_H
#define ALREADYRUNNINGPROCESSESDIALOG_H

#include "../processviewer/processes.h"
#include "../version.h"
#include "../servers.h"

#include <QObject>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDialog>
#include <QApplication>
#include <QProcess>
#include <QDir>

class AlreadyRunningProcessesDialog : public QDialog
{
    Q_OBJECT

    public:
        AlreadyRunningProcessesDialog(QWidget *parent = false);
        void checkAlreadyRunningServers(Processes *vprocesses);
};

#endif // ALREADYRUNNINGPROCESSESDIALOG_H
