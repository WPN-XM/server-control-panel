#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QElapsedTimer>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace Downloader
{
    class TransferItem : public QObject
    {
        Q_OBJECT
    public:
        TransferItem(const QNetworkRequest &r, QNetworkAccessManager &n);
        void startGetRequest();
    signals:
        void downloadProgress(QMap<QString, QVariant>);
        void transferFinished(Downloader::TransferItem *self);
    public slots:
        void updateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    private slots:
        void finished();

    public:
        QNetworkRequest request;
        QNetworkReply *reply;
        QNetworkAccessManager &nam;
        QFile *inputFile;
        QFile *outputFile;
        QList<QUrl> redirects;
        QElapsedTimer timer;
        QMap<QString, QVariant> progress;

    private:
        QString getSizeHumanReadable(qint64 bytes);
    };

    class DownloadItem : public TransferItem
    {
        Q_OBJECT
    public:
        DownloadItem(const QNetworkRequest &r, QNetworkAccessManager &nam);
        ~DownloadItem() override;
        void setDownloadFolder(const QString &folder);
        enum DownloadMode
        {
            SkipIfExists,
            Overwrite,
            Enumerate
        } downloadMode = SkipIfExists;
        void setDownloadMode(DownloadItem::DownloadMode mode);

    private:
        QString downloadFolder;
        bool downloadSkipped;
    signals:
        void transferFinished(Downloader::TransferItem *self);
    private slots:
        void readyRead();
        void finished();
    };

    class DownloadManager : public QObject
    {
        Q_OBJECT
    public:
        DownloadManager();
        ~DownloadManager() override;
        void get(QNetworkRequest &request, QString dlFolder, DownloadItem::DownloadMode dlMode);
        void get(QNetworkRequest &request);
        enum QueueMode
        {
            Parallel,
            Serial
        };
        void setQueueMode(QueueMode mode);
        TransferItem *findTransfer(const QUrl &url);

        void setDownloadFolder(QString downloadFolder);
        void setDownloadMode(DownloadItem::DownloadMode mode);

    public slots:
        void checkForAllDone();

    private slots:
        void finished(QNetworkReply *reply);
#ifndef QT_NO_SSL
        void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
#endif
        void downloadFinished(Downloader::TransferItem *item);

    private:
        TransferItem *findTransfer(QNetworkReply *reply);

        QNetworkAccessManager nam;
        QList<TransferItem *> transfers;
        QueueMode queueMode{Parallel};
        QString downloadFolder;
        DownloadItem::DownloadMode downloadMode = DownloadItem::DownloadMode::SkipIfExists;

    public:
        int FilesDownloadedCounter = 0;
        int FilesToDownloadCounter = 0;
    };
} // namespace Downloader

#endif // DOWNLOADMANAGER_H
