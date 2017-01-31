#include "processes.h"
#include <QApplication>

#include <QDebug>

// initialize static members
Processes *Processes::theInstance = NULL;
QList<Process> Processes::processesList;
QList<PidAndPort> Processes::portsList;
QList<Process> Processes::monitoredProcessesList;

Processes *Processes::getInstance()
{
    if (theInstance == NULL) {
        theInstance = new Processes();
    }

    return theInstance;
}

void Processes::release()
{
    if (theInstance != NULL) {
        delete theInstance;
    }
    theInstance = NULL;
}

Processes::Processes()
{
    refresh();
}

QList<Process> Processes::getProcessesList()
{
    return processesList;
}
QList<PidAndPort> Processes::getPortsList()
{
    return portsList;
}
QList<Process> Processes::getMonitoredProcessesList()
{
    return monitoredProcessesList;
}

void Processes::refresh()
{
    processesList = getRunningProcesses();
    portsList     = getPorts();
}

Process Processes::find(const QString &name)
{
    Process p;
    foreach(p, getProcessesList())
    {
        if (p.pid < 0) {            
            continue; // if negative pid
        }
        if (p.name != name && p.name != name + ".exe") {
            continue; // if executable name doesn't match
        }
        return p;
    }
    return p;
}

QStringList Processes::getProcessNamesToSearchFor()
{
    QStringList processesToSearch;

    processesToSearch << "nginx"
                      << "apache"
                      << "memcached"
                      << "mysqld"
                      << "spawn"
                      << "php-cgi"
                      << "mongod"
                      << "postgres"
                      << "redis-server";

    return processesToSearch;
}

bool Processes::areThereAlreadyRunningProcesses()
{
    qDebug() << "[Processes] Check for already running processes.";

    QStringList processesToSearch = getProcessNamesToSearchFor();

    // foreach processesToSearch take a look in the runningProcessesList
    for (int i = 0; i < processesToSearch.size(); ++i)
    {
        qDebug() << "Searching for process: " << processesToSearch.at(i).toLocal8Bit().constData();
        foreach(Process process, getProcessesList()) {
            if(process.name.contains( processesToSearch.at(i).toLatin1().constData() )) {
                qDebug() << "Found: " << process.name;
                monitoredProcessesList.append(process);
            }
        }
    }

    // only show the "process shutdown" dialog, when there are processes to shutdown
    return (!monitoredProcessesList.isEmpty());
}

// static
QList<Process> Processes::getRunningProcesses()
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

// static
QStringList Processes::getProcessDetails(DWORD processID)
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

// static
QList<PidAndPort> Processes::getPorts()
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
       //qDebug() << "Couldn't get our IP table";
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

// static
bool Processes::killProcess(quint64 pid)
{
    //qDebug() << "going to kill process by pid:" << pid;
    HANDLE hProcess;
    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if(hProcess){
        TerminateProcess(hProcess, 0);
        CloseHandle(hProcess);
        return true;
    }
    return false;
}

// static
bool Processes::killProcess(const QString &name)
{
    //qDebug() << "going to kill process by name:" << name;
    Process p = find(name);
    if (p.pid < 0) {
        //qDebug() << "could not find Process for process name: " << name;
        return false;
    }
    return killProcess(p.pid);
}

// static
QString Processes::getSizeHumanReadable(float bytes)
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
