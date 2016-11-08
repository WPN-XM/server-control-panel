#ifndef FILEHANDLING_H
#define FILEHANDLING_H

//#include <utime.h>

#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QProcess>
#include <QTemporaryFile>

namespace File
{
    bool truncate(const QString &file);
    bool move(const QString &source, const QString &target);
    bool copyFile(const QString &source, const QString &target);
    bool copyData(const QString &source, QDataStream &target);
}

#endif // FILEHANDLING_H
