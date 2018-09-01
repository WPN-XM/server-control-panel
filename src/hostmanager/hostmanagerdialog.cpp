#include "hostmanagerdialog.h"

namespace HostsFileManager
{
    HostsManagerDialog::HostsManagerDialog(QWidget *parent) : QDialog(parent)
    {
        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        auto *toolbar = new QToolBar(this);
        toolbar->addAction("Add", this, SLOT(addEntry()));
        toolbar->addAction("Edit", this, SLOT(editEntry()));
        toolbar->addAction("Delete", this, SLOT(removeEntry()));

        QPushButton *btnOk = new QPushButton(QApplication::style()->standardIcon(QStyle::SP_VistaShield), "OK", this);
        QPushButton *btnCancel = new QPushButton("Cancel", this);

        auto *tableModel = new HostsTableModel(this);
        tableModel->setList(Host::GetHosts());

        tableView = new QTableView(this);
        tableView->setModel(tableModel);
        tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableView->horizontalHeader()->setStretchLastSection(true);
        tableView->verticalHeader()->setVisible(false);
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableView->setSelectionMode(QAbstractItemView::SingleSelection);
        tableView->setMinimumWidth(300);

        auto *gLayout = new QGridLayout;
        gLayout->addWidget(toolbar, 0, 0);
        gLayout->addWidget(tableView, 1, 0);

        auto *buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(btnOk);
        buttonLayout->addWidget(btnCancel);

        gLayout->addLayout(buttonLayout, 2, 0, Qt::AlignRight);

        auto *mainLayout = new QVBoxLayout;
        mainLayout->addLayout(gLayout);
        setLayout(mainLayout);

        connect(btnOk, SIGNAL(clicked()), this, SLOT(accept()));
        connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
        // connect(tableView->selectionModel(),
        // SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
        // SIGNAL(selectionChanged(QItemSelection)));

        setWindowTitle(tr("WPX-XM Server Control Panel - Host File Manager"));
        setFixedWidth(400);
    }

    HostsManagerDialog::~HostsManagerDialog()
    {
        auto *model = dynamic_cast<HostsTableModel *>(tableView->model());
        qDeleteAll(model->getList());
    }

    void HostsManagerDialog::addEntry()
    {
        HostsAddDialog aDialog;

        if (aDialog.exec()) {
            QString name    = aDialog.name();
            QString address = aDialog.address();

            auto *model = dynamic_cast<HostsTableModel *>(tableView->model());
            QList<Host *> list     = model->getList();

            // do the add
            Host host(name, address);
            if (!list.contains(&host)) {
                model->insertRows(0, 1, QModelIndex());

                QModelIndex index = model->index(0, HostsTableModel::COLUMN_NAME, QModelIndex());
                model->setData(index, name, Qt::EditRole);
                index = model->index(0, HostsTableModel::COLUMN_ADDRESS, QModelIndex());
                model->setData(index, address, Qt::EditRole);
            } else {
                QMessageBox::information(this, tr("Duplicate Entry"), tr("The host mapping already exists."));
            }
        }
    }

    void HostsManagerDialog::editEntry()
    {
        auto *model              = dynamic_cast<HostsTableModel *>(tableView->model());
        QItemSelectionModel *selectionModel = tableView->selectionModel();

        QModelIndexList indexes = selectionModel->selectedRows();
        QModelIndex index;
        int row = -1;
        QString name;
        QString address;

        if (!indexes.empty()) {
            foreach (index, indexes) {
                row = index.row();

                QModelIndex indexName = model->index(row, HostsTableModel::COLUMN_NAME, QModelIndex());
                QVariant varName      = model->data(indexName, Qt::DisplayRole);
                name                  = varName.toString();

                QModelIndex indexAddress = model->index(row, HostsTableModel::COLUMN_ADDRESS, QModelIndex());
                QVariant varAddr         = model->data(indexAddress, Qt::DisplayRole);
                address                  = varAddr.toString();
            }

            HostsAddDialog aDialog;
            aDialog.edit(name, address);

            if (aDialog.exec()) {
                QString newAddress = aDialog.address();
                if (newAddress != address) {
                    QModelIndex i = model->index(row, HostsTableModel::COLUMN_ADDRESS, QModelIndex());
                    model->setData(i, newAddress, Qt::EditRole);
                }
            }
        }
    }

    void HostsManagerDialog::removeEntry()
    {
        HostsTableModel *model              = dynamic_cast<HostsTableModel *>(tableView->model());
        QItemSelectionModel *selectionModel = tableView->selectionModel();

        QModelIndexList indexes = selectionModel->selectedRows();
        QModelIndex index;

        foreach (index, indexes) {
            model->removeRows(index.row(), 1, QModelIndex());
        }
    }

    void HostsManagerDialog::accept()
    {
        auto *model = dynamic_cast<HostsTableModel *>(tableView->model());
        Host::SetHosts(model->getList());
        QDialog::accept();
    }
}
