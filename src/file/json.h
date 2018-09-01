#ifndef JSON_H
#define JSON_H

#include <QFile>
#include <QJsonDocument>

namespace File
{
    class JSON
    {
    public:
        static void save(const QJsonDocument &document, const QString &fileName);
        static QJsonDocument load(const QString &fileName);
    };

    class Text
    {
    public:
        static void save(const QString &text, const QString &fileName);
        static QString load(const QString &fileName);
    };
} // namespace File
#endif // JSON_H
