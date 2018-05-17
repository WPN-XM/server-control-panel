#ifndef LABELWITHHOVERTOOLTIP_H
#define LABELWITHHOVERTOOLTIP_H

#include <QLabel>

#include "BalloonTip.h"

class LabelWithHoverTooltip : public QLabel
{
    Q_OBJECT

public:
    explicit LabelWithHoverTooltip(QWidget *parent = 0);
    void enableToolTip(bool enabled);
    void setTooltipText(const QString &value);

private:
    void openBalloonTipForPHP(const QString &message);
    BalloonTip *balloonTip = nullptr;
    QString myText;
    bool tooltipEnabled;

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
};

#endif // LABELWITHHOVERTOOLTIP_H
