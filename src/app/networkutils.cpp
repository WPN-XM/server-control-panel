#include "networkutils.h"

NetworkUtils::NetworkUtils() = default;

/**
 * Note: this method depends on "QT += network" in the .pro file
 */
QList<QHostAddress> NetworkUtils::getLocalHostIPs()
{
    QList<QHostAddress> ips;
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface &iface : interfaces) {

        // interface must be up and running and not a loopback device
        if (iface.flags().testFlag(QNetworkInterface::IsUp) && iface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

            for (const QNetworkAddressEntry &entry : iface.addressEntries()) {

                QHostAddress ip = entry.ip();

                // ignore local host IPs
                if (ip == QHostAddress::LocalHost || ip == QHostAddress::LocalHostIPv6) {
                    continue;
                }

                // add IPv4 IPs
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
