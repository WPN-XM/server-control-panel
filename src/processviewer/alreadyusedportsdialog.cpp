#include "alreadyusedportsdialog.h"

#include <QDebug>

#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QColorDialog>
#include <QDir>

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
        //qDebug() << "[Ports] Used: " << p.pid << p.port;

        Process proc = Processes::getInstance()->findByPid(p.pid);

        // create label
        QLabel *label = new QLabel("Port " + p.port + " used by " + proc.name + " (PID " + p.pid + ")");

        // filter: skip ports used by chrome
        if(proc.name == "chrome.exe" || proc.name == "[System Process]" || proc.name == "svchost.exe") {
            continue;
        }

        // filter: system pid
        if(p.pid == "4") {
            continue;
        }

        // highlight commonly used ports: 80, 8080, 443
        if(p.port == "80" || p.port == "8080" || p.port == "443") {
            QPalette palette = label->palette();

            // check, if it a server from a prior wpnxm run (mark it green)
            if(QDir::cleanPath(proc.path).contains(QDir::currentPath())) {
                palette.setColor(QPalette::WindowText, Qt::darkGreen);
            } else {
                // else it's possible port collision (mark it red)
                palette.setColor(QPalette::WindowText, Qt::darkRed);
            }

            label->setPalette(palette);
        }

        vbox->addWidget(label);
    }

    groupBox->setLayout(vbox);

    QLabel *labelB = new QLabel(tr("Please configure your servers to avoid port collisions.<br><br>"
                                   "Items colored green are running servers from WPN-XM.<br>"
                                   "Items colored red are possible port collisions.<br>"
                                   "Items colored black are used ports.<br><br>"
                                   "The list excludes ports used by some system processes, svchost.exe, pid 4, chrome.exe.<br><br>"
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
