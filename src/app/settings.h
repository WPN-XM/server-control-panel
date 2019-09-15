#ifndef SETTINGS_H
#define SETTINGS_H

#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QPointer>

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

        bool getBool(const QString &key, const QVariant &defaultValue = QVariant()) const;
        bool getBool(const QString &key, const bool &defaultValue) const;

        int getInt(const QString &key, const QVariant &defaultValue = QVariant()) const;
        int getInt(const QString &key, const int &defaultValue) const;

        QString getString(const QString &key, const QVariant &defaultValue = QVariant()) const;
        QString getString(const QString &key, const QString &defaultValue) const;

        QPointer<QSettings> getQSettings();

    private:
        QSettings *settings;
    };
} // namespace Settings

#endif // SETTINGS_H
