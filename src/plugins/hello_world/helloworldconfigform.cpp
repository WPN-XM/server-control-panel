#include "helloworlconfigform.h"
#include "ui_helloworlconfigform.h"

HelloWorlConfigForm::HelloWorlConfigForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HelloWorlConfigForm)
{
    ui->setupUi(this);
}

HelloWorlConfigForm::~HelloWorlConfigForm()
{
    delete ui;
}
