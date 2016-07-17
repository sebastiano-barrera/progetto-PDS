#ifndef CLIENTPROTOCOL_H
#define CLIENTPROTOCOL_H

#include <QObject>
#include <QVector>

#include <unordered_map>
#include <memory>

#include "messagestream.h"
#include "applist.h"
#include "protocol.pb.h"

class QTcpSocket;


class ClientProtocol : public QObject
{
    Q_OBJECT

public:
    typedef quint32 RequestId;
    enum { INVALID_REQUEST = (quint32) -1 };

    explicit ClientProtocol(QTcpSocket *client = 0, QObject *parent = 0);

    inline QTcpSocket* client() const { return client_; }
    void setSocket(QTcpSocket *client);

    inline bool isStarted() const { return state_ != Stopped; }

    inline int pendingRequestsCount() const { return pending_reqs_.size(); }
    RequestId sendRequest(std::unique_ptr<msgs::KeystrokeRequest> req);

signals:
    void started();
    void stopped();
    void appListReceived(const App *app, size_t n_apps);
    void appCreated(const App&);
    void appDestroyed(App::Id);
    void appGotFocus(App::Id);
    void responseReceived(const msgs::KeystrokeRequest& req,
                          const msgs::Response& res);

public slots:
    void start();
    void stop();
    void hardStop();
    void receiveMessage(const QByteArray&);

private:
    enum State {
        Init,
        AcceptingEvents,
        Stopped,
    };

    MessageStream msgStream_;
    QTcpSocket *client_;
    State state_;

    RequestId last_id_;
    std::unordered_map<
        RequestId,
        std::unique_ptr<msgs::KeystrokeRequest>> pending_reqs_;

    RequestId nextId();
};

#endif // CLIENTPROTOCOL_H
