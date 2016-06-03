#include "clientprotocol.h"
#include <QTcpSocket>

#include "protocol.pb.h"

ClientProtocol::ClientProtocol(QTcpSocket *client, QObject *parent) :
    QObject(parent),
    client_(nullptr),
    state_(Init)
{
    setClient(client);
    connect(&msgStream_, &MessageStream::messageReceived, this, &ClientProtocol::receiveMessage);
}

void ClientProtocol::setClient(QTcpSocket *client)
{
    if (client_ == client)
        return;

    // if (client_) { ... }

    client_ = client;
    msgStream_.setDevice(client_);

    if (client_)
        start();
}

void ClientProtocol::start()
{
    state_ = Init;
}

void ClientProtocol::receiveMessage(const QByteArray &msg)
{
    switch (state_) {
    case Init:
        {
            msgs::AppList appList;
            appList.ParseFromArray(msg.data(), msg.size());
            emit appListReceived(appList);
        }
        break;

    case AwaitingRequest:
        qWarning() << "ServerProtocol: no idea what to do with a message\n";
        break;
    }
}
