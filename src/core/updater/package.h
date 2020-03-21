#ifndef PACKAGE_H
#define PACKAGE_H

#include "common.h"

#include <QObject>

class APP_CORE_EXPORT Package : public QObject
{
    Q_OBJECT
public:
    explicit Package(QObject *parent = nullptr);

    QList<QString> listUpgrades();

    void upgradeAll();
    void upgrade(const QString &packageName);
    void install(const QString &packageName);
    void version(const QString &packageName);

signals:

public slots:
};

#endif // PACKAGE_H
