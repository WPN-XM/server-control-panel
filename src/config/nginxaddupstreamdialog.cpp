#include "nginxaddupstreamdialog.h"
#include "ui_nginxaddupstreamdialog.h"

namespace Configuration
{
    NginxAddUpstreamDialog::NginxAddUpstreamDialog(QWidget *parent)
        : QDialog(parent), ui(new Configuration::Ui::NginxAddPoolDialog)
    {
        // remove question mark from the title bar
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        ui->setupUi(this);
    }

    NginxAddUpstreamDialog::~NginxAddUpstreamDialog() { delete ui; }

    QString NginxAddUpstreamDialog::pool() { return ui->lineEdit_pool->text().trimmed(); }

    QString NginxAddUpstreamDialog::method() { return ui->lineEdit_method->text().trimmed(); }
}
