#ifndef HOST_H
#define HOST_H

// Windows / C++
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include <string>

#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QTemporaryFile>
#include <QDir>
#include <QThread>
#include <QList>
#include <QString>

namespace HostsFileManager
{
    class Host
    {
        public:
            explicit Host();
            explicit Host(QString strName, QString strAddress);

            static QList<Host*> GetHosts();
            static void SetHosts(QList<Host*> listHosts);

            QString name();
            void setName(QString strName);

            QString address();
            void setAddress(QString strAddress);

            //bool isEnable();
            //void setEnable(bool bEnable);

            bool operator==(const Host &host) const;

        private:
            static QString getHostFile();

            //bool m_bIsEnable;
            QString strName;
            QString strAddress;
    };

}

#endif // HOST_H
