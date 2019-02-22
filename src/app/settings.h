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
        SettingsManager(QObject *parent = nullptr);

        QVariant get(const QString &key, const QVariant &defaultValue = QVariant()) const;
        QStringList getKeys(const QString &groupPrefix) const;
        QString file() const;
        void set(const QString &key, const QVariant &value);

        QSettings getSettingsObject();
    };
} // namespace Settings

#endif // SETTINGS_H
