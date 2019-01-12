#ifndef PROCESSVIEWERDIALOG_H
#define PROCESSVIEWERDIALOG_H

#include "src/file/csv.h"
#include "src/processviewer/processes.h"

#include <QDesktopWidget>
#include <QDialog>
#include <QProcess>
#include <QThread>
#include <QTreeWidgetItem>

namespace Ui
{
    class ProcessViewerDialog;
}

class ProcessViewerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProcessViewerDialog(QWidget *parent);
    ~ProcessViewerDialog();

    QTreeWidgetItem *addRoot(Processes::Process process);
    void addChild(QTreeWidgetItem *parent, Processes::Process process);

    void setChecked_ShowOnlyWpnxmProcesses();
    void setProcessesInstance(Processes *p);

private:
    Ui::ProcessViewerDialog *ui;
    Processes *processes;

    QList<Processes::Process> runningProcesses;
    QList<Processes::PidAndPort> ports;

    void renderProcesses();
    void refreshProcesses();

    QList<Processes::PidAndPort> getPorts();

    enum Columns
    {
        COLUMN_NAME = 0,
        COLUMN_PID  = 1,
        COLUMN_PORT = 2,
        COLUMN_MEM  = 3,
    };

    void filter(const QString &filterByItem, const QString &query);

private slots:
    void on_lineEdit_searchProcessByName_textChanged(const QString &query);
    void on_lineEdit_searchProcessByPid_textChanged(const QString &query);
    void on_lineEdit_searchProcessByPort_textChanged(const QString &query);

    void on_pushButton_Refresh_released();
    void on_pushButton_KillProcess_released();
    void on_checkBox_filterExcludeWindowsProcesses_stateChanged(int state);
    void on_checkBox_filterShowOnlyWpnxmProcesses_stateChanged(int state);
};

#endif // PROCESSVIEWERDIALOG_H
