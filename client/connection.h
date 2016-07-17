#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QElapsedTimer>
#include <QHostAddress>

#include "clientprotocol.h"

#include <unordered_map>

namespace msgs {
class Application;
}

class Connection;


// `App` objects are small, lightweight data structures about a single
// application window in a remote server.
//
// NOTE: AppList depends on the address of App objects: if an App object is copied,
//       AppLists will treat them as distinct objects (despite having the same
//       parent Connection and id). This will be fixed, but for now it's a sad fact of life.
class App : public QObject
{
    Q_OBJECT
public:
    typedef ClientProtocol::AppId Id;
    enum {
        INVALID_ID = (Id) -1
    };

    App(Connection *conn, const msgs::Application& msg, QObject *parent = 0);

    inline bool isValid() const { return valid_; }
    inline Id id() const { return id_; }
    inline const QString& name() const { return name_; }
    inline Connection* parentConn() const { return parentConn_; }

    void setFocused(bool focused);
    inline bool isFocused() const { return focused_; }
    quint64 focusTimeMS() const;

private:
    bool valid_;
    Id id_;
    QString name_;
    Connection *parentConn_;
    bool focused_;
    QElapsedTimer focusTimer_;
    quint64 totalFocusTime_;
};


class Connection : public QObject
{
    Q_OBJECT

    QTcpSocket sock_;
    ClientProtocol proto_;

    // using a map keeps the keys ordered, which allows us to predict
    // the order of the rows in the model (AppListModel)
    std::unordered_map<App::Id, std::unique_ptr<App>> apps_;
    QElapsedTimer connectionTimer_;
    App::Id focusedApp_;

public:
    explicit Connection(QObject *parent = 0);

    // The socket is freely accessible from the outside:
    // all events are correctly handled from within the Connection object.
    inline QAbstractSocket& socket()      { return sock_; }
    const QAbstractSocket& socket() const { return sock_; }

    // Return elapsed time since connection, in ms
    quint64 timeConnectedMS() const;

    inline unsigned appsCount() const { return apps_.size(); }
    QVector<const App*> apps() const;
    const App* focusedApp() const;

    void sendRequest(const msgs::KeystrokeRequest&);

signals:
    void stateChanged();
    void appCreated(const App*);

private slots:
    void setAppList(const std::vector<const msgs::Application *> &appMsgs);
    void createApp(const msgs::Application&);
    void destroyApp(ClientProtocol::AppId);
    void setFocusedApp(ClientProtocol::AppId);
    void reset();
    void resetConnectionTime();
};

#endif // CONNECTION_H
