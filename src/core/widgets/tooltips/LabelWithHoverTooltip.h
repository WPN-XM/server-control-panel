#ifndef LABELWITHHOVERTOOLTIP_H
#define LABELWITHHOVERTOOLTIP_H

#include "common.h"

#include <QLabel>

#include "BalloonTip.h"

namespace Widgets::Tooltips
{

    class APP_CORE_EXPORT LabelWithHoverTooltip : public QLabel
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
        void enterEvent(QEvent *event) override;
        void leaveEvent(QEvent *event) override;
    };

} // namespace Widgets::Tooltips

#endif // LABELWITHHOVERTOOLTIP_H
