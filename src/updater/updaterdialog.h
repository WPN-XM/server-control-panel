#ifndef UPDATERDIALOG_H
#define UPDATERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProcess>
#include <QMessageBox> /* Remove+Debug: Download clicked */
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDir>
#include <QNetworkProxy>

#include "src/json.h"
#include "src/registry/registrymanager.h"

#include "src/updater/downloadmanager.h"

#include "actioncolumnitemdelegate.h"
#include "softwarecolumnitemdelegate.h"

namespace Updater
{
    namespace Ui {
        class UpdaterDialog;
    }

    class UpdaterDialog : public QDialog
    {
        Q_OBJECT

        public:
            explicit UpdaterDialog(QWidget *parent = 0);
            ~UpdaterDialog();
            void initModel(QJsonObject json);
            void initView();
            enum Columns {
             // Column   0             1             2             3            4         5
                SoftwareComponent, WebsiteURL, YourVersion,  LatestVersion, DownloadURL,  Action
            };
            Ui::UpdaterDialog *ui;
        protected:
            QStandardItemModel           *model;
            QSortFilterProxyModel        *sortFilterProxyModel;
            SoftwareRegistry::Manager    *softwareRegistry;
            Downloader::DownloadManager  downloadManager;
        private:
            QUrl getDownloadUrl(const QModelIndex &index);
            bool validateURL(QUrl url);
            Updater::SoftwareColumnItemDelegate *softwareDelegate;
            Updater::ActionColumnItemDelegate   *actionDelegate;
        signals:
            void clicked(const QString &websiteLink);
        public slots:
            void doDownload(const QModelIndex &index);
            void doInstall(const QModelIndex &index);
            void downloadsFinished();
        private slots:
            void on_searchLineEdit_textChanged(const QString &arg1);
    };

    class ProgressBarUpdater : public QObject
    {
        Q_OBJECT
        public:
            explicit ProgressBarUpdater(UpdaterDialog *parent = 0, int currentRow = 0);
        public slots:
            void updateProgress(QMap<QString, QVariant> progress);
            void downloadFinished(Downloader::TransferItem *transfer);
        private:
            QMap<QString, QVariant>  progress;
        protected:
            QModelIndex        index;
            QAbstractItemModel *model;
            const int          currentRow;
    };
}

#endif // UPDATERDIALOG_H
