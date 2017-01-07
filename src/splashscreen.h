#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include "version.h"

#include <QApplication>
#include <QPainter>
#include <QDateTime>
#include <QPicture>
#include <QScreen>
#include <QStyleOptionProgressBar>
#include <QProgressBar>
#include <QSplashScreen>

namespace ServerControlPanel
{
    class SplashScreen : public QSplashScreen
    {
        Q_OBJECT

        public:
            explicit SplashScreen(const QPixmap &pixmap = QPixmap(), Qt::WindowFlags f = 0);

        public slots:
            void setProgress(int value);
            void setMessage(QString msg, int progress);

        private:
            int progress;
            QString message = "Loading ..";

        protected:
            void drawContents(QPainter *painter);
    };
}

#endif // SPLASHSCREEN_H
