#include "src/tooltips/TrayToolTip.h"
#include "src/version.h"

TrayToolTip::TrayToolTip(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);

    // to hide it from appearing in the task manager
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground, true);

    this->resize(tipSizeX, tipSizeY);
    this->setObjectName("TrayToolTip");

    groupBox = new QGroupBox(this);
    groupBox->setGeometry(10, 10, tipSizeX - 20, tipSizeY - 10);
    groupBox->setStyleSheet("border: 1px solid #ccc; background-color: #eee; border-radius: 2px;");

    labelGroupBoxTitle = new QLabel(groupBox);
    labelGroupBoxTitle->setText(APP_NAME_AND_VERSION);
    labelGroupBoxTitle->setFont(QFont("Verdana", 7, QFont::Light));
    labelGroupBoxTitle->setStyleSheet(
        "border-radius: 0; border:0; "
        "padding-bottom: 3px; border-bottom: 1px "
        "solid #ccc");

    labelIcon = new QLabel(groupBox);
    labelIcon->setStyleSheet("border:0;");

    labelTitle = new QLabel(groupBox);
    labelTitle->setStyleSheet("border:0;");

    labelMessage = new QLabel(groupBox);
    labelMessage->setStyleSheet("border:0;");

    vBoxLayout = new QVBoxLayout();
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->addWidget(labelGroupBoxTitle);
    vBoxLayout->addWidget(labelTitle);
    vBoxLayout->addWidget(labelMessage);

    hBoxLayout = new QHBoxLayout(groupBox);
    hBoxLayout->setContentsMargins(5, 5, 15, 5);
    hBoxLayout->addWidget(labelIcon, 1, Qt::AlignCenter);
    hBoxLayout->addLayout(vBoxLayout, 4);
}

void TrayToolTip::showMessage(const QPixmap &pixmap, QString title, QString msg, QPoint point)
{
    labelIcon->setPixmap(pixmap);
    labelTitle->setText(title);
    labelMessage->setText(msg);

    // TrayToolTip
    int rectX, rectY;
    rectX = point.rx() - (tipSizeX / 3);
    rectY = point.ry() - (tipSizeY + 10);
    move(QPoint(rectX, rectY));
    QWidget::show();
}

void TrayToolTip::showMessage(QString msg, QPoint point)
{
    labelMessage->setText(msg);

    // TrayToolTip
    int rectX, rectY;
    rectX = point.rx() - (tipSizeX / 3);
    rectY = point.ry() - (tipSizeY + 10);
    move(QPoint(rectX, rectY));
    QWidget::show();
}

void TrayToolTip::hoverEvent(QHoverEvent *) { hide(); }
