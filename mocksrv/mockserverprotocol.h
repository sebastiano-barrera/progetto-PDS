#ifndef MOCKSERVERPROTOCOL_H
#define MOCKSERVERPROTOCOL_H

#include <QObject>
#include <QTimer>
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

    const QVector<WindowInfo> &windows_;

    State state_;
    MessageStream msgStream_;
    QTcpSocket *client_;

    QTimer timer_;
    int last_id_;

public:
    MockServerProtocol(const QVector<WindowInfo> &windows,
                       QTcpSocket *client = 0,
                       QObject *parent = 0);

public slots:
    void start();
    void messageReceived(const QByteArray& msg);
    void stop();

private slots:
    void onTimeout();
};

#endif // MOCKSERVERPROTOCOL_H
