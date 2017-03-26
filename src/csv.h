#ifndef CSV_H
#define CSV_H

#include <QStringList>

namespace File
{
    class CSV
    {
    public:
        static QList<QStringList> parseFromString(const QString &string);
        static QList<QStringList> parseFromFile(const QString &filename,
                                                const QString &codec = QString());
        static bool write(const QList<QStringList> data, const QString &filename, const QString &codec = QString());

    private:
        static QList<QStringList> parse(const QString &string);
        static QString initString(const QString &string);
    };
}

#endif // CSV_H
