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
  setWindowFlags(  Qt::FramelessWindowHint | Qt::ToolTip );
  setAttribute( Qt::WA_TranslucentBackground, true );

  createAnimation();
  createRects();
  defineArrowPosition();

  setArrowPosition( my_arrowPos );
  setFixedSize( my_popupRect.width() + 60, my_popupRect.height() + 60 );
  connect( my_closeButton, SIGNAL(clicked()), this, SLOT(close()) );

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

void BalloonTip::createAnimation() {
  my_animation = new QPropertyAnimation( this, "windowOpacity" );
  my_animation->setStartValue( 0.9 );
  my_animation->setEndValue( 0.0 );
  my_animation->setDuration( my_duration );
  my_animation->setEasingCurve( QEasingCurve::InCubic );
  connect( my_animation, SIGNAL(finished()), this, SLOT(close()) );
}

void BalloonTip::createRects() {
  QFont font = this->font();
  font.setBold( true );
  font.setPixelSize( 12 );
  QFontMetrics metrics( font );
  QRect rect = metrics.boundingRect( QRect( 10, 10, 500, 500 ), Qt::TextSingleLine, my_title );
  if ( rect.width() < 100 ) {
    rect.setWidth( 100 );
  }
  font.setPixelSize( 12 );
  font.setBold( false );
  setFont( font );
  metrics = QFontMetrics( font );

  my_textRect = metrics.boundingRect( QRect( 10, 30, rect.width() + 90, 500), Qt::TextWordWrap, my_text );
  if ( my_textRect.width() < rect.width() ) {
    my_textRect.setWidth( rect.width() + 90 );
  }

  my_popupRect = QRect( 0, 0, my_textRect.width() + 20, my_textRect.height() + 40 );
}

void BalloonTip::defineArrowPosition() {
  QSize desktopSize = QApplication::desktop()->size();
  QPoint pos = mapToGlobal( my_pos );
  if ( pos.x() < desktopSize.width() / 2 ) {
    if ( pos.y() < desktopSize.height() / 2 ) {
      my_arrowPos = TopLeft;
    } else {
      my_arrowPos = BottomLeft;
    }
  } else {
    if ( pos.y() < desktopSize.height() / 2 ) {
      my_arrowPos = TopRight;
    } else {
      my_arrowPos = BottomRight;
    }
  }
}

BalloonTip::~BalloonTip() {
  my_closeButton->deleteLater();
}

void BalloonTip::paintEvent(QPaintEvent * /*ev*/ ) {
  QPainter painter( this );

  painter.setRenderHints( QPainter::Antialiasing | QPainter::TextAntialiasing );
  painter.setBrush( Qt::white );
  painter.setPen( QColor( 50, 50, 50 , 90 ) );
  painter.setFont( this->font() );

  QRect popupRect = relativePopupRect();
  QRect textRect = relativeTextRect();


  QPainterPath path;
  QPolygon p;

  switch( my_arrowPos ) {
  case BottomLeft :
    p << QPoint( 30, popupRect.height() + 60 )
      << QPoint( 60, popupRect.height() + 30 )
      << QPoint( 90, popupRect.height() + 30 );
  break;
  case TopLeft :
    p << QPoint( 30, 0 ) << QPoint( 60, 30 ) << QPoint( 90, 30 );
  break;
  case BottomRight :
    p << QPoint( popupRect.width() - 30, popupRect.height() + 60 )
      << QPoint( popupRect.width() - 60, popupRect.height() + 30 )
      << QPoint( popupRect.width() - 90, popupRect.height() +30 );
  break;
  case TopRight :
    p << QPoint( popupRect.width() - 30, 0 )
      << QPoint( popupRect.width() - 60, 30 )
      << QPoint( popupRect.width() - 90, 30 );
  break;
  }

  path.addPolygon( p );
  path.addRoundedRect( popupRect, 5, 5 );
  path = path.simplified();
  painter.drawPath( path );
  painter.setPen( QColor(20, 20, 20 ) );
  painter.drawText( textRect, my_text );

  QFont font = this->font();
  font.setBold( true );
  font.setPixelSize( 12 );
  painter.setFont( font );
  painter.setPen( QColor( 48, 159, 220 ) );
  if ( !my_icon.isNull() ) {
    painter.drawText( textRect.topLeft() + QPoint( 20, -10 ), my_title );
    painter.drawPixmap( textRect.topLeft() + QPoint( 0, -22 ), my_icon );
  } else {
    painter.drawText( textRect.topLeft() + QPoint( 5, -10 ), my_title );
  }
}

BalloonTip::ArrowPosition BalloonTip::arrowPosition() {
  return my_arrowPos;
}

void BalloonTip::setArrowPosition( BalloonTip::ArrowPosition arrowPos ) {
  my_arrowPos = arrowPos;
  QRect r = relativePopupRect();
  my_closeButton->move( r.topRight() - QPoint( 30, - 5 ) );
}


QRect BalloonTip::relativePopupRect() {
  QRect rect = my_popupRect;
  if ( my_arrowPos == TopLeft || TopRight ) {
    rect.translate( 0, 30 );
  }
  return rect;
}

QRect BalloonTip::relativeTextRect() {
  QRect rect = my_textRect;
  if ( my_arrowPos == TopLeft || TopRight ) {
    rect.translate( 0, 30 );
  }
  return rect;
}

void BalloonTip::enterEvent( QEvent* ev ) {
//  setWindowOpacity( 0.9 );
//  my_animation->stop();
  QWidget::enterEvent( ev );
}

void BalloonTip::leaveEvent( QEvent* ev ) {
//  if ( my_animated == true ) {
//    my_animation->start();
//  }
  QWidget::leaveEvent( ev );
}

bool BalloonTip::close() {
  qDebug() << Q_FUNC_INFO << my_title << this;
  my_animation->stop();
  my_animated = false;
  emit finished();
  return QWidget::close();
}

void BalloonTip::show() {
  qDebug() << Q_FUNC_INFO << my_title << this;
  QWidget::show();
//  my_animation->stop();
//  if ( my_duration > -1 ) {
//    my_animated = true;
//  } else {
//    my_animated = false;
//  }
//  if ( my_animated == true ) {
//    my_animation->start();
//  }
}

bool BalloonTip::eventFilter(QObject *object, QEvent *event)
{
  Q_UNUSED(object);

  if (event->type() != QEvent::Move &&
      event->type() != QEvent::Resize) {
    return false;
  }

  my_animation->start();

  /*
  QPoint pos = parentWidget()->pos();
  pos += QPoint( parentWidget()->width() / 2, parentWidget()->height() / 2 );
  pos =  parentWidget()->mapToGlobal( pos );
  move( pos );*/
  return false;
}

void BalloonTip::move( QPoint pos ) {
  QWidget::move( pos );
  defineArrowPosition();
  switch( my_arrowPos ) {
  case BottomLeft :
    pos.setY( pos.y() - my_popupRect.height() - 60 );
  case TopLeft :
    pos.setX( pos.x() - 30 );
  break;
  case BottomRight :
    pos.setY( pos.y() - my_popupRect.height() - 60 );
  case TopRight :
    pos.setX( pos.x() - my_popupRect.width() + 30 );
  break;
  }
  QWidget::move( pos );
  update();
}
