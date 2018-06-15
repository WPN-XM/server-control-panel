#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

namespace Settings
{
    /// Implements the application settings repository.
    /*!
    This class stores the application settings.
*/
    class SettingsManager : public QObject
    {
        Q_OBJECT

    public:
        SettingsManager(QObject *parent = 0);

        QVariant get(const QString &key, const QVariant &defaultValue = QVariant()) const;
        QStringList getKeys(const QString &groupPrefix) const;
        QString file() const;
        void set(const QString &key, const QVariant &value);
    };
}

#endif // SETTINGS_H
