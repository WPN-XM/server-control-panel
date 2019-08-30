#ifndef BALLONTIP_H
#define BALLONTIP_H

#include <QFrame>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>

#include <QStyle>

namespace Tooltips
{

    /*
      Class TipButton : button used in the BalloonTip class
    */
    class TipButton : public QPushButton
    {
        Q_OBJECT
    public:
        /* The role determines the look-and-feel of the button. */
        enum TipButtonRoles
        {
            Close,
            Config,
            NoButton
        };
        Q_DECLARE_FLAGS(TipButtonRole, TipButtonRoles)
        Q_ENUMS(TipButtonRoles)

    protected:
        TipButtonRole my_role;

    public:
        TipButton(TipButtonRole role, QWidget *parent) : QPushButton(parent)
        {
            setFixedSize(20, 20);
            my_role = role;
        }

        void paintEvent(QPaintEvent *) override
        {
            if (my_role == TipButton::NoButton) {
                return;
            }

            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            QColor color(150, 150, 150);

            QRect border(2, 2, 16, 16);
            painter.setPen(QPen(color, 2));
            painter.drawRoundedRect(border, 3, 3);

            if (my_role == TipButton::Close) {
                painter.setPen(QPen(color, 3));
                painter.drawLine(6, 6, 14, 14);
                painter.drawLine(6, 14, 14, 6);
            } else if (my_role == TipButton::Config) {
                painter.setPen(Qt::transparent);
                painter.setBrush(color);
                QPainterPath path;
                QPainterPath path2;
                path.addEllipse(4, 3, 12, 9);
                path.addRect(8, 3, 4, 4);
                path2.addRect(8, 11, 4, 5);

                path = path.united(path2).simplified();
                painter.drawPath(path);
            }
        }
    };

    /*
      Class BalloonTip : A balloon tip used to display messages
    */
    class BalloonTip : public QWidget
    {
        Q_OBJECT
        Q_PROPERTY(ArrowPosition my_arrowPos READ arrowPosition WRITE setArrowPosition)

    public:
        /* This enum determines in which corner will be displayed the arrow */
        enum ArrowPositions
        {
            TopLeft     = 0,
            TopRight    = 1,
            BottomLeft  = 2,
            BottomRight = 3,
            LeftTop     = 4
        };
        Q_DECLARE_FLAGS(ArrowPosition, ArrowPositions)
        Q_ENUMS(ArrowPositions)

    protected:
        QPixmap my_icon;
        QString my_text;
        QString my_title;
        QPoint my_pos;
        ArrowPosition my_arrowPos;
        QRect my_popupRect;
        QRect my_textRect;
        int my_duration;

        TipButton *my_closeButton;
        TipButton *my_configButton;

        void paintEvent(QPaintEvent *event) override;

        QRect relativePopupRect();
        QRect relativeTextRect();

        void enterEvent(QEvent *event) override;
        void leaveEvent(QEvent *event) override;
        void init();
        void defineArrowPosition();
        void createRects();
        bool eventFilter(QObject *, QEvent *) override;

    public:
        explicit BalloonTip(QString title, QString text, int duration = 2000, QWidget *parent = nullptr);
        explicit BalloonTip(
            const QPixmap &pix, QString title, QString text, int duration = 2000, QWidget *parent = nullptr);
        explicit BalloonTip(
            QStyle::StandardPixmap pix, QString title, QString text, int duration = 2000, QWidget *parent = nullptr);

        ~BalloonTip() override;
        ArrowPosition arrowPosition();
        void setArrowPosition(ArrowPosition pos);
        void move(QPoint pos);

    signals:
        void finished();

    public slots:
        void show();
        bool close();
    };

} // namespace Tooltips
#endif // BALLONTIP_H
