#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QWidget>

namespace Ui
{
    class Plugin_HelloWorld_ConfigDialog;
}

namespace Plugin_HelloWorld_NS
{
    class Plugin_HelloWorld_ConfigDialog : public QWidget
    {
        Q_OBJECT

    public:
        explicit Plugin_HelloWorld_ConfigDialog(QWidget *parent = nullptr);
        ~Plugin_HelloWorld_ConfigDialog();

    private:
        Ui::Plugin_HelloWorld_ConfigDialog *ui;
    };

} // namespace Plugin_HelloWorld_NS

#endif // CONFIGDIALOG_H
