#ifndef CLIENTPROTOCOL_H
#define CLIENTPROTOCOL_H

#include <QObject>
#include "messagestream.h"

class QTcpSocket;
class AppList;

class ClientProtocol : public QObject
{
    Q_OBJECT

    enum State {
        Init,
        AwaitingRequest,
    };

    MessageStream msgStream_;
    QTcpSocket *client_;
    State state_;

public:
    explicit ClientProtocol(QTcpSocket *client = 0, QObject *parent = 0);

    inline QTcpSocket* client() const { return client_; }
    void setClient(QTcpSocket *client);

signals:
    void appListReceived(const AppList&);

public slots:
    void start();
    void receiveMessage(const QByteArray&);
};

#endif // CLIENTPROTOCOL_H
