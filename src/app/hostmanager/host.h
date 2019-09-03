#ifndef HOST_H
#define HOST_H

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
    class Host
    {
    public:
        explicit Host();
        explicit Host(const QString &name, const QString &address);

        static QList<Host *> GetHosts();
        static void SetHosts(QList<Host *> listHosts);

        QString hostname;
        QString ipaddress;
        // bool isEnable();
        // void setEnable(bool bEnable);

        bool operator==(const Host &host) const;

    private:
        static QString getHostFile();
        // bool m_bIsEnable;
    };
} // namespace HostsFileManager

#endif // HOST_H
