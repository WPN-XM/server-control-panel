#include "AlreadyRunningProcessesDialog.h"

#include <QDebug>

AlreadyRunningProcessesDialog::AlreadyRunningProcessesDialog(QWidget *parent) : QDialog(parent) {}

/*
 * Check active processes and report, if processes are already running.
 * We do this to avoid collisions.
 * A modal dialog with checkboxes for running processes is shown.
 * The user can select the processes to shutdown or continue.
 */
void AlreadyRunningProcessesDialog::checkAlreadyRunningServers()
{
    qDebug() << "[Processes Running] Check for already running processes.";

    // qDebug() << "Already running Processes found : " <<
    // Processes::getInstance()->getMonitoredProcessesList();

    // only show the "process shutdown" dialog, when there are processes to
    // shutdown
    // if(processes->areThereAlreadyRunningProcesses())
    //{
    QLabel *labelA      = new QLabel(tr("The following processes are already running:"));
    QGroupBox *groupBox = new QGroupBox(tr("Running Processes"));
    QVBoxLayout *vbox   = new QVBoxLayout;

    QList<Process> runningProcessesList = Processes::getRunningProcesses();

    // iterate over proccesFoundList and draw a "process shutdown" checkbox for
    // each one
    foreach (Process p, runningProcessesList) {
        // create checkbox
        QCheckBox *checkbox = new QCheckBox(p.name);
        checkbox->setChecked(true);
        checkbox->setCheckable(true);
        // add checkbox to view
        vbox->addWidget(checkbox);
    }

    groupBox->setLayout(vbox);

    QLabel *labelB =
        new QLabel(tr("Please select the processes you wish to shutdown.<br><br>"
                      "Click Shutdown to shut the selected processes down and continue "
                      "using the server control panel.<br>"
                      "To proceed without shutting processes down, click Continue.<br>"));

    QPushButton *ShutdownButton = new QPushButton(tr("Shutdown"));
    QPushButton *ContinueButton = new QPushButton(tr("Continue"));
    ShutdownButton->setDefault(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(ShutdownButton, QDialogButtonBox::ActionRole);
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
    connect(ShutdownButton, SIGNAL(clicked()), dlg, SLOT(accept()));
    connect(ContinueButton, SIGNAL(clicked()), dlg, SLOT(reject()));

    // show modal window
    int dialogCode = dlg->exec();

    // fire modal window event loop and wait for button clicks
    // if shutdown was clicked (accept), execute shutdowns
    // if continue was clicked (reject), do nothing and proceed to mainwindow
    if (dialogCode == QDialog::Accepted) {
        // fetch all checkboxes
        QList<QCheckBox *> allCheckBoxes = dlg->findChildren<QCheckBox *>();

        // iterate checkbox values
        int c = allCheckBoxes.size();
        for (int i = 0; i < c; ++i) {
            QCheckBox *cb = allCheckBoxes.at(i);
            if (cb->isChecked()) {
                qDebug() << "[Process Shutdown]" << cb->text();

                // handle the PostgreSQL PID file deletion, too
                if (cb->text() == "postgres") {
                    QString file =
                        QDir::toNativeSeparators(QApplication::applicationDirPath() + "/bin/pgsql/data/postmaster.pid");
                    if (QFile::exists(file)) {
                        QFile::remove(file);
                    }
                }

                // taskkill parameters:
                // /f = force shutdown, /t = structure shutdown, /im = the name of the
                // process
                // nginx and mariadb need a forced shutdown !
                QProcess process;
                process.start("cmd.exe", QStringList() << "/c"
                                                       << "taskkill /f /t /im " + cb->text());
                process.waitForFinished();
                qDebug() << "[Process Shutdown] Result:\n" << process.readAllStandardOutput();
            }
            delete cb;
        }

        // delay, after process kills
        Processes::delay(250);

        //}

        // if continue was clicked (reject), do update status indicators in
        // mainwindow and tray
        /*if(dialogCode == QDialog::Rejected)
    {
        emit signalUpdateServerStatusIndicators();
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
