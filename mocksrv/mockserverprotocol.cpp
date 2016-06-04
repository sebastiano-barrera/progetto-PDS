#include "mockserverprotocol.h"

#include <QTcpSocket>
#include <QHostAddress>
#include "protocol.pb.h"

#include <iostream>

MockServerProtocol::MockServerProtocol(const QVector<WindowInfo> &windows,
                                       QTcpSocket *client,
                                       QObject *parent) :
    QObject(parent),
    windows_(windows),
    state_(Init),
    msgStream_(),
    client_(client),
    last_id_(0)
{
    Q_ASSERT (client_ != nullptr);

    msgStream_.setDevice(client_);

    timer_.setInterval(1500);
    timer_.setSingleShot(false);
    // timer_.start();

    connect(&timer_, &QTimer::timeout, this, &MockServerProtocol::onTimeout);
    connect(&msgStream_, &MessageStream::messageReceived,
            this, &MockServerProtocol::messageReceived);
    connect(client_, &QTcpSocket::disconnected, this, &MockServerProtocol::stop);
}

void MockServerProtocol::start()
{
    Q_ASSERT (state_ == Init);

    msgs::AppList msg;
    int n_windows = windows_.size() / 3;
    for (int i=0; i < n_windows; i++) {
        const WindowInfo& window = windows_[i];
        msgs::Application *item = msg.add_apps();
        item->set_id(i*10);
        item->set_name(window.name().toStdString());
    }

    last_id_ = n_windows;

    msgStream_.sendMessage(msg);
    state_ = WindowListPosted;
}

void MockServerProtocol::stop()
{
    timer_.stop();
    state_ = Init;
}

void MockServerProtocol::onTimeout()
{
    int index = qrand() % windows_.size();
    const WindowInfo& win = windows_[index];

    msgs::Event event;
    auto app = event.mutable_created();
    app->set_id(index);
    app->set_name(win.name().toStdString());

    msgStream_.sendMessage(event);
}

void MockServerProtocol::messageReceived(const QByteArray& msg)
{
    msgs::KeystrokeRequest req;
    req.ParseFromArray(msg.data(), msg.size());

    qDebug() << "received request: "
             << req.DebugString().c_str();
}
