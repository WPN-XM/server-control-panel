#ifndef PACKAGE_H
#define PACKAGE_H

#include <QObject>

class Package : public QObject
{
    Q_OBJECT
public:
    explicit Package(QObject *parent = 0);

    QList<QString> listUpgrades();

    void upgradeAll();
    void upgrade(QString packageName);
    void install(QString packageName);
    void version(QString packageName);

signals:

public slots:
};

#endif // PACKAGE_H
