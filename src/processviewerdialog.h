#ifndef PROCESSVIEWERDIALOG_H
#define PROCESSVIEWERDIALOGg_H

#include "src/csv.h"

#include <QDialog>
#include <QTreeWidgetItem>
#include <QProcess>
#include <QDebug>
#include <QDesktopWidget>

namespace Ui {
    class ProcessViewerDialog;
}

class ProcessViewerDialog : public QDialog
{
    Q_OBJECT

    public:
        explicit ProcessViewerDialog(QWidget *parent = 0);
        ~ProcessViewerDialog();

        QTreeWidgetItem* addRoot(QString processName, QString pid, QString mem);
        void addChild(QTreeWidgetItem *parent, QString processName, QString pid, QString mem);

        QList<QStringList> runWMICQuery(const QString &cmd);

    private:
        Ui::ProcessViewerDialog *ui;

        QString getSizeHumanReadable(QString bytes);
};

#endif // PROCESSVIEWERDIALOG_H
