#include "json.h"

namespace File
{
    QJsonDocument JSON::load(QString fileName)
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        file.close();
        return document;
    }

    void JSON::save(QJsonDocument document, QString fileName)
    {
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(document.toJson());
        file.close();
    }

    QString Text::load(QString fileName)
    {
        // qDebug() << "[TEXT] Reading file [" << fileName << "]";

        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString text = QString(file.readAll());
        file.close();
        return text;
    }

    void Text::save(QString text, QString fileName)
    {
        // qDebug() << "[TEXT] Writing file [" << fileName << "]";

        QFile file(fileName);
        file.open(QFile::WriteOnly);
        file.write(text.toLatin1());
        file.close();
    }
}
