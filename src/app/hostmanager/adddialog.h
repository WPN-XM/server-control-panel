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
        explicit HostsAddDialog(QWidget *parent = nullptr);
        QString name();
        QString address();

        void edit(const QString &name, const QString &adress);

    signals:

    public slots:

    private:
        QLineEdit *lineEdit_Name;
        QLineEdit *lineEdit_Address;
    };
} // namespace HostsFileManager

#endif // ADDDIALOG_H
