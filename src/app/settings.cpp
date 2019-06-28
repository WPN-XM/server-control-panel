#include "settings.h"

namespace Settings
{
    const QString appSettingsFileName("wpn-xm.ini");

    SettingsManager::SettingsManager(QObject *parent) : QObject(parent)
    {
        settings = new QSettings(file(), QSettings::IniFormat);
    }

    QString SettingsManager::file() const
    {
        QString file = QCoreApplication::applicationDirPath() + '/' + appSettingsFileName;
        return QDir::toNativeSeparators(file);
    }

    QVariant SettingsManager::get(const QString &key, const QVariant &defaultValue) const
    {
        return settings->value(key, defaultValue);
    }

    void SettingsManager::set(const QString &key, const QVariant &value) { settings->setValue(key, value); }

    QStringList SettingsManager::getKeys(const QString &groupPrefix) const
    {
        settings->beginGroup(groupPrefix);
        QStringList list = settings->allKeys();
        settings->endGroup();
        return list;
    }

    bool SettingsManager::getBool(const QString &key, const QVariant &defaultValue) const
    {
        return settings->value(key, defaultValue).toBool();
    }

    bool SettingsManager::getBool(const QString &key, const bool &defaultValue) const
    {
        return settings->value(key, QVariant(defaultValue)).toBool();
    }

    int SettingsManager::getInt(const QString &key, const QVariant &defaultValue) const
    {
        return settings->value(key, defaultValue).toInt();
    }

    int SettingsManager::getInt(const QString &key, const int &defaultValue) const
    {
        return settings->value(key, QVariant(defaultValue)).toInt();
    }

    QString SettingsManager::getString(const QString &key, const QVariant &defaultValue) const
    {
        return settings->value(key, defaultValue).toString();
    }

    QString SettingsManager::getString(const QString &key, const QString &defaultValue) const
    {
        return settings->value(key, QVariant(defaultValue)).toString();
    }

    QPointer<QSettings> SettingsManager::getQSettings() { return settings; }

} // namespace Settings
