#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "host.h"

namespace HostsFileManager
{
    class HostsAddDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit HostsAddDialog(QWidget *parent = 0);
        QString name();
        QString address();

        void edit(QString name, QString adress);

    signals:

    public slots:

    private:
        QLineEdit *lineEdit_Name;
        QLineEdit *lineEdit_Address;
    };
}

#endif // ADDDIALOG_H
