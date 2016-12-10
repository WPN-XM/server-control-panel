#ifndef TrayToolTip_H
#define TrayToolTip_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TrayToolTip : public QWidget
{
    Q_OBJECT

    public:
        explicit TrayToolTip(QWidget *parent = 0);
        void showMessage(const QPixmap &pixmap, QString title, QString msg, QPoint point);
        void showMessage(QString msg, QPoint point);

    signals:

    public slots:

    private:
        QLabel *labelGroupBoxTitle;
        QFrame *hLine;
        QLabel *labelIcon;
        QLabel *labelTitle;
        QLabel *labelMessage;

        QHBoxLayout *hBoxLayout;
        QVBoxLayout *vBoxLayout;

        QGroupBox *groupBox;

        static const int tipSizeX = 250;
        static const int tipSizeY = 140;

    protected:
        void hoverEvent(QHoverEvent *);
};

#endif // TrayToolTip_H
