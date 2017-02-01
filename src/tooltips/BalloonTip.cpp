#include "BalloonTip.h"

#include <QFontMetrics>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QPushButton>
#include <QTimer>
#include <QCloseEvent>
#include <QStyle>
#include <QIcon>

BalloonTip::BalloonTip( QStyle::StandardPixmap icon, QString title, QString text, int duration, QWidget* parent ) :
    QWidget( parent )
{
    my_closeButton = new TipButton( TipButton::Close, this );
    my_title = title;
    my_text = text;
    my_duration = duration;
    my_icon = qApp->style()->standardIcon( icon ).pixmap( QSize( 15, 15 ) );
    init();
}

BalloonTip::BalloonTip( QPixmap icon, QString title, QString text, int duration, QWidget* parent ) :
    QWidget( parent )
{
    my_closeButton = new TipButton( TipButton::Close, this );
    my_title = title;
    my_text = text;
    my_duration = duration;
    my_icon = icon.scaled( QSize( 15, 15 ), Qt::KeepAspectRatio );
    init();
}

BalloonTip::BalloonTip( QString title, QString text, int duration, QWidget* parent ) :
    QWidget( parent )
{
    my_closeButton = new TipButton( TipButton::Close, this );
    my_title = title;
    my_text = text;
    my_duration = duration;
    init();
}

void BalloonTip::init()
{
    setWindowFlags( Qt::FramelessWindowHint | Qt::ToolTip );
    setAttribute( Qt::WA_TranslucentBackground, true );

    createRects();

    //defineArrowPosition();
    setArrowPosition(my_arrowPos);

    connect(my_closeButton, SIGNAL(clicked()), this, SLOT(close()));

    if ( parentWidget() != 0 ) {
        parentWidget()->installEventFilter( this );
        QWidget* w = parentWidget()->parentWidget();
        while ( w != 0  ) {
            w->installEventFilter( this );
            w = w->parentWidget();
        }
    }

    setFixedSize( my_popupRect.size() + QSize( 60, 60 ) );
}

void BalloonTip::createRects()
{
    QFont font = this->font();

    // font for title
    font.setBold(true);
    font.setPixelSize(10);
    QFontMetrics metrics(font);

    // title rect
    QRect rect = metrics.boundingRect(QRect(10, 10, 500, 500), Qt::TextSingleLine, my_title);
    if(rect.width() < 100) {
        rect.setWidth(100);
    }

    // font for text
    font.setPixelSize(10);
    font.setBold(false);
    setFont(font);
    metrics = QFontMetrics(font);

    // text rect
    my_textRect = metrics.boundingRect(QRect(10, 30, rect.width() + 90, 500), Qt::TextWordWrap, my_text);
    if(my_textRect.width() < rect.width()) {
        my_textRect.setWidth(rect.width() + 90);
    }

    my_popupRect = QRect(0, 0, my_textRect.width() + 20, my_textRect.height() + 40);
}

void BalloonTip::defineArrowPosition()
{
    QSize desktopSize = QApplication::desktop()->size();
    QPoint pos = mapToGlobal(my_pos);

    if(pos.x() < desktopSize.width() / 2) {
        if(pos.y() < desktopSize.height() / 2) {
            my_arrowPos = TopLeft;
        } else {
            my_arrowPos = BottomLeft;
        }
    } else {
        if(pos.y() < desktopSize.height() / 2) {
            my_arrowPos = TopRight;
        } else {
            my_arrowPos = BottomRight;
        }
    }
}

BalloonTip::~BalloonTip()
{
    my_closeButton->deleteLater();
}

void BalloonTip::paintEvent(QPaintEvent * /*ev*/ )
{
    QPainter painter( this );

    painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );
    painter.setBrush( Qt::white );
    painter.setPen(QColor(0, 0, 0, 130));
    painter.setFont(this->font());

    QRect popupRect = relativePopupRect();
    QRect textRect = relativeTextRect();

    QPainterPath path;
    QPolygon arrowTriangle;

    switch(my_arrowPos)
    {
        case BottomLeft :
            arrowTriangle << QPoint(30, popupRect.height() + 60)
                          << QPoint(60, popupRect.height() + 30)
                          << QPoint(90, popupRect.height() + 30);
            break;
        case TopLeft :
            arrowTriangle << QPoint(30, 0)
                          << QPoint(60, 15)
                          << QPoint(90, 15);
            break;
        case BottomRight :
            arrowTriangle << QPoint(popupRect.width() - 30, popupRect.height() + 60)
                          << QPoint(popupRect.width() - 60, popupRect.height() + 30)
                          << QPoint(popupRect.width() - 90, popupRect.height() +30);
            break;
        case TopRight :
            arrowTriangle << QPoint(popupRect.width() - 30, 0)
                          << QPoint(popupRect.width() - 60, 30)
                          << QPoint(popupRect.width() - 90, 30);
            break;
        case LeftTop :
            arrowTriangle << QPoint(popupRect.left() - 10, popupRect.height() * 0.6)
                          << QPoint(popupRect.left()     , popupRect.height() * 0.6 + 5)
                          << QPoint(popupRect.left()     , popupRect.height() * 0.6 - 5);
            break;
    }

    path.addPolygon(arrowTriangle);
    path.addRoundedRect(popupRect, 3, 3);
    path = path.simplified();
    painter.drawPath(path);
    painter.setPen(QColor(20, 20, 20));
    painter.drawText(textRect, my_text);

    QFont font = this->font();
    font.setBold(true);
    font.setPixelSize(12);
    painter.setFont(font);
    painter.setPen(QColor(48, 159, 220));
    if (!my_icon.isNull()) {
        painter.drawText(textRect.topLeft() + QPoint(20, -10), my_title);
        painter.drawPixmap(textRect.topLeft() + QPoint(0, -22), my_icon);
    } else {
        painter.drawText(textRect.topLeft() + QPoint(5, -10), my_title);
    }
}

BalloonTip::ArrowPosition BalloonTip::arrowPosition()
{
    return my_arrowPos;
}

void BalloonTip::setArrowPosition(BalloonTip::ArrowPosition arrowPos)
{
    my_arrowPos = arrowPos;
    QRect r = relativePopupRect();
    my_closeButton->move(r.topRight() - QPoint(30, - 5));
}


QRect BalloonTip::relativePopupRect()
{
    QRect rect = my_popupRect;
    // move tooltip rect based on the arrowPosition
    if(my_arrowPos == TopLeft || TopRight) {
        // move the tooltip rect a bit down, to have Y space for the arrow/triangle
        rect.translate(0, 15);
    }
    if(my_arrowPos == LeftTop) {
        // move the tooltip rect a bit to right, to have Y space for the arrow/triangle
        rect.translate(15, 0);
    }
    return rect;
}

QRect BalloonTip::relativeTextRect()
{
    QRect rect = my_textRect;
    if(my_arrowPos == TopLeft || TopRight) {
        rect.translate(0, 15);
    }
    if(my_arrowPos == LeftTop) {
        rect.translate(15, 0);
    }
    return rect;
}

void BalloonTip::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
}

void BalloonTip::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
}

bool BalloonTip::close()
{
    //qDebug() << Q_FUNC_INFO << my_title << this;
    emit finished();
    return QWidget::close();
}

void BalloonTip::show()
{
    //qDebug() << Q_FUNC_INFO << my_title << this;
    QWidget::show();
}

bool BalloonTip::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);

    if(event->type() != QEvent::Move && event->type() != QEvent::Resize) {
        return false;
    }

    return false;
}

void BalloonTip::move( QPoint pos )
{
    QWidget::move( pos );
    switch( my_arrowPos )
    {
        case BottomLeft :
            pos.setY(pos.y() - my_popupRect.height() - 60);
        case TopLeft :
            pos.setX(pos.x() - 30 );
            break;
        case BottomRight :
            pos.setY(pos.y() - my_popupRect.height() - 60);
        case TopRight :
            pos.setX(pos.x() - my_popupRect.width() + 30);
            break;
        case LeftTop :
            pos.setX(pos.x() + 8);
            pos.setY(pos.y() - my_popupRect.height() * 0.63);
            break;
    }
    QWidget::move(pos);
    update();
}
