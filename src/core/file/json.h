#ifndef JSON_H
#define JSON_H

#include "common.h"

#include <QFile>
#include <QJsonDocument>

namespace File
{
    class APP_CORE_EXPORT JSON
    {
    public:
        static void save(const QJsonDocument &document, const QString &fileName);
        static QJsonDocument load(const QString &fileName);
    };

    class APP_CORE_EXPORT Text
    {
    public:
        static void save(const QString &text, const QString &fileName);
        static QString load(const QString &fileName);
    };
} // namespace File
#endif // JSON_H
