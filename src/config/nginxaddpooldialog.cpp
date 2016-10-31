#include "nginxaddpooldialog.h"
#include "ui_nginxaddpooldialog.h"

namespace Configuration
{
    NginxAddPoolDialog::NginxAddPoolDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Configuration::Ui::NginxAddPoolDialog)
    {
        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        ui->setupUi(this);
    }

    NginxAddPoolDialog::~NginxAddPoolDialog()
    {
        delete ui;
    }

    QString NginxAddPoolDialog::pool()
    {
        return ui->lineEdit_pool->text().trimmed();
    }

    QString NginxAddPoolDialog::method()
    {
        return ui->lineEdit_method->text().trimmed();
    }
}
