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

    // center dialog
    this->move(QApplication::desktop()->screen()->rect().center() - this->rect().center());

    ui->treeWidget->setColumnCount(3);

    // get process data in CSV format using a "Windows Management Instrumentation Commandline" query
    //QString cmd = "wmic process where \"ExecutablePath LIKE '%c%'\" GET Name, ExecutablePath, ProcessId, ParentProcessId, WorkingSetSize /format:csv";
    QString cmd = "wmic process GET Name, ExecutablePath, ParentProcessId, ProcessId, WorkingSetSize /format:csv";

    QList<QStringList> list = runWMICQuery(cmd);

    foreach(QStringList item, list) {

        // skip items
        if(item.first() == "") continue;      // 1 line: empty list element
        if(item.first() == "Node") continue;  // 2 line: list with column keys

        qDebug() << item;

        QString host        = item.at(0);
        QString path        = item.at(1);
        QString name        = item.at(2);
        QString parentpid   = item.at(3);
        QString pid         = item.at(4);
        QString mem         = getSizeHumanReadable(item.at(5));

        // find parentItem in the tree by looking for parentId recursivley
        QList<QTreeWidgetItem *> parentItem = ui->treeWidget->findItems(parentpid, Qt::MatchContains | Qt::MatchRecursive, 1);

        // found a parentItem?
        if(!parentItem.empty()) {
            // grab the rootItem
            QTreeWidgetItem *rootItem = parentItem.at(0);

            addChild(rootItem, name, pid, mem);
        } else {
            addRoot(name, pid, mem);
            continue;
        }
    }

    // resize columns to contents
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    ui->treeWidget->expandAll();
}

ProcessViewerDialog::~ProcessViewerDialog()
{
    delete ui;
}

QTreeWidgetItem* ProcessViewerDialog::addRoot(QString processName, QString pid, QString mem)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, processName);
    item->setText(1, pid);
    item->setText(2, mem);
    ui->treeWidget->addTopLevelItem(item);
    return item;
}

void ProcessViewerDialog::addChild(QTreeWidgetItem *parent, QString processName, QString pid, QString mem)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, processName);
    item->setText(1, pid);
    item->setText(2, mem);
    parent->addChild(item);
}

QList<QStringList> ProcessViewerDialog::runWMICQuery(const QString &cmd)
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

QString ProcessViewerDialog::getSizeHumanReadable(QString bytes)
{
     float num = bytes.toFloat();
     QStringList list;
     list << "KB" << "MB";

     QStringListIterator i(list);
     QString unit("bytes");

     while(num >= 1024.0 && i.hasNext())
      {
         unit = i.next();
         num /= 1024.0;
     }
     return QString::fromLatin1("%1 %2").arg(num, 3, 'f', 1).arg(unit);
}
