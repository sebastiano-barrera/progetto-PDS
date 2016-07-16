#ifndef CLIENTPROTOCOL_H
#define CLIENTPROTOCOL_H

#include <QObject>
#include <QVector>

#include "messagestream.h"
#include "applist.h"

class QTcpSocket;

namespace msgs {
    class AppList;
    class KeystrokeRequest;
}


class ClientProtocol : public QObject
{
    Q_OBJECT

    enum State {
        Init,
        AcceptingEvents,
        Stopped,
    };

    MessageStream msgStream_;
    QTcpSocket *client_;
    State state_;

public:
    explicit ClientProtocol(QTcpSocket *client = 0, QObject *parent = 0);

    inline QTcpSocket* client() const { return client_; }
    void setSocket(QTcpSocket *client);

    inline bool isStarted() const { return state_ != Stopped; }

    void sendRequest(const msgs::KeystrokeRequest&);

signals:
    void started();
    void stopped();
    void appListReceived(const App *app, size_t n_apps);
    void appCreated(const App&);
    void appDestroyed(App::Id);
    void appGotFocus(App::Id);

public slots:
    void start();
    void stop();
    void receiveMessage(const QByteArray&);
};

#endif // CLIENTPROTOCOL_H
