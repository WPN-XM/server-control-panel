#include "configdialog.h"
#include "ui_configdialog.h"

namespace Plugin_HelloWorld_NS
{
    Plugin_HelloWorld_ConfigDialog::Plugin_HelloWorld_ConfigDialog(QWidget *parent)
        : QWidget(parent), ui(new Ui::Plugin_HelloWorld_ConfigDialog)
    {
        // constructor
        ui->setupUi(this);
    }

    Plugin_HelloWorld_ConfigDialog::~Plugin_HelloWorld_ConfigDialog()
    {
        // deconstructor
        delete ui;
    }

} // namespace Plugin_HelloWorld_NS
