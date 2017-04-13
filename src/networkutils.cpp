#include "networkutils.h"

NetworkUtils::NetworkUtils() {}

/**
 * Note: this method depends on "QT += network" in the .pro file
 */
QList<QHostAddress> NetworkUtils::getLocalHostIPs()
{
    QList<QHostAddress> ips;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface iface, interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) && iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            foreach (const QNetworkAddressEntry &entry, iface.addressEntries()) {
                QHostAddress ip = entry.ip();
                // ignore local host IPs
                if (ip == QHostAddress::LocalHost || ip == QHostAddress::LocalHostIPv6) {
                    continue;
                }
                if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
                    ips.append(ip);
                }
            }
        }
    }

    return ips;
}

/**
 * Get IP address of your system
 */
QString NetworkUtils::getIpAddress() { return QString(); }

/**
 * Ping a host
 */
void NetworkUtils::ping(QString hostname) { Q_UNUSED(hostname); }

/**
 * TraceRoute a host
 */
void NetworkUtils::traceroute(QString hostname) { Q_UNUSED(hostname); }
