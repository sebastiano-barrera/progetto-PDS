#include "app.h"

#include <QPixmap>
#include <QImage>
#include <QDebug>


App::App(Connection *conn, const msgs::Application &msg, QObject *parent) :
    QObject(parent),
    id_(msg.id()),
    parentConn_(conn),
    focused_(false),
    totalFocusTime_(0)
{
    assert (conn != nullptr);
    assert (id_ != INVALID_ID);


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

App& App::operator=(App&& rhs)
{
    std::swap(valid_, rhs.valid_);
    std::swap(id_, rhs.id_);
    std::swap(name_, rhs.name_);
    if (parentConn_ != rhs.parentConn_) {
        std::swap(parentConn_, rhs.parentConn_);
        std::swap(focusTimer_, rhs.focusTimer_);
        std::swap(totalFocusTime_, rhs.totalFocusTime_);
    }
    setFocused(rhs.focused_);
    return *this;
}

void App::setFocused(bool focused)
{
    if (focused_ == focused)
        return;

    focused_ = focused;
    if (focused_) {
        if (valid_)
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

