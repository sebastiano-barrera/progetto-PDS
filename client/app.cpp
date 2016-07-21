#include "app.h"

#include <QPixmap>
#include <QImage>
#include <QDebug>


App::App(Connection *conn, const msgs::Application &msg, QObject *parent) :
    QObject(parent),
    parentConn_(conn)
{
    assert (conn != nullptr);
    resetFromMessage(msg);
}

void App::resetFromMessage(const msgs::Application &msg)
{
    assert (msg.id() != INVALID_ID);

    id_ = msg.id();
    focused_ = false;
    totalFocusTime_ = 0;

    if (msg.has_win_title())
        title_ = QString::fromStdString(msg.win_title());

    if (msg.has_name())
        processPath_ = QFileInfo(QString::fromStdString(msg.name()));

    if (msg.has_icon()) {
        auto& icon = msg.icon();
        QImage image(reinterpret_cast<const uchar*>(icon.pixels().data()),
                     icon.width(), icon.height(),
                     QImage::Format_ARGB32);
        icon_ = QPixmap::fromImage(image.scaledToHeight(16));
    }
}

void App::setFocused(bool focused)
{
    if (focused_ == focused)
        return;

    focused_ = focused;
    if (focused_) {
        focusTimer_.start();
    } else {
        if (focusTimer_.isValid()) {
            totalFocusTime_ += focusTimer_.elapsed();
            focusTimer_.invalidate();
        }
    }
}

quint64 App::focusTimeMS() const
{
    quint64 elapTime = totalFocusTime_;
    if (focused_) {
        assert (focusTimer_.isValid());
        elapTime += focusTimer_.elapsed();
    }
    return elapTime;
}

