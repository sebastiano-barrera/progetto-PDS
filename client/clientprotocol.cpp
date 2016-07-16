#include "clientprotocol.h"
#include "protocol.pb.h"

#include <QTcpSocket>


ClientProtocol::ClientProtocol(QTcpSocket *client, QObject *parent) :
    QObject(parent),
    client_(nullptr),
    state_(Stopped)
{
    setSocket(client);
    connect(&msgStream_, &MessageStream::messageReceived, this, &ClientProtocol::receiveMessage);
}

void ClientProtocol::setSocket(QTcpSocket *client)
{
    if (client_ == client)
        return;

    if (client_) {
        client_->disconnect(this);
        stop();
    }

    client_ = client;
    msgStream_.setDevice(client_);

    if (client_) {
        connect(client_, &QTcpSocket::aboutToClose, this, &ClientProtocol::stop);

        if (client_->isOpen())
            start();
    }
}

void ClientProtocol::start()
{
    state_ = Init;
    emit started();
}

void ClientProtocol::stop()
{
    state_ = Stopped;
    emit stopped();
}

void ClientProtocol::receiveMessage(const QByteArray &msg)
{
    switch (state_) {
        case Init: {
            msgs::AppList appList;
            appList.ParseFromArray(msg.data(), msg.size());

            // some copying happens here. don't know how to avoid it "cleanly"
            QVector<App> apps;
            for (const msgs::Application& app : appList.apps())
                apps.append(App(app));

            emit appListReceived(apps.data(), apps.size());
            state_ = AcceptingEvents;
            break;
        }

        case AcceptingEvents:  {
            msgs::Event event;
            event.ParseFromArray(msg.data(), msg.size());
            if (event.has_created())
                emit appCreated(App(event.created()));
            if (event.has_destroyed())
                emit appDestroyed(event.destroyed().id());
            if (event.has_got_focus())
                emit appGotFocus(event.got_focus().id());
            break;
        }
    }
}

void ClientProtocol::sendRequest(const msgs::KeystrokeRequest &req)
{
    qDebug() << req.DebugString().c_str();
    std::string raw = req.SerializeAsString();
    msgStream_.sendMessage(raw.data(), raw.size());
}
