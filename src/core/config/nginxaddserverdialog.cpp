#include "nginxaddserverdialog.h"
#include "ui_nginxaddserverdialog.h"

namespace Configuration
{
    NginxAddServerDialog::NginxAddServerDialog(QWidget *parent)
        : QDialog(parent), ui(new Configuration::Ui::NginxAddServerDialog)
    {
        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        ui->setupUi(this);
    }

    NginxAddServerDialog::~NginxAddServerDialog() { delete ui; }

    QString NginxAddServerDialog::address() { return ui->lineEdit_IP->text().trimmed(); }

    QString NginxAddServerDialog::port() { return ui->lineEdit_Port->text().trimmed(); }

    QString NginxAddServerDialog::weight() { return ui->lineEdit_Weight->text().trimmed(); }

    QString NginxAddServerDialog::maxfails() { return ui->lineEdit_MaxFails->text().trimmed(); }

    QString NginxAddServerDialog::timeout() { return ui->lineEdit_Timeout->text().trimmed(); }

    QString NginxAddServerDialog::phpchildren() { return ui->lineEdit_PHPChildren->text().trimmed(); }
} // namespace Configuration
