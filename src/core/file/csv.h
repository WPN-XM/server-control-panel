#ifndef CSV_H
#define CSV_H

#include "common.h"

#include <QStringList>

namespace File
{
    class APP_CORE_EXPORT CSV
    {
    public:
        static QList<QStringList> parseFromString(const QString &string);
        static QList<QStringList> parseFromFile(const QString &filename, const QString &codec = QString());
        static bool write(const QList<QStringList> &data, const QString &filename, const QString &codec = QString());

    private:
        static QList<QStringList> parse(const QString &string);
        static QString initString(const QString &string);
    };
} // namespace File

#endif // CSV_H
