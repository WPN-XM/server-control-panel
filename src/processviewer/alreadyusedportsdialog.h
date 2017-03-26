#ifndef ALREADYUSEDPORTSDIALOG_H
#define ALREADYUSEDPORTSDIALOG_H

#include <QApplication>
#include <QDialog>
#include <QObject>
#include <QWidget>

#include "../processviewer/processes.h"
#include "../version.h"

class AlreadyUsedPortsDialog : public QDialog
{
    Q_OBJECT

public:
    AlreadyUsedPortsDialog(QWidget *parent = false);
    void checkAlreadyUsedPorts();
};

#endif // ALREADYUSEDPORTSDIALOG_H
