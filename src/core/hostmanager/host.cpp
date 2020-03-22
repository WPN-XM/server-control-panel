#include "host.h"
#include <QDebug>

namespace HostsFileManager
{
    // path: "%SystemRoot%\\System32\\drivers\\etc\\hosts"
    QList<Host *> Host::GetHosts()
    {
        QList<Host *> listReturn;

        QFile hostFile(getHostFile());
        if (hostFile.open(QFile::ReadOnly)) {
            QTextStream hostStream(&hostFile);
            QString strLine;
            while (!(strLine = hostStream.readLine()).isNull()) {
                strLine          = strLine.trimmed();
                QStringList list = strLine.split(QRegExp("[ \t]"), QString::SkipEmptyParts);

                // skip comment line
                if (list.empty() || list.startsWith("#"))
                    continue;

                Host *host = new Host(list[1], list[0]);
                listReturn << host;
            }
            qDebug() << strLine;
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
                    tempStream << host->ipaddress << "       " << host->hostname << "\r\n";
                }
            }

            // Copy remaining hosts
            tempStream << "\r\n";
            while (!listHosts.isEmpty()) {
                Host *host = listHosts.takeFirst();
                tempStream << host->ipaddress << "       " << host->hostname << "\r\n";
            }

            // Cleanup
            hostFile.close();
            tempFile.close();
        }

        // Copy content of tempfile to host file
        QString strHostFile = QDir::toNativeSeparators(hostFile.fileName());
        QString strTempFile = QDir::toNativeSeparators(tempFile.fileName());

        QString strArguments = "/c copy /y \"" + strTempFile + "\" \"" + strHostFile + "\"";
        LPCTSTR wcArguments  = strArguments.toStdString().c_str();

        SHELLEXECUTEINFO shExecInfo;
        shExecInfo.cbSize       = sizeof(SHELLEXECUTEINFO);
        shExecInfo.fMask        = 0;
        shExecInfo.hwnd         = nullptr;
        shExecInfo.lpVerb       = "runas";
        shExecInfo.lpFile       = "cmd.exe";
        shExecInfo.lpParameters = wcArguments;
        shExecInfo.lpDirectory  = nullptr;
        shExecInfo.nShow        = SW_MAXIMIZE;
        shExecInfo.hInstApp     = nullptr;

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

        return windir + R"(\System32\drivers\etc\hosts)";
    }

    //--------------------------------
    Host::Host() { enabled = true; }

    Host::Host(const QString &name, const QString &address)
        : hostname(name.trimmed()), ipaddress(address.trimmed()), enabled(true)
    {
    }

    void Host::setEnabled(bool enable) { enabled = enable; }
    bool Host::isEnabled() { return enabled; }

    void Host::setHostname(const QString &host) { hostname = host; }
    QString Host::getHostname() { return hostname; }

    void Host::setIp(const QString &ip) { ipaddress = ip; }
    QString Host::getIp() { return ipaddress; }

    bool Host::operator==(const Host &host) const { return hostname == host.hostname && ipaddress == host.ipaddress; }
} // namespace HostsFileManager