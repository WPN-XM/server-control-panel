#ifndef PROCESSES_H
#define PROCESSES_H

#include <QObject>
#include <QIcon>
#include <QFileInfo>
#include <QFileIconProvider>

#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <TlHelp32.h>

// Need to link with Iphlpapi.lib for GetExtendedTcpTable() used in getPorts()
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

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

class Processes : public QObject
{
    Q_OBJECT

    public:
        // singleton
        static Processes *getInstance();
        static void release();
        static Processes *theInstance;

        static QList<Process> getProcessesList();
        static QList<PidAndPort> getPortsList();
        static QList<Process> getMonitoredProcessesList();

        static bool killProcess(qint64 pid);
        static bool killProcess(const QString &name);

        static Process find(const QString &name);

        static QList<Process> getRunningProcesses();
        static QList<PidAndPort> getPorts();

        static bool areThereAlreadyRunningProcesses();

        static void refresh();       

        enum ProcessState {
            NotRunning,
            Starting,
            Running
        };
        Q_ENUM(ProcessState)

        ProcessState getProcessState(QString processName) const;

        static bool startDetached(const QString &program, const QStringList &arguments, const QString &workingDirectory = QString());
        static bool startDetached(const QString &program, const QStringList &arguments);
        static bool startDetached(const QString &command);

        static void delay(int millisecondsToWait);

    private:
        // constructor is private, because singleton
        explicit Processes();
        static QList<Process> processesList;
        static QList<PidAndPort> portsList;
        static QList<Process> monitoredProcessesList;

        static QStringList getProcessDetails(DWORD processID);
        static QString getSizeHumanReadable(float bytes);

        static QStringList getProcessNamesToSearchFor();

        static QString qt_create_commandline(const QString &program, const QStringList &arguments);
};

#endif // PROCESSES_H
