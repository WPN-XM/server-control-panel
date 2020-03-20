#include "filehandling.h"

namespace File
{
    bool truncate(const QString &file)
    {
        QFile f(file);

        if (f.exists()) {
            f.open(QFile::WriteOnly | QFile::Truncate);
            f.close();
            return true;
        }
        f.close();
        return false;
    }

    bool move(const QString &source, const QString &target)
    {
        if (source.isEmpty() || target.isEmpty()) {
            return false;
        }

        if (source == target) {
            return false;
        }

        bool moveSucceed = copyFile(source, target);

        if (moveSucceed) {
            moveSucceed &= QFile::remove(source);
        }

        return moveSucceed;
    }

    bool copyData(QIODevice &src, QIODevice &dest)
    {
        bool success = false;

        if ((src.openMode() & QIODevice::ReadOnly) == 0) {
            return false;
        }

        if ((dest.openMode() & QIODevice::WriteOnly) == 0) {
            return false;
        }

        QByteArray bb(65536, ' ');

        if (bb.size() > 0) { // Check for memory allocation failure
            qint64 byteswritten;
            qint64 bytesread = src.read(bb.data(), bb.size());
            success          = (bytesread > 0);
            while (bytesread > 0) {
                byteswritten = dest.write(bb.data(), bytesread);
                success &= (bytesread == byteswritten);
                bytesread = src.read(bb.data(), bb.size());
            }
        }

        return success;
    }

    bool copyFile(const QString &source, const QString &target)
    {
        bool success = true;

        if ((source.isEmpty()) || (target.isEmpty())) {
            return false;
        }

        if (source == target) {
            return false;
        }

        QFile s(source);

        if (!s.exists()) {
            return false;
        }

        QFile t(target);

        if (s.open(QIODevice::ReadOnly)) {
            if (t.open(QIODevice::WriteOnly)) {
                success = copyData(s, t);
                success &= (s.error() == QFile::NoError && t.error() == QFile::NoError);
                t.close();
            }
            s.close();
        }

        return success;
    }
}
