#ifndef LABELWITHHOVERTOOLTIP_H
#define LABELWITHHOVERTOOLTIP_H

#include <QLabel>

#include "BalloonTip.h"

namespace Tooltips
{

    class LabelWithHoverTooltip : public QLabel
    {
        Q_OBJECT

    public:
        explicit LabelWithHoverTooltip(QWidget *parent = nullptr);
        void enableToolTip(bool enabled);
        void setTooltipText(const QString &value);

    private:
        void openBalloonTipForPHP(const QString &message);
        BalloonTip *balloonTip = nullptr;
        QString myText;
        bool tooltipEnabled{};

    protected:
        void enterEvent(QEvent *event);
        void leaveEvent(QEvent *event);
    };

} // namespace Tooltips

#endif // LABELWITHHOVERTOOLTIP_H
