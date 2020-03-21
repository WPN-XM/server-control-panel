#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include "common.h"
#include "version.h"

#include <utility>

#include <QApplication>
#include <QDateTime>
#include <QPainter>
#include <QPicture>
#include <QProgressBar>
#include <QScreen>
#include <QSplashScreen>
#include <QStyleOptionProgressBar>

namespace ServerControlPanel
{
    class APP_CORE_EXPORT SplashScreen : public QSplashScreen
    {
        Q_OBJECT

    public:
        explicit SplashScreen(const QPixmap &pixmap = QPixmap(), Qt::WindowFlags f = nullptr);

    public slots:
        void setProgress(int value);
        void setMessage(QString msg, int progress);

    private:
        int progress    = 0;
        QString message = "Loading ..";

    protected:
        void drawContents(QPainter *painter) override;
    };
} // namespace ServerControlPanel

#endif // SPLASHSCREEN_H
