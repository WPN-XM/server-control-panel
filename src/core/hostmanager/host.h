#ifndef HOST_H
#define HOST_H

#include "common.h"

// Windows / C++
#include <Windows.h>
#include <shellapi.h>
#include <cstdlib>
#include <string>

#include <QDir>
#include <QFile>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QThread>

namespace HostsFileManager
{
    class APP_CORE_EXPORT Host
    {
    public:
        explicit Host();
        explicit Host(const QString &name, const QString &address);

        static QString getHostFile();

        static QList<Host *> GetHosts();
        static void SetHosts(QList<Host *> listHosts);

        void setHostname(const QString &host);
        QString getHostname();

        void setIp(const QString &ip);
        QString getIp();

        bool isEnabled();
        void setEnabled(bool enable);

        bool operator==(const Host &host) const;

    private:
        QString hostname;
        QString ipaddress;
        bool enabled;
    };
} // namespace HostsFileManager

#endif // HOST_H
