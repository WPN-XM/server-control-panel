#ifndef ALREADYRUNNINGPROCESSESDIALOG_H
#define ALREADYRUNNINGPROCESSESDIALOG_H

#include "common.h"
#include "../processviewer/processes.h"
#include "../servers.h"

#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QGroupBox>
#include <QLabel>
#include <QObject>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

class APP_CORE_EXPORT AlreadyRunningProcessesDialog : public QDialog
{
    Q_OBJECT

public:
    AlreadyRunningProcessesDialog(QWidget *parent);
    void checkAlreadyRunningServers();
    /*signals:
      void signalUpdateServerStatusIndicators();*/
};

#endif // ALREADYRUNNINGPROCESSESDIALOG_H
