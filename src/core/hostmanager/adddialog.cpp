#include "adddialog.h"

namespace HostsFileManager
{
    HostsAddDialog::HostsAddDialog(QWidget *parent) : QDialog(parent)
    {
        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        QPushButton *btnOk     = new QPushButton("OK", this);
        QPushButton *btnCancel = new QPushButton("Cancel", this);

        lineEdit_Name    = new QLineEdit(this);
        lineEdit_Address = new QLineEdit(this);
        lineEdit_Address->setText("127.0.0.1");

        auto *gLayout = new QGridLayout;
        gLayout->setColumnStretch(1, 2);
        gLayout->addWidget(new QLabel("Address", this), 0, 0);
        gLayout->addWidget(lineEdit_Address, 0, 1);

        gLayout->addWidget(new QLabel("Name", this), 1, 0);
        gLayout->addWidget(lineEdit_Name, 1, 1);

        auto *buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(btnOk);
        buttonLayout->addWidget(btnCancel);

        gLayout->addLayout(buttonLayout, 2, 1, Qt::AlignRight);

        auto *mainLayout = new QVBoxLayout;
        mainLayout->addLayout(gLayout);
        setLayout(mainLayout);

        connect(btnOk, &QPushButton::clicked, this, &HostsAddDialog::accept);
        connect(btnCancel, &QPushButton::clicked, this, &HostsAddDialog::reject);

        setWindowTitle(tr("Add Host"));
    }

    QString HostsAddDialog::name() { return lineEdit_Name->text().trimmed(); }

    QString HostsAddDialog::address() { return lineEdit_Address->text().trimmed(); }

    void HostsAddDialog::edit(const QString &name, const QString &adress)
    {
        setWindowTitle(tr("Edit Host"));
        // m_leName->setEnabled(false);
        lineEdit_Name->setText(name);
        lineEdit_Address->setText(adress);
    }
} // namespace HostsFileManager
