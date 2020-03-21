#ifndef NGINXADDUPSTREAMDIALOG_H
#define NGINXADDUPSTREAMDIALOG_H

#include "common.h"

#include <QDialog>

namespace Configuration
{
    namespace Ui
    {
        class NginxAddPoolDialog;
    }

    class APP_CORE_EXPORT NginxAddUpstreamDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit NginxAddUpstreamDialog(QWidget *parent = nullptr);
        explicit NginxAddUpstreamDialog(QWidget &parent) = delete; // ???
        ~NginxAddUpstreamDialog() override;

        QString pool();
        QString method();

        enum Column
        {
            Pool,
            Method
        };

    private:
        Ui::NginxAddPoolDialog *ui;
    };
} // namespace Configuration
#endif // NGINXADDUPSTREAMDIALOG_H
