#include "host.h"

namespace HostsFileManager
{
    QList<Host *> Host::GetHosts()
    {
        QList<Host *> listReturn;

        QFile hostFile(getHostFile());
        if (hostFile.open(QFile::ReadOnly)) {
            QTextStream hostStream(&hostFile);
            QString strLine;
            while (!(strLine = hostStream.readLine()).isNull()) {
                strLine = strLine.trimmed();
                QStringList list = strLine.split(QRegExp("[ \t]"), QString::SkipEmptyParts);

                // skip comment line
                if (list.empty() || list.startsWith("#"))
                    continue;

                Host *host = new Host(list[1], list[0]);
                listReturn << host;
            }
        }

        return listReturn;
    }

    void Host::SetHosts(QList<Host *> listHosts)
    {
        QFile hostFile(getHostFile());
        QTemporaryFile tempFile;
        tempFile.setAutoRemove(false);
        if (hostFile.open(QFile::ReadOnly) && tempFile.open()) {
            QTextStream hostStream(&hostFile);
            QTextStream tempStream(&tempFile);

            QString strLine;
            while (!(strLine = hostStream.readLine()).isNull()) {
                if (strLine.trimmed().isEmpty() || strLine.startsWith("#")) {
                    tempStream << strLine << "\r\n";
                    continue;
                }

                // It's a host line
                QStringList lst = strLine.split(QRegExp("[ \t]"), QString::SkipEmptyParts);

                Host hostFromFile(lst[1], lst[0]);
                int index = listHosts.indexOf(&hostFromFile);
                // It match an host setup
                if (index >= 0) {
                    Host *host = listHosts.takeAt(index);
                    tempStream << host->address() << "       " << host->name() << "\r\n";
                }
            }

            // Copy remaining hosts
            tempStream << "\r\n";
            while (!listHosts.isEmpty()) {
                Host *host = listHosts.takeFirst();
                tempStream << host->address() << "       " << host->name() << "\r\n";
            }

            // Cleanup
            hostFile.close();
            tempFile.close();
        }

        // Copy content of tempfile to host file
        QString strHostFile = QDir::toNativeSeparators(hostFile.fileName());
        QString strTempFile = QDir::toNativeSeparators(tempFile.fileName());

        QString strArguments = "/c copy /y \"" + strTempFile + "\" \"" + strHostFile + "\"";
        std::wstring tmp = strArguments.toStdWString();
        LPCTSTR wcArguments = tmp.c_str();

        SHELLEXECUTEINFO shExecInfo;
        shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
        shExecInfo.fMask = 0; // NULL;
        shExecInfo.hwnd = NULL;
        shExecInfo.lpVerb = L"runas";
        shExecInfo.lpFile = L"cmd.exe";
        shExecInfo.lpParameters = wcArguments;
        shExecInfo.lpDirectory = NULL;
        shExecInfo.nShow = SW_MAXIMIZE;
        shExecInfo.hInstApp = NULL;

        ShellExecuteEx(&shExecInfo);
    }

    QString Host::getHostFile()
    {
        QString windir;

        // "getenv("windir") is deprecated, use _dupenv_s"
        char *buf = nullptr;
        size_t sz = 0;
        if (_dupenv_s(&buf, &sz, "windir") == 0 && buf != nullptr) {
            windir = buf;
            free(buf);
        }

        return windir + "\\System32\\drivers\\etc\\hosts";
    }

    //--------------------------------
    Host::Host()
    {
        // m_bIsEnable = true;
    }

    Host::Host(QString strName, QString strAddress)
    {
        setName(strName);
        setAddress(strAddress);
        // m_bIsEnable = true;
    }

    void Host::setAddress(QString strAddress) { strAddress = strAddress.trimmed(); }

    QString Host::address() { return strAddress; }

    void Host::setName(QString strName) { strName = strName.trimmed(); }

    QString Host::name() { return strName; }

    // void Host::setEnable(bool bEnable)
    //{
    //    m_bIsEnable = bEnable;
    //}

    // bool Host::isEnable()
    //{
    //    return m_bIsEnable;
    //}

    bool Host::operator==(const Host &host) const { return strName == host.strName && strAddress == host.strAddress; }
}
