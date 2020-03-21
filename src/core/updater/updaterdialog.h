#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

#include "common.h"

#include <QDialog>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox> /* Remove+Debug: Download clicked */
#include <QNetworkProxy>
#include <QProcess>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>

#include "file/json.h"
#include "registry/registrymanager.h"

#include "updater/downloadmanager.h"

#include "actioncolumnitemdelegate.h"
#include "softwarecolumnitemdelegate.h"

namespace Updater
{
    namespace Ui
    {
        class UpdaterDialog;
    }

    class APP_CORE_EXPORT UpdaterDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit UpdaterDialog(QWidget *parent = nullptr);
        ~UpdaterDialog() override;
        void initModel(QJsonObject json);
        void initView();
        enum Columns
        {
            // Column   0             1             2             3            4 5
            SoftwareComponent,
            WebsiteURL,
            YourVersion,
            LatestVersion,
            DownloadURL,
            Action
        };
        Ui::UpdaterDialog *ui;

    protected:
        QStandardItemModel *model{};
        QSortFilterProxyModel *sortFilterProxyModel{};
        SoftwareRegistry::Manager *softwareRegistry{};
        Downloader::DownloadManager downloadManager;

    private:
        QUrl getDownloadUrl(const QModelIndex &index);
        bool validateURL(const QUrl &url);
        Updater::SoftwareColumnItemDelegate *softwareDelegate{};
        Updater::ActionColumnItemDelegate *actionDelegate{};
    signals:
        void clicked(const QString &websiteLink);
    public slots:
        void doDownload(const QModelIndex &index);
        void doInstall(const QModelIndex &index);
        void downloadsFinished();
    private slots:
        void on_searchLineEdit_textChanged(const QString &arg1);
    };

    class APP_CORE_EXPORT ProgressBarUpdater : public QObject
    {
        Q_OBJECT
    public:
        explicit ProgressBarUpdater(UpdaterDialog *parent = nullptr, int indexRow = 0);
    public slots:
        void updateProgress(const QMap<QString, QVariant> &progress);
        void downloadFinished(Downloader::TransferItem *transfer);

    private:
        QMap<QString, QVariant> progress;

    protected:
        QModelIndex index;
        QAbstractItemModel *model;
        const int currentRow;
    };
} // namespace Updater

#endif // UPDATERDIALOG_H
