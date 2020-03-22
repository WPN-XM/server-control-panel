#include "csv.h"

#include <QFile>
#include <QRegExp>
#include <QTextCodec>
#include <QTextStream>

namespace File
{
    QList<QStringList> CSV::parse(const QString &string)
    {
        enum State
        {
            Normal,
            Quote
        } state = Normal;
        QList<QStringList> data;
        QStringList line;
        QString value;

        const int stringSize = string.size();

        for (int i = 0; i < stringSize; i++) {
            QChar current = string.at(i);

            // Normal
            if (state == Normal) {
                // newline
                if (current == '\n') {
                    // add value
                    line.append(value);
                    value.clear();
                    // add line
                    data.append(line);
                    line.clear();
                }
                // comma
                else if (current == ',') {
                    // add line
                    line.append(value);
                    value.clear();
                }
                // double quote
                else if (current == '"') {
                    state = Quote;
                }
                // character
                else {
                    value += current;
                }
            }
            // Quote
            else if (state == Quote) {
                // double quote
                if (current == '"') {
                    int index  = (i + 1 < string.size()) ? i + 1 : string.size();
                    QChar next = string.at(index);
                    if (next == '"') {
                        value += '"';
                        i++;
                    } else {
                        state = Normal;
                    }
                }
                // other
                else {
                    value += current;
                }
            }
        }

        return data;
    }

    QString CSV::initString(const QString &string)
    {
        QString result = string;
        result.replace("\r\n", "\n");
        result.replace("\r", "");
        if (result.at(result.size() - 1) != '\n') {
            result += '\n';
        }
        return result;
    }

    QList<QStringList> CSV::parseFromString(const QString &string) { return parse(initString(string)); }

    QList<QStringList> CSV::parseFromFile(const QString &filename, const QString &codec)
    {
        QString string;
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            if (!codec.isEmpty()) {
                in.setCodec(QTextCodec::codecForName(codec.toLatin1()));
            }
            string = in.readAll();
            file.close();
        }
        return parse(initString(string));
    }

    bool CSV::write(const QList<QStringList> &data, const QString &filename, const QString &codec)
    {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        QTextStream out(&file);
        if (!codec.isEmpty()) {
            out.setCodec(codec.toLatin1());
        }

        for (const QStringList &line : data) {
            QStringList output;
            for (QString value : line) {
                if (value.contains(QRegExp(",|\r\n"))) {
                    output << ("\"" + value + "\"");
                } else if (value.contains("\"")) {
                    output << value.replace("\"", "”");
                } else {
                    output << value;
                }
            }
            out << output.join(",") << "\r\n";
        }

        file.close();

        return true;
    }
} // namespace File