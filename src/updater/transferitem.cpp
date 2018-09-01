#include "downloadmanager.h"

#include <QDir>

namespace Downloader
{
    TransferItem::TransferItem(const QNetworkRequest &r, QNetworkAccessManager &n)
        : request(r), reply(nullptr), nam(n), inputFile(nullptr), outputFile(nullptr)
    {
        // qDebug() << "New TransferItem instantiated";
    }

    void TransferItem::startGetRequest()
    {
        qDebug() << "TransferItem::startRequest()";

        reply = nam.get(request);

        connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(updateDownloadProgress(qint64, qint64)));
        connect(reply, SIGNAL(finished()), this, SLOT(finished()));

        timer.start();
    }

    void TransferItem::finished() { emit transferFinished(this); }

    // SLOT
    void TransferItem::updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
    {
        progress["bytesReceived"] = QString::number(bytesReceived);
        progress["bytesTotal"]    = QString::number(bytesTotal);
        progress["size"]          = getSizeHumanReadable(outputFile->size());
        progress["speed"] = QString::number((double)outputFile->size() / timer.elapsed(), 'f', 0).append(" KB/s");
        // progress["time"]          =
        // QString::number((double)timer.elapsed()/1000,'f',2).append("s");
        // progress["eta"]           =
        progress["percentage"] =
            (bytesTotal > 0) ? QString::number(bytesReceived * 100 / bytesTotal).append("%") : "0 %";

        emit downloadProgress(progress);
    }

    QString TransferItem::getSizeHumanReadable(qint64 bytes)
    {
        float num = bytes;
        QStringList list;
        list << "KB"
             << "MB"
             << "GB"
             << "TB";

        QStringListIterator i(list);
        QString unit("bytes");

        while (num >= 1024.0 && i.hasNext()) {
            unit = i.next();
            num /= 1024.0;
        }
        return QString::fromLatin1("%1 %2").arg(num, 3, 'f', 1).arg(unit);
    }

    DownloadItem::DownloadItem(const QNetworkRequest &r, QNetworkAccessManager &manager)
        : TransferItem(r, manager), downloadFolder()
    {
    }

    DownloadItem::~DownloadItem() {}

    void DownloadItem::readyRead()
    {
        // qDebug() << "DownloadItem::readyRead() ...";

        if (!outputFile) {
            outputFile = new QFile(this);
        }

        if (!outputFile->isOpen()) {
            // qDebug() << reply->header(QNetworkRequest::ContentTypeHeader) <<
            // reply->header(QNetworkRequest::ContentLengthHeader);

            // get filename from URL
            QString fileName = reply->url().path();
            fileName         = fileName.mid(fileName.lastIndexOf('/') + 1);
            if (fileName.isEmpty()) {
                fileName = QLatin1String("index.html"); // fallback filename
            }
            qDebug() << "[DownloadItem::readyRead] Filename from URL:" << fileName;

            // TODO move to settings function? (folder name should be fixed/ensured,
            // when set and get to/from settings)
            if (!downloadFolder.endsWith(QDir::separator())) {
                downloadFolder.append(QDir::separator());
            }

            QString downloadFilePath = QDir::toNativeSeparators(downloadFolder.append(fileName));

            outputFile->setFileName(downloadFilePath);

            qDebug() << "[DownloadItem::readyRead] Download FullFilePath:" << downloadFilePath;
            qDebug() << "[DownloadItem::readyRead] DownloadMode" << downloadMode;

            if (downloadMode == DownloadMode::SkipIfExists) {
                qDebug() << "[DownloadItem::readyRead] DownloadMode::SkipIfExists - File "
                            "exists:"
                         << outputFile->exists();
                if (outputFile->exists()) {
                    downloadSkipped = true;
                    reply->abort();
                    return;
                }
                outputFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
            } else if (downloadMode == DownloadMode::Overwrite) {
                // if the file already exists, overwrite (delete and write)
                if (outputFile->exists()) {
                    outputFile->remove();
                }
                outputFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
            } else if (downloadMode == DownloadMode::Enumerate) {
                // if the file already exists, append a number, e.g. "file.zip.1"
                for (int i = 1; i < 1000; i++) {
                    if (!outputFile->exists() && outputFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                        break;
                    }
                    QString enumedFileName   = QString(QLatin1String("%1.%2")).arg(fileName).arg(i);
                    QString downloadFilePath = QDir::toNativeSeparators(downloadFolder.append(enumedFileName));
                    outputFile->setFileName(downloadFilePath);
                }
            }

            // if file still not open, abort
            if (!outputFile->isOpen()) {
                qDebug() << "[DownloadItem::readyRead] couldn't open output file" << outputFile->fileName();
                reply->abort();
                return;
            }

            qDebug() << reply->url() << " -> " << outputFile->fileName();
        }

        // write reply to file
        outputFile->write(reply->readAll());
    }

    void DownloadItem::finished()
    {
        qDebug() << "DownloadItem::finished()";

        // handle Redirection
        if (reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isValid()) {

            if (downloadSkipped) {
                timer.invalidate();
                emit transferFinished(this);
                return;
            }

            QUrl url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            url      = reply->url().resolved(url);
            qDebug() << "[DownloadItem] Finished, but " << reply->url() << "redirected to " << url;
            if (redirects.contains(url)) {
                qDebug() << "[DownloadItem] Redirect Loop Detected";
            } else if (redirects.count() > 10) {
                qDebug() << "[DownloadItem] Too Many Redirects";
            } else {
                // follow redirect
                if (outputFile && outputFile->isOpen()) {
                    if (!outputFile->seek(0) || !outputFile->resize(0)) {
                        outputFile->close();
                        outputFile->remove();
                    }
                }
                reply->deleteLater();
                reply = nam.get(QNetworkRequest(url));
                reply->setParent(this);
                connect(reply, SIGNAL(readyRead()), this, SLOT(readyRead()));
                connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this,
                        SLOT(updateDownloadProgress(qint64, qint64)));
                connect(reply, SIGNAL(finished()), this, SLOT(finished()));
                timer.restart();
                redirects.append(url);
                return;
            }
        }

        // write reply to file
        if (outputFile && outputFile->isOpen()) {
            outputFile->write(reply->readAll());
            outputFile->close();
        }

        timer.invalidate();

        emit transferFinished(this);
    }

    void DownloadItem::setDownloadFolder(QString folder) { downloadFolder = folder; }

    void DownloadItem::setDownloadMode(DownloadItem::DownloadMode mode) { downloadMode = mode; }
} // namespace Downloader
