#include "settings.h"

namespace Settings
{
    const QString appSettingsFileName("wpn-xm.ini");

    SettingsManager::SettingsManager(QObject *parent) : QObject(parent) {}

    QString SettingsManager::file() const
    {
        QString file = QCoreApplication::applicationDirPath() + '/' + appSettingsFileName;
        return QDir::toNativeSeparators(file);
    }

    QVariant SettingsManager::get(const QString &key, const QVariant &defaultValue) const
    {
        QSettings settings(file(), QSettings::IniFormat);
        return settings.value(key, defaultValue);
    }

    void SettingsManager::set(const QString &key, const QVariant &value)
    {
        QSettings settings(file(), QSettings::IniFormat);
        settings.setValue(key, value);
    }

    QStringList SettingsManager::getKeys(const QString &groupPrefix) const
    {
        QSettings settings(file(), QSettings::IniFormat);
        settings.beginGroup(groupPrefix);
        return settings.allKeys();
    }

    void SettingsManager::sync() {
        settings->sync();
    }

    SettingsManager::~SettingsManager() {
        delete settings;
    }
}
