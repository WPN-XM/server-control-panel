#include "alreadyusedportsdialog.h"

#include <QDebug>

#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

AlreadyUsedPortsDialog::AlreadyUsedPortsDialog(QWidget *parent) :
    QDialog(parent)
{

}

void AlreadyUsedPortsDialog::checkAlreadyUsedPorts()
{
    qDebug() << "[Ports] Check for already used ports.";

    QLabel *labelA  = new QLabel(tr("The following ports are already used:"));
    QGroupBox *groupBox = new QGroupBox(tr("Used Ports"));
    QVBoxLayout *vbox = new QVBoxLayout;

    QList<PidAndPort> ports = Processes::getInstance()->getPortsList();

    // iterate over ports list and draw a label for each
    foreach(PidAndPort p, ports) {
        qDebug() << "[Ports] Used: " << p.pid << p.port;

        Process proc = Processes::getInstance()->findByPid(p.pid);


        // create label
        QLabel *label = new QLabel("Port " + p.port + " used by " + proc.name + " (PID " + p.pid + ")");
        vbox->addWidget(label);
    }

    groupBox->setLayout(vbox);

    QLabel *labelB = new QLabel(tr("Please configure your servers to avoid port collisions.<br><br>"
                                   "To proceed click Continue.<br>"));

    //QPushButton *ShutdownButton = new QPushButton(tr("Shutdown"));
    QPushButton *ContinueButton = new QPushButton(tr("Continue"));
    ContinueButton->setDefault(true);
    //ShutdownButton->setDefault(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    //buttonBox->addButton(ShutdownButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(ContinueButton, QDialogButtonBox::ActionRole);

    // build dialog to inform user about running processes
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(labelA);
    grid->addWidget(groupBox);
    grid->addWidget(labelB);
    grid->addWidget(buttonBox);

    QDialog *dlg = new QDialog(this);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setLayout(grid);
    dlg->resize(250, 100);
    dlg->setWindowTitle(tr(APP_NAME));
    dlg->setWindowFlags(dlg->windowFlags() | Qt::WindowStaysOnTopHint);

    // Set signal and slot for "Buttons"
    //connect(ShutdownButton, SIGNAL(clicked()), dlg, SLOT(accept()));
    connect(ContinueButton, SIGNAL(clicked()), dlg, SLOT(reject()));

    // show modal window
    int dialogCode = dlg->exec();

    // fire modal window event loop and wait for button clicks
    // if shutdown was clicked (accept), execute shutdowns
    // if continue was clicked (reject), do nothing and proceed to mainwindow
    if(dialogCode == QDialog::Accepted)
    {

    }
}
