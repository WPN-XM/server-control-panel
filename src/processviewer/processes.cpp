#include "processes.h"

#include <QApplication>
#include <QDebug>
#include <QTime>

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
    refresh();

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

Process Processes::findByName(const QString &name)
{
    Process p;
    foreach(p, getProcessesList())
    {
        if (p.pid < 0) {
            continue; // if negative pid
        }
        if (name.contains(p.name)) { // || name == p.name || name + ".exe" == p.name
            return p; // if executable name matches
        }
    }
    p.name = "process not found";
    return p;
}

Process Processes::findByPid(const QString &pid)
{
    Process p;
    foreach(p, getProcessesList()) {
        if (p.pid == pid) {
            return p;
        }
    }
    p.name = "process not found";
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
                      << "php-cgi-spawner"
                      << "php-cgi"
                      << "mongod"
                      << "postgres"
                      << "redis-server";

    return processesToSearch;
}

bool Processes::areThereAlreadyRunningProcesses()
{
    qDebug() << "[Processes]" << "Check for already running processes.";

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

Processes::ProcessState Processes::getProcessState(const QString &processName) const
{
    Process p = findByName(processName);
    qDebug() << "[Processes] Getting Process State for " << processName << ".\n[Processes] Found Process?\t" << p.name;
    return (p.name == "process not found") ? ProcessState::NotRunning : ProcessState::Running;
}

// static
bool Processes::killProcess(qint64 pid)
{
    qDebug() << "going to kill process by pid:" << pid;

    HANDLE hProcess;

    hProcess = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_TERMINATE, FALSE, DWORD(pid));

    if (hProcess == NULL){
        qDebug() << "OpenProcess() failed, ecode:" << GetLastError();
        return false;
    }

    BOOL result = TerminateProcess(hProcess, 0); // 137 = SIGKILL

    CloseHandle(hProcess);

    if (result == 0){
        //qDebug() << "Error. Could not TerminateProcess with pid:" << pid;
        return false;
    }

    return true;
}

// static
bool Processes::killProcess(const QString &name)
{
    //qDebug() << "going to kill process by name:" << name;
    Process p = findByName(name);
    if (p.pid < 0) {
        return false;
    }
    qint64 pid = p.pid.toLong();
    return killProcess(pid);
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

// needed for Processes::startDetached.
// can be removed, when we compile with Qt5.8 where startDetached() is fixed. whenever that happens.
QString Processes::qt_create_commandline(const QString &program, const QStringList &arguments)
{
    QString cmd;

    cmd = "C:\\windows\\system32\\cmd.exe /c " + program + QLatin1Char(' ');

    for (int i=0; i<arguments.size(); ++i) {
        cmd += QLatin1Char(' ') + arguments.at(i);
    }

    return cmd;
}

// can be removed, when we compile with Qt5.8 where startDetached() is fixed.
bool Processes::startDetached(const QString &program, const QStringList &arguments, const QString &workingDir)
{
    bool success = false;
    static const DWORD errorElevationRequired = 740;
    PROCESS_INFORMATION pinfo;

    // the goal is to create a new process, which is able to survive, if the parent (wpn-xm.exe) is killed
    // we need DETACHED_PROCESS or CREATE_NEW_PROCESS_GROUP for this.

    // changed to parent-> runs cmd.exe with no window -> runs child -> parent kills cmd.exe (child gets parentless)

    DWORD dwCreationFlags = CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW;
    STARTUPINFOW startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
                                 (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                 (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                               };    

    //qDebug() << program, arguments;

    QString cmd = "C:\\windows\\system32\\cmd.exe /c " + program + QLatin1Char(' ');

    for (int i=0; i<arguments.size(); ++i) {
        cmd += QLatin1Char(' ') + arguments.at(i);
    }

    qDebug() << "[startDetached] Running command:" << cmd;

    success = CreateProcess(0, (wchar_t*)cmd.utf16(),
                            0, 0, FALSE, dwCreationFlags, 0,
                            workingDir.isEmpty() ? 0 : (wchar_t*)workingDir.utf16(),
                            &startupInfo, &pinfo);

    if (success) {
        CloseHandle(pinfo.hThread);
        CloseHandle(pinfo.hProcess);

        // if we have a cmd.exe parent, kill it, so that it's child gets independent/parentless
        if(cmd.contains("cmd.exe")) {
            delay(500);
            killProcess(pinfo.dwProcessId);
        }

    } else if (GetLastError() == errorElevationRequired) {
        qDebug() << "[startDetached] errorElevationRequired";
        success = false;
    }

    return success;
}

bool Processes::start(const QString &program, const QStringList &arguments, const QString &workingDir)
{
    bool success = false;

    static const DWORD errorElevationRequired = 740;
    PROCESS_INFORMATION pinfo;
    DWORD dwCreationFlags = CREATE_UNICODE_ENVIRONMENT | CREATE_DEFAULT_ERROR_MODE | CREATE_NO_WINDOW;
    STARTUPINFOW startupInfo = { sizeof( STARTUPINFO ), 0, 0, 0,
                                 (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                 (ulong)CW_USEDEFAULT, (ulong)CW_USEDEFAULT,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                               };

    //qDebug() << program, arguments;

    QString cmd = program + QLatin1Char(' ');

    for (int i=0; i<arguments.size(); ++i) {
        cmd += QLatin1Char(' ') + arguments.at(i);
    }

    qDebug() << "[start] Running command:" << cmd;

    success = CreateProcess(0, (wchar_t*)cmd.utf16(),
                            0, 0, FALSE, dwCreationFlags, 0,
                            workingDir.isEmpty() ? 0 : (wchar_t*)workingDir.utf16(),
                            &startupInfo, &pinfo);

    if (success) {
        CloseHandle(pinfo.hThread);
        CloseHandle(pinfo.hProcess);

    } else if (GetLastError() == errorElevationRequired) {
        qDebug() << "[start] errorElevationRequired";
        success = false;
    }

    return success;
}

void Processes::delay(int millisecondsToWait)
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}
