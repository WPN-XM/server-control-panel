#include "AlreadyRunningProcessesDialog.h"

#include <QDebug>

AlreadyRunningProcessesDialog::AlreadyRunningProcessesDialog(QWidget *parent) :
    QDialog(parent)
{

}

void AlreadyRunningProcessesDialog::checkAlreadyRunningServers(Processes *processes)
{
    qDebug() << "[Processes Running] Check for already running processes.";

    // Check active processes and report, if processes are already running.
    // We do this to avoid collisions.
    // A modal dialog with checkboxes for running processes is shown.
    // The user might then select the processes to shutdown or continue.

    QList<Process> processList = processes->getRunningProcesses();

    // define processes to look for
    QStringList processesToSearch;
    processesToSearch << "nginx"
                      << "apache"
                      << "memcached"
                      << "mysqld"
                      << "spawn"
                      << "php-cgi"
                      << "mongod"
                      << "postgres"
                      << "redis-server";

    // init a list for found processes
    QStringList processesFoundList;

    // foreach processesToSearch take a look in the processList
    for (int i = 0; i < processesToSearch.size(); ++i)
    {
        //qDebug() << "Searching for process: " << processesToSearch.at(i).toLocal8Bit().constData();

        foreach(Process process, processList)
        {
            if((process.name).contains(processesToSearch.at(i).toLatin1().constData())) {
                // process found
                processesFoundList << processesToSearch.at(i).toLatin1().constData();
            }
        }
    }

    //qDebug() << "Already running Processes found : " << processesFoundList;

    // only show the "process shutdown" dialog, when there are processes to shutdown
    if(false == processesFoundList.isEmpty())
    {
        QLabel *labelA = new QLabel(tr("The following processes are already running:"));
        QGroupBox *groupBox = new QGroupBox(tr("Running Processes"));
        QVBoxLayout *vbox = new QVBoxLayout;

        // iterate over proccesFoundList and draw a "process shutdown" checkbox for each one
        int c = processesFoundList.size();
        for(int i = 0; i < c; ++i) {
           // create checkbox
           QCheckBox *checkbox = new QCheckBox(processesFoundList.at(i));
           checkbox->setChecked(true);
           checkbox->setCheckable(true);
           // add checkbox to view
           vbox->addWidget(checkbox);
        }

        groupBox->setLayout(vbox);

        QLabel *labelB = new QLabel(tr("Please select the processes you wish to shutdown.<br><br>"
                                       "Click Shutdown to shut the selected processes down and continue using the server control panel.<br>"
                                       "To proceed without shutting processes down, click Continue.<br>"));

        QPushButton *ShutdownButton = new QPushButton(tr("Shutdown"));
        QPushButton *ContinueButton = new QPushButton(tr("Continue"));
        ShutdownButton->setDefault(true);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
        buttonBox->addButton(ShutdownButton, QDialogButtonBox::ActionRole);
        buttonBox->addButton(ContinueButton, QDialogButtonBox::ActionRole);

        // e) build dialog to inform user about running processes
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
        connect(ShutdownButton, SIGNAL(clicked()), dlg, SLOT(accept()));
        connect(ContinueButton, SIGNAL(clicked()), dlg, SLOT(reject()));

        // show modal window
        int dialogCode = dlg->exec();

        // fire modal window event loop and wait for button clicks
        // if shutdown was clicked (accept), execute shutdowns
        // if continue was clicked (reject), do nothing and proceed to mainwindow
        if(dialogCode == QDialog::Accepted)
        {
            // fetch all checkboxes
            QList<QCheckBox *> allCheckBoxes = dlg->findChildren<QCheckBox *>();

            // iterate checkbox values
            c = allCheckBoxes.size();
            for(int i = 0; i < c; ++i) {
               QCheckBox *cb = allCheckBoxes.at(i);
               if(cb->isChecked())
               {
                   qDebug() << "[Process Shutdown]" << cb->text();

                   // handle the PostgreSQL PID file deletion, too
                   if(cb->text() == "postgres") {
                       QString file = QDir::toNativeSeparators(qApp->applicationDirPath() + "/bin/pgsql/data/postmaster.pid");
                       if(QFile().exists(file)) {
                           QFile().remove(file);
                       }
                   }

                   // taskkill parameters:
                   // /f = force shutdown, /t = structure shutdown, /im = the name of the process
                   // nginx and mariadb need a forced shutdown !
                   QProcess process;
                   process.start("cmd.exe", QStringList() <<"/c"<<"taskkill /f /t /im "+cb->text()+".exe");
                   process.waitForFinished();
                   qDebug() << "[Process Shutdown] Result:\n" << process.readAllStandardOutput();
               }
               delete cb;
            }
        }

        // if continue was clicked (reject), do update status indicators in mainwindow and tray
        /*if(dialogCode == QDialog::Rejected)
        {
            int c = processesFoundList.size();
            for(int i = 0; i < c; ++i) {
                QString procname = processesFoundList.at(i);
                QString servername = this->servers->getCamelCasedServerName(procname).toLocal8Bit().constData();
                Servers::Server *server = this->servers->getServer(servername.toLocal8Bit().constData());
                qDebug() << "[Processes Running] The process" << procname << " has the Server" << server->name;

                if(server->name != "Not Installed") {
                    // set indicator - main window
                    setLabelStatusActive(servername, true);
                    // set indicator - tray menu
                    server->trayMenu->setIcon(QIcon(":/status_run"));
                }
            }
        }*/

        delete vbox;
        delete labelA;
        delete labelB;
        delete ShutdownButton;
        delete ContinueButton;
        delete buttonBox;
        delete groupBox;
        delete grid;
        delete dlg;
    }
}

