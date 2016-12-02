#include "processviewerdialog.h"
#include "ui_processviewerdialog.h"

ProcessViewerDialog::ProcessViewerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProcessViewerDialog)
{
    ui->setupUi(this);

    setWindowTitle("Process Viewer");

    // remove question mark from the title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setColumnWidth(0, 130);
    ui->treeWidget->setColumnWidth(1, 90);

    // get process data in CSV format using a "Windows Management Instrumentation Commandline" query
    QString cmd = "wmic process where \"ExecutablePath LIKE '%server_main%'\" GET Name, ExecutablePath, ProcessId, ParentProcessId /format:csv";

    QList<QStringList> list = execute(cmd);

    foreach(QStringList item, list) {

        // skip items
        if(item.first() == "") continue;      // 1 empty list element
        if(item.first() == "Node") continue;  // 2 list with keys

        //qDebug() << item;

        // add elements, which are always a root node
        if(item.at(2) == "spawn.exe" || item.at(2) != "php-cgi.exe") {
            addRoot(item.at(2), item.at(4));
            continue;
        }

        // add PHP process as child, that's the case, when the spawner is used
        if(item.at(2) == "php-cgi.exe") {
            // find root by parentId
            QTreeWidgetItem *rootItem = ui->treeWidget->findItems(item.at(3), Qt::MatchExactly, 1).at(0);
            // add this PHP process as child of that root
            addChild(rootItem, item.at(2), item.at(4));
        }
    }

    ui->treeWidget->expandAll();
}

ProcessViewerDialog::~ProcessViewerDialog()
{
    delete ui;
}

QTreeWidgetItem* ProcessViewerDialog::addRoot(QString name, QString description)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, name);
    item->setText(1, description);
    ui->treeWidget->addTopLevelItem(item);
    return item;
}

void ProcessViewerDialog::addChild(QTreeWidgetItem *parent, QString name, QString description)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, name);
    item->setText(1, description);
    parent->addChild(item);
}

QList<QStringList> ProcessViewerDialog::execute(const QString &cmd)
{
    QProcess process;
    process.start(cmd);

    if (!process.waitForStarted()) {
        return QList<QStringList>();
    }

    if (!process.waitForFinished()) {
        return QList<QStringList>();
    }

    int res = process.exitCode();
    if (res) {  //error
        return QList<QStringList>();
    }

    QByteArray data = process.readAllStandardOutput();
    QString string = QString::fromLocal8Bit(data);

    QList<QStringList> list = File::CSV::parseFromString(string);

    return list;
}
