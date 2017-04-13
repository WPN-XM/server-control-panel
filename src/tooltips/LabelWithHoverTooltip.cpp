#include "LabelWithHoverTooltip.h"
#include "BalloonTip.h"

#include <QDebug>

LabelWithHoverTooltip::LabelWithHoverTooltip(QWidget *parent) : QLabel(parent)
{
    this->setAttribute(Qt::WA_Hover, true);

    setAlignment(Qt::AlignCenter);
    setTextFormat(Qt::PlainText);
}

void LabelWithHoverTooltip::setText(const QString &value)
{
    if (objectName() == "label_PHP_Port") {
        text() = "The following upstreams are running: \n \n bla \n blubb \n " + value;
    } else {
        text() = value;
    }
}

void LabelWithHoverTooltip::openBalloonTipForPHP(const QString &message)
{
    balloonTip = new BalloonTip("PHP Upstreams", message, 1000, this);
    balloonTip->setArrowPosition(BalloonTip::LeftTop);
    balloonTip->move(QCursor::pos());
    balloonTip->show();
}

void LabelWithHoverTooltip::enterEvent(QEvent *event)
{
    qDebug() << Q_FUNC_INFO << objectName();
    if (objectName() == "label_PHP_Port") {
        qDebug() << text();
        openBalloonTipForPHP(text());
    }
    QWidget::enterEvent(event);
}

void LabelWithHoverTooltip::leaveEvent(QEvent *event)
{
    qDebug() << Q_FUNC_INFO << objectName();

    if (balloonTip != 0) {
        balloonTip->close();
        delete balloonTip;
    }

    QWidget::leaveEvent(event);
}
