#include "downloadmanager.h"

#include <QCoreApplication>
#include <QSslError>

namespace Downloader
{
    DownloadManager::DownloadManager()
        : queueMode(Parallel), FilesDownloadedCounter(0), FilesToDownloadCounter(0)
    {
        connect(&nam, SIGNAL(finished(QNetworkReply *)), this,
                SLOT(finished(QNetworkReply *)));

#ifndef QT_NO_SSL
        connect(&nam, SIGNAL(sslErrors(QNetworkReply *, QList<QSslError>)), this,
                SLOT(sslErrors(QNetworkReply *, QList<QSslError>)));
#endif
    }

    DownloadManager::~DownloadManager() {}

    void DownloadManager::get(QNetworkRequest &request, QString dlFolder, DownloadItem::DownloadMode dlMode)
    {
        setDownloadFolder(dlFolder);
        setDownloadMode(dlMode);
        get(request);
    }

    void DownloadManager::get(QNetworkRequest &request)
    {
        qDebug() << "DownloadManager::get()"
                 << "Download enqueued.";

        // set Request Headers
        QString appVersion(qApp->applicationName() + qApp->applicationVersion());
        const static QByteArray userAgent(
            QByteArray(appVersion.toStdString().c_str()));
        request.setRawHeader("User-Agent", userAgent);
        request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

        // enqueue the download
        DownloadItem *dl = new DownloadItem(request, nam);

        dl->setDownloadFolder(downloadFolder);
        dl->setDownloadMode(downloadMode);

        transfers.append(dl);
        FilesToDownloadCounter = transfers.count();

        connect(dl, SIGNAL(transferFinished(TransferItem *)),
                SLOT(downloadFinished(TransferItem *)));
    }

    void DownloadManager::finished(QNetworkReply *)
    {
        qDebug() << "DownloadManager::finished()";
    }

    void DownloadManager::downloadFinished(Downloader::TransferItem *item)
    {
        qDebug() << "Download finished " << item->reply->url();
        qDebug() << " with HTTP Status: "
                 << item->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute)
                        .toInt();
        if (item->reply->error() != QNetworkReply::NoError) {
            qDebug() << "and error: " << item->reply->error()
                     << item->reply->errorString();
        }
        transfers.removeOne(item);
        FilesToDownloadCounter = transfers.count();
        ++FilesDownloadedCounter;
        item->deleteLater();
        checkForAllDone();
    }

    void DownloadManager::checkForAllDone()
    {
        if (transfers.isEmpty()) {
            qDebug() << "[Downloader] Download queue is now empty! All Done.";
            FilesDownloadedCounter = FilesToDownloadCounter = 0;
            return;
        }

        foreach (TransferItem *item, transfers) {
            if (!item->reply) {
                item->startGetRequest();
                // by default multiple downloads are processed in parallel.
                // but in serial mode, only one transfer starts at a time.
                if (queueMode == Serial) {
                    break;
                }
            }
        }
    }

#ifndef QT_NO_SSL
    void DownloadManager::sslErrors(QNetworkReply *,
                                    const QList<QSslError> &errors)
    {
        qDebug() << "sslErrors";
        foreach (const QSslError &error, errors) {
            qDebug() << error.errorString();
            qDebug() << error.certificate().toPem();
        }
    }
#endif

    TransferItem *DownloadManager::findTransfer(QUrl url)
    {
        foreach (TransferItem *item, transfers) {
            if (item->request.url() == url) {
                return item;
            }
        }
        return 0;
    }

    TransferItem *DownloadManager::findTransfer(QNetworkReply *reply)
    {
        foreach (TransferItem *item, transfers) {
            if (item->reply == reply) {
                return item;
            }
        }
        return 0;
    }

    void DownloadManager::setQueueMode(QueueMode mode) { queueMode = mode; }

    void DownloadManager::setDownloadFolder(QString folder)
    {
        downloadFolder = folder;
    }

    void DownloadManager::setDownloadMode(DownloadItem::DownloadMode mode)
    {
        downloadMode = mode;
    }
}
