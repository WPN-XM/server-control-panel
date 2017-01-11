#include "processviewerdialog.h"
#include "ui_processviewerdialog.h"

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

    ui->treeWidget->setColumnCount(4);
    ui->treeWidget->setSortingEnabled(true);

    // resize columns to contents
    ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_NAME, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_PID, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_PORT, QHeaderView::ResizeToContents);

    // set text alignment for columns
    //ui->treeWidget->setText()

    renderProcesses();

    ui->treeWidget->expandAll();
}


QList<PidAndPort> ProcessViewerDialog::getPorts()
{
    QList<PidAndPort> ports;

    MIB_TCPTABLE_OWNER_PID *pTCPInfo;
    MIB_TCPROW_OWNER_PID *owner;
    DWORD size;
    DWORD result;

    result = GetExtendedTcpTable(NULL,     &size, false, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    pTCPInfo = (MIB_TCPTABLE_OWNER_PID*)malloc(size);
    result = GetExtendedTcpTable(pTCPInfo, &size, false, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);

    if (result != NO_ERROR)
    {
       qDebug() << "Couldn't get our IP table";
       return ports;
    }

    // iterate through tcpinfo table
    for (DWORD dwLoop = 0; dwLoop < pTCPInfo->dwNumEntries; dwLoop++)
    {
       owner = &pTCPInfo->table[dwLoop];

       // The dwLocalPort, and dwRemotePort members are in network byte order.
       // The ntohs() or inet_ntoa() functions in Windows Sockets or conversion is needed.
       // here's a trick, which saves header inclusion headache:
       long port = (owner->dwLocalPort/256) + (owner->dwLocalPort%256) * 256;

       PidAndPort p;
       p.pid = QString::number(owner->dwOwningPid);
       p.port = QString::number(port);

       ports.append(p);
    }

    return ports;
}

void ProcessViewerDialog::refreshProcesses()
{
    ui->treeWidget->clear();
    renderProcesses();
    ui->treeWidget->expandAll();
}

void ProcessViewerDialog::on_pushButton_Refresh_released()
{
    refreshProcesses();
}

void ProcessViewerDialog::renderProcesses()
{
    QList<Process> processes = getRunningProcesses();

    QList<PidAndPort> ports = getPorts();

    foreach(Process process, processes)
    {
        // find parentItem in the tree by looking for parentId recursivley
        QList<QTreeWidgetItem *> parentItem = ui->treeWidget->findItems(
            process.ppid, Qt::MatchContains | Qt::MatchRecursive, 1
        );

        // lookup port for this pid and add it to the process struct
        foreach (const PidAndPort &p, ports) {
            if (p.pid == process.pid) {
                process.port = p.port;
                break;
            }
        }

        // if there is a parent item, then add the proccess as a child, else it's a parent itself
        if(!parentItem.empty()) {
            addChild(parentItem.at(0), process);
        } else {
            addRoot(process);
        }
    }
}

void ProcessViewerDialog::on_lineEdit_searchProcessByName_textChanged(const QString &query)
{
    filter("byName", query);
}

void ProcessViewerDialog::on_lineEdit_searchProcessByPid_textChanged(const QString &query)
{
    filter("byPid", query);
}

void ProcessViewerDialog::on_lineEdit_searchProcessByPort_textChanged(const QString &query)
{
    filter("byPort", query);
}

/**
 * Search for items in the Progess TreeWidget by using type-ahead search
 */
void ProcessViewerDialog::filter(QString filterByItem, const QString &query)
{
    int searchInColumn = Columns::COLUMN_NAME;

    if(filterByItem == "byName") {
        searchInColumn = Columns::COLUMN_NAME;
    }
    if(filterByItem == "byPort") {
        searchInColumn = Columns::COLUMN_PORT;
    }
    if(filterByItem == "byPID") {
        searchInColumn = Columns::COLUMN_PID;
    }

    ui->treeWidget->expandAll();

    // Iterate over all child items : filter items with "contains" query
    QTreeWidgetItemIterator iterator(ui->treeWidget, QTreeWidgetItemIterator::All);
    while(*iterator)
    {
        QTreeWidgetItem *item = *iterator;
        if(item && item->text(searchInColumn).contains(query, Qt::CaseInsensitive)) {
            item->setHidden(false);
        } else {
            // Problem: the matched child is visibile, but parent is hidden, because no match.
            // so, lets hide only items without childs.
            // any not matching parent will stay visible.. until next iteration, see below.
            if(item->childCount() == 0) {
                item->setHidden(true);
            }
        }
        ++iterator;
    }

    // Iterate over items with childs : hide, if they do not have a matching (visible) child (see above).
    QTreeWidgetItemIterator parentIterator(ui->treeWidget, QTreeWidgetItemIterator::HasChildren);
    while(*parentIterator)
    {
        QTreeWidgetItem *item = *parentIterator;
        // count the number of hidden childs
        int childs = item->childCount();
        int hiddenChilds = 0;
        for (int i = 0; i < childs; ++i) {
            if(item->child(i)->isHidden()) {
                ++hiddenChilds;
            }
        }
        // finally: if all childs are hidden, hide the parent (*item), too
        if(hiddenChilds == childs) {
            item->setHidden(true);
        }
        ++parentIterator;
    }
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
    item->setData(0, Qt::ItemDataRole::ToolTipRole, process.path);
    item->setText(1, process.pid);
    item->setText(2, process.port);
    item->setText(3, process.memoryUsage);
    ui->treeWidget->addTopLevelItem(item);
    return item;
}

void ProcessViewerDialog::addChild(QTreeWidgetItem *parent, Process process)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, process.name);
    item->setIcon(0, process.icon);
    item->setData(0, Qt::ItemDataRole::ToolTipRole, process.path);
    item->setText(1, process.pid);
    item->setText(2, process.port);
    item->setText(3, process.memoryUsage);
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

void ProcessViewerDialog::on_pushButton_KillProcess_released()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    quint64 pid = item->text(1).toLong();
    if(killProcess(pid)) {
        QObject().thread()->usleep(1000*1000*0.5); // 0,5sec
        refreshProcesses();
    }
}

bool ProcessViewerDialog::killProcess(quint64 pid)
{
    HANDLE hProcess;
    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if(hProcess){
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
        return true;
    }
    return false;
}

void ProcessViewerDialog::on_checkBox_filterExcludeWindowsProcesses_checked()
{
    // remove PIDs below 1000
    // remove names with c:\windows
}
