#ifndef MOCKSERVERPROTOCOL_H
#define MOCKSERVERPROTOCOL_H

#include <QObject>

#include "messagestream.h"
#include "windowinfo.h"

class QTcpSocket;
class QByteArray;


class MockServerProtocol : public QObject {
    Q_OBJECT

    enum State {
        Init,
        WindowListPosted,
    };

    State state_;
    QTcpSocket *client_;
    MessageStream msgStream_;
    const QVector<WindowInfo> &windows_;

public:
    MockServerProtocol(QTcpSocket *client,
                       const QVector<WindowInfo> &windows,
                       QObject *parent = 0);

public slots:
    void start();
    void messageReceived(const QByteArray& msg);
};

#endif // MOCKSERVERPROTOCOL_H
