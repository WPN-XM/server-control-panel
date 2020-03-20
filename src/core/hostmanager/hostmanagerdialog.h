#ifndef HOSTMANAGERDIALOG_H
#define HOSTMANAGERDIALOG_H

#include "adddialog.h"
#include "host.h"
#include "hosttablemodel.h"

#include <QApplication>
#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>

namespace HostsFileManager
{
    class HostsManagerDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit HostsManagerDialog(QWidget *parent = nullptr);
        explicit HostsManagerDialog(QWidget &parent) = delete;
        ~HostsManagerDialog() override;

    signals:
        void selectionChanged(const QItemSelection &selected);

    public slots:
        void addEntry();
        void editEntry();
        void removeEntry();
        void accept() override;

    private:
        QTableView *tableView;
    };
} // namespace HostsFileManager

#endif // HOSTMANAGERDIALOG_H
