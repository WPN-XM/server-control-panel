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
#include <QThread>

#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <TlHelp32.h>

// Need to link with Iphlpapi.lib for GetExtendedTcpTable() used in getPorts()
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

namespace Ui {
    class ProcessViewerDialog;
}

struct Process
{
    QString name;        // 1
    QString path;
    QString pid;         // 2
    QString ppid;
    QString port;        // 3
    QString memoryUsage; // 4
    QIcon icon;
};


struct PidAndPort {
    QString pid;
    QString port;
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

        QList<Process> getRunningProcesses();
        QStringList getProcessDetails(DWORD processID);

        QString getSizeHumanReadable(float bytes);

        void renderProcesses();
        void refreshProcesses();

        bool killProcess(quint64 pid);

        QList<PidAndPort> getPorts();

        enum Columns {
            COLUMN_NAME = 0,
            COLUMN_PID  = 1,
            COLUMN_PORT = 2,
            COLUMN_MEM  = 3,
        };

        void filter(QString filterByItem, const QString &query);

    private slots:
        void on_lineEdit_searchProcessByName_textChanged(const QString &query);
        void on_lineEdit_searchProcessByPid_textChanged(const QString &query);
        void on_lineEdit_searchProcessByPort_textChanged(const QString &query);

        void on_pushButton_Refresh_released();
        void on_pushButton_KillProcess_released();
        void on_checkBox_filterExcludeWindowsProcesses_checked();        

};

#endif // PROCESSVIEWERDIALOG_H
