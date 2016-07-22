#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QElapsedTimer>
#include <QHostAddress>

#include <unordered_map>

#include "clientprotocol.h"

namespace msgs {
    class KeystrokeRequest;
    class Application;
    class Response;
}

class App;


class Connection : public QObject
{
    Q_OBJECT

    QTcpSocket sock_;
    ClientProtocol proto_;

    // caching from sock_.peerAddr() and sock_.peerPort()
    QHostAddress addr_;
    quint32 port_;

    // using a map keeps the keys ordered, which allows us to predict
    // the order of the rows in the model (AppListModel).

    // The `App` must be stored through a raw pointer, so we can avoid deleting
    // it directly, instead letting Qt's main loop delete it (by calling
    // deleteLater()). This way, handlers for signals such as destroyed(), that
    // other objects use to correctly update themselves, can still access the
    // object being deleted correctly.
    std::unordered_map<ClientProtocol::AppId, App*> apps_;
    ClientProtocol::AppId focusedApp_;

    QElapsedTimer connectionTimer_;

public:
    explicit Connection(QObject *parent = 0);
    virtual ~Connection();

    // The socket is freely accessible from the outside:
    // all events are correctly handled from within the Connection object.
    inline QAbstractSocket&         socket()        { return sock_; }
    inline const QAbstractSocket&   socket() const  { return sock_; }

    // Like socket().peerAddress() and socket().peerPort(), but the
    // information isn't lost after the socket disconnects
    inline const QHostAddress& hostAddress() const { return addr_; }
    inline quint16             port() const        { return port_; }
    QString endpointAddress() const;

    // Return elapsed time since connection, in ms
    quint64 timeConnectedMS() const;

    inline unsigned appsCount() const { return apps_.size(); }
    QVector<const App*> apps() const;
    const App* appById(ClientProtocol::AppId) const;
    const App* focusedApp() const;

    inline ClientProtocol::RequestId sendRequest(std::unique_ptr<msgs::KeystrokeRequest> req) {
        return proto_.sendRequest(std::move(req));
    }
    inline int pendingRequestsCount() {
        return proto_.pendingRequestsCount();
    }

signals:
    void appCreated(const App*);
    void responseReceived(Connection *conn,
                          const msgs::KeystrokeRequest &req,
                          const msgs::Response& res);

private slots:
    void setAppList(const std::vector<const msgs::Application *> &appMsgs);
    void createApp(const msgs::Application&);
    void destroyApp(ClientProtocol::AppId);
    void setFocusedApp(ClientProtocol::AppId);
    void handleResponse(const msgs::KeystrokeRequest &req, const msgs::Response&);
    void reset();
    void resetConnectionTime();
    void socketStateChanged();
};

#endif // CONNECTION_H
