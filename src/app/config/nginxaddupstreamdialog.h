#ifndef NGINXADDUPSTREAMDIALOG_H
#define NGINXADDUPSTREAMDIALOG_H

#include <QDialog>

namespace Configuration
{
    namespace Ui
    {
        class NginxAddPoolDialog;
    }

    class NginxAddUpstreamDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit NginxAddUpstreamDialog(QWidget *parent = nullptr);
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
