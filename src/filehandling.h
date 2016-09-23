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
    bool copyFile(const QString &source, const QString &target);
    bool copyFileToStream(const QString &source, QDataStream &target);
    bool moveFile(const QString &source, const QString &target);
}

#endif // FILEHANDLING_H
