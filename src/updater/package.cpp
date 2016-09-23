#include "package.h"

Package::Package(QObject *parent) : QObject(parent)
{

}

/**
 * Get a list of packages that need to be upgraded.
 */
QList<QString> Package::listUpgrades()
{
    QList<QString> list; 
    list << "foo";
    return list;
}

/**
 * Upgrades all packages
 */
void Package::upgradeAll()
{

}

/**
 * Upgrade one package
 */
void Package::upgrade(QString packageName)
{
    Q_UNUSED(packageName);
}

/**
 * Get current version of a package
 */
void Package::version(QString packageName)
{
    Q_UNUSED(packageName);
}

/**
 * Install or upgrade package
 */
void Package::install(QString packageName)
{
    Q_UNUSED(packageName);
}



