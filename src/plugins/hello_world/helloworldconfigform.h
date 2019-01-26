#ifndef HELLOWORLCONFIGFORM_H
#define HELLOWORLCONFIGFORM_H

#include <QWidget>

namespace Ui {
class HelloWorlConfigForm;
}

class HelloWorlConfigForm : public QWidget
{
    Q_OBJECT

public:
    explicit HelloWorlConfigForm(QWidget *parent = nullptr);
    ~HelloWorlConfigForm();

private:
    Ui::HelloWorlConfigForm *ui;
};

#endif // HELLOWORLCONFIGFORM_H
