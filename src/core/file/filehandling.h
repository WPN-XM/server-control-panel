#ifndef FILEHANDLING_H
#define FILEHANDLING_H

//#include <utime.h>
#include "common.h"

#include <QByteArray>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QTemporaryFile>

namespace File
{
    bool APP_CORE_EXPORT truncate(const QString &file);
    bool APP_CORE_EXPORT move(const QString &source, const QString &target);
    bool APP_CORE_EXPORT copyFile(const QString &source, const QString &target);
    bool APP_CORE_EXPORT copyData(const QString &source, QDataStream &target);
} // namespace File

#endif // FILEHANDLING_H
