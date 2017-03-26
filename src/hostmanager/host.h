#ifndef HOST_H
#define HOST_H

// Windows / C++
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
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
        explicit Host(QString strName, QString strAddress);

        static QList<Host *> GetHosts();
        static void SetHosts(QList<Host *> listHosts);

        QString name();
        void setName(QString strName);

        QString address();
        void setAddress(QString strAddress);

        // bool isEnable();
        // void setEnable(bool bEnable);

        bool operator==(const Host &host) const;

    private:
        static QString getHostFile();

        // bool m_bIsEnable;
        QString strName;
        QString strAddress;
    };
}

#endif // HOST_H
