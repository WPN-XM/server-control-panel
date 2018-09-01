#include "processviewerdialog.h"
#include "ui_processviewerdialog.h"

#include <QDebug>

ProcessViewerDialog::ProcessViewerDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ProcessViewerDialog) {
  ui->setupUi(this);

  setWindowTitle("Process Viewer");

  // remove question mark from the title bar
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  // center dialog
  setGeometry(
      QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(),
                          QApplication::desktop()->availableGeometry(this)));

  ui->treeWidget->setColumnCount(4);
  ui->treeWidget->setSortingEnabled(true);

  // resize columns to contents
  ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_NAME,
                                                 QHeaderView::ResizeToContents);
  ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_PID,
                                                 QHeaderView::ResizeToContents);
  ui->treeWidget->header()->setSectionResizeMode(Columns::COLUMN_PORT,
                                                 QHeaderView::ResizeToContents);

  // set text alignment for columns
  // ui->treeWidget->setText()

  renderProcesses();

  ui->treeWidget->expandAll();

  // connect buttons
  connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(close()));
  connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));

  setFocus();
}

void ProcessViewerDialog::setProcessesInstance(Processes *p) { processes = p; }

void ProcessViewerDialog::setChecked_ShowOnlyWpnxmProcesses() {
  // ui->checkBox_filterShowOnlyWpnxmProcesses->setEnabled(true);
  // ui->checkBox_filterShowOnlyWpnxmProcesses->setChecked(true);
}

void ProcessViewerDialog::refreshProcesses() {
  ui->treeWidget->clear();
  renderProcesses();
  ui->treeWidget->expandAll();
}

void ProcessViewerDialog::renderProcesses() {
  foreach (Process process, processes->getRunningProcesses()) {
    // find parentItem in the tree by looking for parentId recursivley
    QList<QTreeWidgetItem *> parentItem = ui->treeWidget->findItems(
        process.ppid, Qt::MatchContains | Qt::MatchRecursive, 1);

    // lookup port for this pid and add it to the process struct
    foreach (const PidAndPort &p, processes->getPorts()) {
      if (p.pid == process.pid) {
        process.port = p.port;
        break;
      }
    }

    // if there is a parent item, then add the proccess as a child, else it's a
    // parent itself
    if (!parentItem.empty()) {
      addChild(parentItem.at(0), process);
    } else {
      addRoot(process);
    }
  }
}

void ProcessViewerDialog::on_pushButton_Refresh_released() {
  refreshProcesses();
}

void ProcessViewerDialog::on_lineEdit_searchProcessByName_textChanged(
    const QString &query) {
  filter("byName", query);
}

void ProcessViewerDialog::on_lineEdit_searchProcessByPid_textChanged(
    const QString &query) {
  filter("byPid", query);
}

void ProcessViewerDialog::on_lineEdit_searchProcessByPort_textChanged(
    const QString &query) {
  filter("byPort", query);
}

/**
 * Search for items in the Progess TreeWidget by using type-ahead search
 */
void ProcessViewerDialog::filter(QString filterByItem, const QString &query) {
  int searchInColumn = Columns::COLUMN_NAME;

  if (filterByItem == "byName") {
    searchInColumn = Columns::COLUMN_NAME;
  }
  if (filterByItem == "byPort") {
    searchInColumn = Columns::COLUMN_PORT;
  }
  if (filterByItem == "byPID") {
    searchInColumn = Columns::COLUMN_PID;
  }

  ui->treeWidget->expandAll();

  // Iterate over all child items : filter items with "contains" query
  QTreeWidgetItemIterator iterator(ui->treeWidget,
                                   QTreeWidgetItemIterator::All);
  while (*iterator) {
    QTreeWidgetItem *item = *iterator;
    if (item &&
        item->text(searchInColumn).contains(query, Qt::CaseInsensitive)) {
      item->setHidden(false);
    } else {
      // Problem: the matched child is visibile, but parent is hidden, because
      // no match.
      // so, lets hide only items without childs.
      // any not matching parent will stay visible.. until next iteration, see
      // below.
      if (item->childCount() == 0) {
        item->setHidden(true);
      }
    }
    ++iterator;
  }

  // Iterate over items with childs : hide, if they do not have a matching
  // (visible) child (see above).
  QTreeWidgetItemIterator parentIterator(ui->treeWidget,
                                         QTreeWidgetItemIterator::HasChildren);
  while (*parentIterator) {
    QTreeWidgetItem *item = *parentIterator;
    // count the number of hidden childs
    int childs = item->childCount();
    int hiddenChilds = 0;
    for (int i = 0; i < childs; ++i) {
      if (item->child(i)->isHidden()) {
        ++hiddenChilds;
      }
    }
    // finally: if all childs are hidden, hide the parent (*item), too
    if (hiddenChilds == childs) {
      item->setHidden(true);
    }
    ++parentIterator;
  }
}

ProcessViewerDialog::~ProcessViewerDialog() { delete ui; }

QTreeWidgetItem *ProcessViewerDialog::addRoot(Process process) {
  auto *item = new QTreeWidgetItem(ui->treeWidget);
  item->setText(0, process.name);
  item->setIcon(0, process.icon);
  item->setData(0, Qt::ItemDataRole::ToolTipRole, process.path);
  item->setText(1, process.pid);
  item->setText(2, process.port);
  item->setText(3, process.memoryUsage);
  ui->treeWidget->addTopLevelItem(item);
  return item;
}

void ProcessViewerDialog::addChild(QTreeWidgetItem *parent, Process process) {
  auto *item = new QTreeWidgetItem();
  item->setText(0, process.name);
  item->setIcon(0, process.icon);
  item->setData(0, Qt::ItemDataRole::ToolTipRole, process.path);
  item->setText(1, process.pid);
  item->setText(2, process.port);
  item->setText(3, process.memoryUsage);
  parent->addChild(item);
}

void ProcessViewerDialog::on_pushButton_KillProcess_released() {
  QTreeWidgetItem *item = ui->treeWidget->currentItem();

  if (!item) {  // do nothing, if no item selected
    return;
  }

  qint64 pid = item->text(1).toLong();
  if (Processes::killProcess(pid)) {
    QObject().thread()->usleep(1000 * 1000 * 0.5);  // 0,5sec
    refreshProcesses();
  }
}

void ProcessViewerDialog::
    on_checkBox_filterExcludeWindowsProcesses_stateChanged(int state) {
  if (state == Qt::Checked) {
    qDebug() << "exclude all windows system processes";

    QTreeWidgetItemIterator iterator(ui->treeWidget,
                                     QTreeWidgetItemIterator::All);

    while (*iterator) {
      QTreeWidgetItem *item = *iterator;

      if (item && !item->isHidden()) {
        // remove names with c:\windows
        QString path =
            item->data(Columns::COLUMN_NAME, Qt::ItemDataRole::ToolTipRole)
                .toString();
        if (path.contains("C:\\Windows", Qt::CaseInsensitive)) {
          qDebug() << "removed windows item" << path;
          item->setHidden(true);
        }

        // name
        if (item->text(Columns::COLUMN_NAME).contains("svchost.exe") ||
            item->text(Columns::COLUMN_NAME).contains("fontdrvhost.exe") ||
            item->text(Columns::COLUMN_NAME).contains("dwm.exe") ||
            item->text(Columns::COLUMN_NAME).contains("upeksvr.exe")) {
          qDebug() << "removed windows item because name:"
                   << item->text(Columns::COLUMN_NAME);
          item->setHidden(true);
        }
      }
      ++iterator;
    }
  }

  if (state == Qt::Unchecked) {
    refreshProcesses();
  }
}

void ProcessViewerDialog::on_checkBox_filterShowOnlyWpnxmProcesses_stateChanged(
    int state) {
  if (state == Qt::Checked) {
    qDebug() << "show only processes from our folder structure";

    QTreeWidgetItemIterator iterator(ui->treeWidget,
                                     QTreeWidgetItemIterator::All);

    while (*iterator) {
      QTreeWidgetItem *item = *iterator;

      if (item && !item->isHidden() &&
          !item->text(Columns::COLUMN_NAME).contains("wpn-xm")) {
        // remove all names without the install location of the server stack
        QString path =
            item->data(Columns::COLUMN_NAME, Qt::ItemDataRole::ToolTipRole)
                .toString();
        if (!path.contains(qApp->applicationDirPath(), Qt::CaseInsensitive)) {
          qDebug() << "removed app dir " << path << qApp->applicationDirPath();
          item->setHidden(true);
        }
      }
      ++iterator;
    }
  }

  if (state == Qt::Unchecked) {
    refreshProcesses();
  }
}
