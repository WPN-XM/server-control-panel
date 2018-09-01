#ifndef SERVICES_H
#define SERVICES_H

#include <QObject>

class Services
{
public:
    Services();

    void available(QString serviceName);
    void install(QString serviceName);
    void remove(QString serviceName);
    void reinstall(const QString &serviceName);
    void status(QString serviceName);
};

#endif // SERVICES_H
