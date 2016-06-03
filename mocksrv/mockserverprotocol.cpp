#include "mockserverprotocol.h"

#include <QTcpSocket>
#include <QHostAddress>
#include "protocol.pb.h"

MockServerProtocol::MockServerProtocol(QTcpSocket *client,
                                       const QVector<WindowInfo> &windows,
                                       QObject *parent) :
    QObject(parent),
    state_(Init),
    client_(client),
    msgStream_(client_),
    windows_(windows)
{
    Q_ASSERT (client_ != nullptr);
    connect(&msgStream_, &MessageStream::messageReceived,
            this, &MockServerProtocol::messageReceived);
}

void MockServerProtocol::start()
{
    Q_ASSERT (state_ == Init);

    AppList msg;
    int id = 0;
    for (const WindowInfo& window : windows_) {
        Application *item = msg.add_apps();
        item->set_app_id(id++);
        item->set_name(window.name().toStdString());
    }

    qInfo() << "Sending message to " << client_->peerAddress().toString() << ": "
            << msg.DebugString().c_str();

    std::string encoded = msg.SerializeAsString();
    quint32 msg_len = encoded.size();
    client_->write((char*) &msg_len, sizeof(msg_len));
    client_->write(encoded.data(), encoded.size());
}

void MockServerProtocol::messageReceived(const QByteArray& msg)
{
}
