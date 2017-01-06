#include "processviewerdialog.h"
#include "ui_processviewerdialog.h"

#include <QFileInfo>
#include <QFileIconProvider>

#include <Windows.h>
#include <tlhelp32.h>

ProcessViewerDialog::ProcessViewerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProcessViewerDialog)
{
    ui->setupUi(this);

    setWindowTitle("Process Viewer");

    // remove question mark from the title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // center dialog
    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());

    ui->treeWidget->setColumnCount(3);

    QList<Process> processes = getRunningProcesses();

    foreach(Process process, processes)
    {
        // find parentItem in the tree by looking for parentId recursivley
        QList<QTreeWidgetItem *> parentItem = ui->treeWidget->findItems(
            process.ppid, Qt::MatchContains | Qt::MatchRecursive, 1
        );

        // if there is a parent item, then add the proccess as a child, else it's a parent itself
        if(!parentItem.empty()) {
            addChild(parentItem.at(0), process);
        } else {
            addRoot(process);
        }
    }

    // resize columns to contents
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    ui->treeWidget->expandAll();
}

ProcessViewerDialog::~ProcessViewerDialog()
{
    delete ui;
}

QTreeWidgetItem* ProcessViewerDialog::addRoot(Process process)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, process.name);
    item->setIcon(0, process.icon);
    item->setText(1, process.pid);
    item->setText(2, process.memoryUsage);
    ui->treeWidget->addTopLevelItem(item);
    return item;
}

void ProcessViewerDialog::addChild(QTreeWidgetItem *parent, Process process)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, process.name);
    item->setIcon(0, process.icon);
    item->setText(1, process.pid);
    item->setText(2, process.memoryUsage);
    parent->addChild(item);
}

QString ProcessViewerDialog::getSizeHumanReadable(float bytes)
{
     QStringList list;
     list << "KB" << "MB";

     QStringListIterator i(list);
     QString unit("bytes");

     while(bytes >= 1024.0 && i.hasNext())
      {
         unit = i.next();
         bytes /= 1024.0;
     }
     return QString::fromLatin1("%1 %2").arg(bytes, 3, 'f', 1).arg(unit);
}

QList<Process> ProcessViewerDialog::getRunningProcesses()
{
    QList<Process> processes;

    PROCESSENTRY32 pe;

    // set the size of the structure before using it
    pe.dwSize = sizeof(PROCESSENTRY32);

    // take a snapshot of all processes in the system
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return processes;
    }

    BOOL hasNext = Process32First(hSnapshot, &pe);

    while( hasNext )
    {
        QStringList details = getProcessDetails(pe.th32ProcessID);

        Process p;
        p.pid       = QString::number( (int) pe.th32ProcessID);
        p.ppid      = QString::number( (int) pe.th32ParentProcessID);
        p.name      = QString::fromWCharArray(pe.szExeFile);

        if(!details.empty()) {
            p.path          = details.at(0);
            p.memoryUsage   = details.at(1);

            // get icon
            QFileInfo fileInfo = QFileInfo(p.path);
            QFileIconProvider fileicon;
            p.icon = fileicon.icon(fileInfo);
        }

        processes.append(p);

        hasNext = Process32Next(hSnapshot, &pe);
    }

    CloseHandle(hSnapshot);

    return processes;
}


QStringList ProcessViewerDialog::getProcessDetails(DWORD processID)
{
    QStringList processInfos;

    // init: get a handle to the process
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    // we will inevitable run into processes that don't let us peek at them,
    // because we don't have enough rights. including the "System" process,
    // which is a place-holder for ring0 code. let's move on, nothing to see there...
    if(!hProcess) {
        return processInfos;
    }

    // 0 = get executable path
    WCHAR szProcessPath[MAX_PATH];
    DWORD bufSize = MAX_PATH;
    QueryFullProcessImageNameW( hProcess, 0,(LPWSTR) &szProcessPath, &bufSize);
    QString processPath = QString::fromUtf16((ushort*)szProcessPath, bufSize);
    processInfos.append(processPath);

    // 1 - add memory usage
    PROCESS_MEMORY_COUNTERS pmc;
    if(GetProcessMemoryInfo(hProcess,&pmc, sizeof(pmc))){
        QString memoryUsage = getSizeHumanReadable((float) pmc.WorkingSetSize);
        processInfos.append(memoryUsage);
    }
    CloseHandle(hProcess);

    return processInfos;
}

