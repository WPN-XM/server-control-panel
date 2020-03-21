#ifndef NGINXADDSERVERDIALOG_H
#define NGINXADDSERVERDIALOG_H

#include "common.h"

#include <QDialog>

namespace Configuration
{
    namespace Ui
    {
        class NginxAddServerDialog;
    }

    class APP_CORE_EXPORT NginxAddServerDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit NginxAddServerDialog(QWidget *parent = nullptr);
        ~NginxAddServerDialog() override;

        QString address();
        QString port();
        QString weight();
        QString maxfails();
        QString timeout();
        QString phpchildren();

        enum Column
        {
            Address,
            Port,
            Weight,
            MaxFails,
            Timeout,
            PHPChildren
        };

    private:
        Ui::NginxAddServerDialog *ui;
    };
} // namespace Configuration

#endif // NGINXADDSERVERDIALOG_H
