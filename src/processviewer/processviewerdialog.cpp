#include "processviewerdialog.h"
#include "ui_processviewerdialog.h"

#include <QDebug>

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

    ui->treeWidget->setColumnCount(4);
    ui->treeWidget->setSortingEnabled(true);

    // resize columns to contents
    ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_NAME, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_PID, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_PORT, QHeaderView::ResizeToContents);

    // set text alignment for columns
    //ui->treeWidget->setText()

    renderProcesses();

    ui->treeWidget->expandAll();       

    // connect buttons
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(close()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void ProcessViewerDialog::setProcessesInstance(Processes *p)
{
    processes = p;
}

void ProcessViewerDialog::setChecked_ShowOnlyWpnxmProcesses()
{
    ui->checkBox_filterShowOnlyWpnxmProcesses->setEnabled(true);
    ui->checkBox_filterShowOnlyWpnxmProcesses->setChecked(true);
}

void ProcessViewerDialog::refreshProcesses()
{
    ui->treeWidget->clear();
    processes->refresh();
    renderProcesses();
    ui->treeWidget->expandAll();
}

void ProcessViewerDialog::renderProcesses()
{
    foreach(Process process, processes->getProcessesList())
    {
        // find parentItem in the tree by looking for parentId recursivley
        QList<QTreeWidgetItem *> parentItem = ui->treeWidget->findItems(
            process.ppid, Qt::MatchContains | Qt::MatchRecursive, 1
        );

        // lookup port for this pid and add it to the process struct
        foreach (const PidAndPort &p, ports) {
            if (p.pid == process.pid) {
                process.port = p.port;
                break;
            }
        }

        // if there is a parent item, then add the proccess as a child, else it's a parent itself
        if(!parentItem.empty()) {
            addChild(parentItem.at(0), process);
        } else {
            addRoot(process);
        }
    }
}

void ProcessViewerDialog::on_pushButton_Refresh_released()
{
    refreshProcesses();
}

void ProcessViewerDialog::on_lineEdit_searchProcessByName_textChanged(const QString &query)
{
    filter("byName", query);
}

void ProcessViewerDialog::on_lineEdit_searchProcessByPid_textChanged(const QString &query)
{
    filter("byPid", query);
}

void ProcessViewerDialog::on_lineEdit_searchProcessByPort_textChanged(const QString &query)
{
    filter("byPort", query);
}

/**
 * Search for items in the Progess TreeWidget by using type-ahead search
 */
void ProcessViewerDialog::filter(QString filterByItem, const QString &query)
{
    int searchInColumn = Columns::COLUMN_NAME;

    if(filterByItem == "byName") {
        searchInColumn = Columns::COLUMN_NAME;
    }
    if(filterByItem == "byPort") {
        searchInColumn = Columns::COLUMN_PORT;
    }
    if(filterByItem == "byPID") {
        searchInColumn = Columns::COLUMN_PID;
    }

    ui->treeWidget->expandAll();

    // Iterate over all child items : filter items with "contains" query
    QTreeWidgetItemIterator iterator(ui->treeWidget, QTreeWidgetItemIterator::All);
    while(*iterator)
    {
        QTreeWidgetItem *item = *iterator;
        if(item && item->text(searchInColumn).contains(query, Qt::CaseInsensitive)) {
            item->setHidden(false);
        } else {
            // Problem: the matched child is visibile, but parent is hidden, because no match.
            // so, lets hide only items without childs.
            // any not matching parent will stay visible.. until next iteration, see below.
            if(item->childCount() == 0) {
                item->setHidden(true);
            }
        }
        ++iterator;
    }

    // Iterate over items with childs : hide, if they do not have a matching (visible) child (see above).
    QTreeWidgetItemIterator parentIterator(ui->treeWidget, QTreeWidgetItemIterator::HasChildren);
    while(*parentIterator)
    {
        QTreeWidgetItem *item = *parentIterator;
        // count the number of hidden childs
        int childs = item->childCount();
        int hiddenChilds = 0;
        for (int i = 0; i < childs; ++i) {
            if(item->child(i)->isHidden()) {
                ++hiddenChilds;
            }
        }
        // finally: if all childs are hidden, hide the parent (*item), too
        if(hiddenChilds == childs) {
            item->setHidden(true);
        }
        ++parentIterator;
    }
}

ProcessViewerDialog::~ProcessViewerDialog()
{
    delete ui;
}

QTreeWidgetItem* ProcessViewerDialog::addRoot(Process process)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, process.name);
    item->setIcon(0, process.icon);
    item->setData(0, Qt::ItemDataRole::ToolTipRole, process.path);
    item->setText(1, process.pid);
    item->setText(2, process.port);
    item->setText(3, process.memoryUsage);
    ui->treeWidget->addTopLevelItem(item);
    return item;
}

void ProcessViewerDialog::addChild(QTreeWidgetItem *parent, Process process)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, process.name);
    item->setIcon(0, process.icon);
    item->setData(0, Qt::ItemDataRole::ToolTipRole, process.path);
    item->setText(1, process.pid);
    item->setText(2, process.port);
    item->setText(3, process.memoryUsage);
    parent->addChild(item);
}

void ProcessViewerDialog::on_pushButton_KillProcess_released()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    quint64 pid = item->text(1).toLong();
    if(killProcess(pid)) {
        QObject().thread()->usleep(1000*1000*0.5); // 0,5sec
        refreshProcesses();
    }
}

bool ProcessViewerDialog::killProcess(quint64 pid)
{
    return Processes::killProcess(pid);
}


void ProcessViewerDialog::on_checkBox_filterExcludeWindowsProcesses_checked()
{
    // exclude all windows system processes

    // remove PIDs below 1000
    // remove names with c:\windows
}

void ProcessViewerDialog::on_checkBox_filterIncludeOnlyWpnxmProcesses_checked()
{
    // show only processes from our folder structure

    // remove PIDs below 1000
    // remove all names without the install location of the server stack
}
