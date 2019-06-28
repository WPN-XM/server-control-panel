#include "json.h"

namespace File
{
    QJsonDocument JSON::load(const QString &fileName)
    {
        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        file.close();
        return document;
    }

    void JSON::save(const QJsonDocument &document, const QString &fileName)
    {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(document.toJson());
            file.close();
        } else {
            // qDebug() << "Could not write JSON file:" << fileName;
        }
    }

    QString Text::load(const QString &fileName)
    {
        // qDebug() << "[TEXT] Reading file [" << fileName << "]";

        QFile file(fileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString text = QString(file.readAll());
        file.close();
        return text;
    }

    void Text::save(const QString &text, const QString &fileName)
    {
        // qDebug() << "[TEXT] Writing file [" << fileName << "]";

        QFile file(fileName);
        file.open(QFile::WriteOnly);
        file.write(text.toLatin1());
        file.close();
    }
} // namespace File
