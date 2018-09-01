#ifndef TrayToolTip_H
#define TrayToolTip_H

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class TrayToolTip : public QWidget
{
    Q_OBJECT

public:
    explicit TrayToolTip(QWidget *parent = nullptr);
    void showMessage(const QPixmap &pixmap, const QString &title, const QString &msg, QPoint point);
    void showMessage(const QString &msg, QPoint point);

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
