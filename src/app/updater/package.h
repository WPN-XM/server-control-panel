#ifndef PACKAGE_H
#define PACKAGE_H

#include <QObject>

class Package : public QObject
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
