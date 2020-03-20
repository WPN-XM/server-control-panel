#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QObject>

#include <QNetworkAddressEntry>
#include <QNetworkInterface>

class NetworkUtils
{
public:
    NetworkUtils();

    static QList<QHostAddress> getLocalHostIPs();

    QString getIpAddress();
    void ping(const QString &hostname);
    void traceroute(const QString &hostname);
};

#endif // NETWORKUTILS_H
