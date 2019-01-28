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
    void ping(QString hostname);
    void traceroute(QString hostname);
};

#endif // NETWORKUTILS_H
