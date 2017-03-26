#ifndef JSON_H
#define JSON_H

#include <QFile>
#include <QJsonDocument>

namespace File
{
    class JSON
    {
    public:
        static void save(QJsonDocument document, QString fileName);
        static QJsonDocument load(QString fileName);
    };

    class Text
    {
    public:
        static void save(QString text, QString fileName);
        static QString load(QString fileName);
    };
}
#endif // JSON_H
