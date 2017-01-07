#ifndef PROCESSVIEWERDIALOG_H
#define PROCESSVIEWERDIALOGg_H

#include "src/csv.h"

#include <QDialog>
#include <QTreeWidgetItem>
#include <QProcess>
#include <QDebug>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QFileIconProvider>

#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <TlHelp32.h>

namespace Ui {
    class ProcessViewerDialog;
}

struct Process
{
    QString pid;
    QString ppid;
    QString name;
    QString path;
    QString memoryUsage;
    QIcon icon;
};

class ProcessViewerDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit ProcessViewerDialog(QWidget *parent = 0);
        ~ProcessViewerDialog();

        QTreeWidgetItem* addRoot(Process process);
        void addChild(QTreeWidgetItem *parent, Process process);

    private:
        Ui::ProcessViewerDialog *ui;

        QString getSizeHumanReadable(float bytes);
        QList<Process> getRunningProcesses();
        QStringList getProcessDetails(DWORD processID);

    private slots:
        void on_searchProcessLineEdit_textChanged(const QString &query);
};

#endif // PROCESSVIEWERDIALOG_H
