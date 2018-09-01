#include "LabelWithHoverTooltip.h"
#include "BalloonTip.h"

#include <QDebug>

LabelWithHoverTooltip::LabelWithHoverTooltip(QWidget *parent) : QLabel(parent)
{
    this->setAttribute(Qt::WA_Hover, true);

    setAlignment(Qt::AlignCenter);
    setTextFormat(Qt::PlainText);
}

void LabelWithHoverTooltip::enableToolTip(bool enabled) { tooltipEnabled = enabled; }

void LabelWithHoverTooltip::setTooltipText(const QString &value) { myText = value; }

void LabelWithHoverTooltip::openBalloonTipForPHP(const QString &message)
{
    balloonTip = new BalloonTip("PHP Upstreams", message, 1000, this);
    balloonTip->setArrowPosition(BalloonTip::LeftTop);
    balloonTip->move(QCursor::pos());
    balloonTip->show();
}

void LabelWithHoverTooltip::enterEvent(QEvent *event)
{
    if (!tooltipEnabled) {
        return;
    }
    if (objectName() == "label_PHP_Port") {
        openBalloonTipForPHP(myText);
    }
    QWidget::enterEvent(event);
}

void LabelWithHoverTooltip::leaveEvent(QEvent *event)
{
    if (!tooltipEnabled) {
        return;
    }
    if (balloonTip != nullptr) {
        balloonTip->close();
        delete balloonTip;
    }

    QWidget::leaveEvent(event);
}
