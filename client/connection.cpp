#include "connection.h"
#include "protocol.pb.h"
#include "app.h"

#include <cassert>


Connection::Connection(QObject *parent) :
    QObject(parent),
    focusedApp_(App::INVALID_ID)
{
    proto_.setSocket(&sock_);

    connect(&sock_, &QTcpSocket::connected, &proto_, &ClientProtocol::start);
    connect(&sock_, &QTcpSocket::connected, this, &Connection::resetConnectionTime);
    connect(&sock_, &QTcpSocket::stateChanged, this, &Connection::socketStateChanged);

    connect(&proto_, &ClientProtocol::appCreated, this, &Connection::createApp);
    connect(&proto_, &ClientProtocol::appDestroyed, this, &Connection::destroyApp);
    connect(&proto_, &ClientProtocol::appGotFocus, this, &Connection::setFocusedApp);
    connect(&proto_, &ClientProtocol::appListReceived, this, &Connection::setAppList);
    connect(&proto_, &ClientProtocol::responseReceived, this, &Connection::handleResponse);

    connect(&proto_, &ClientProtocol::stopped, this, &Connection::reset);

//    // need to do this to disambiguate with the `error()` getter
//    auto sig_error = static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error);
//    connect(sock_, sig_error, this, &Connection::error);
}

Connection::~Connection()
{
    qWarning() << "Connection deleted";
}

void Connection::resetConnectionTime()
{
    connectionTimer_.restart();
    assert (connectionTimer_.isValid());
}

QString Connection::endpointAddress() const
{
    if (addr_.isNull())
        return QString();
    return QString("%1:%2").arg(addr_.toString()).arg(port_);
}

void Connection::reset()
{
    for (auto& item : apps_)
        item.second->deleteLater();
    apps_.clear();
}

void Connection::setAppList(const std::vector<const msgs::Application *> &appMsgs)
{
    reset();
    for (const auto* appMsg : appMsgs)
        createApp(*appMsg);
}

void Connection::createApp(const msgs::Application &msg)
{
    qWarning("Got app:\n%d\t%s\t%s\n\n",
             msg.id(), msg.name().c_str(), msg.win_title().c_str());
    auto iter = apps_.find(msg.id());
    if (iter == apps_.end()) {
        // NOTE: The `App` must be created with parent = NULL
        // The unordered_map & unique_ptr classes will perform the destruction
        // correctly without Qt's object hierarchy information.
        // If parent was != NULL, the object could potentially be destroyed twice
        // (once by unique_ptr and once by destructor we inherit by QObject)
        auto app = new App(this, msg);
        auto appId = app->id();
        apps_.emplace(appId, app);
        emit appCreated(apps_[appId]);
    } else {
        iter->second->resetFromMessage(msg);
    }
}

void Connection::destroyApp(ClientProtocol::AppId appId)
{
    // This will cause the `App` object to be deleted;
    // the AppList model will handle this event by removing the row
    // corresponding to the deleted `App`.
    apps_.erase(appId);
}

void Connection::setFocusedApp(ClientProtocol::AppId appId)
{
    auto iter = apps_.find(focusedApp_);
    if (iter != apps_.end())
        iter->second->setFocused(false);

    focusedApp_ = appId;

    iter = apps_.find(appId);
    if (iter != apps_.end())
        iter->second->setFocused(true);
    else
        focusedApp_ = App::INVALID_ID;
}

const App* Connection::focusedApp() const
{
    auto iter = apps_.find(focusedApp_);
    if (iter == apps_.end())
        return nullptr;
    return iter->second;
}

QVector<const App*> Connection::apps() const
{
    QVector<const App*> apps;
    for (const auto& itemPair : apps_)
        apps.append(itemPair.second);
    return apps;
}

quint64 Connection::timeConnectedMS() const
{
    if (sock_.state() == QAbstractSocket::ConnectedState)
        return connectionTimer_.elapsed();
    return 0;
}

void Connection::handleResponse(const msgs::KeystrokeRequest& req,
                                const msgs::Response &res)
{
    emit responseReceived(this, req, res);
}

void Connection::socketStateChanged()
{
    // cache peer address and port, so we don't lose that information after disconnecting
    if (sock_.state() == QAbstractSocket::ConnectedState) {
        addr_ = sock_.peerAddress();
        port_ = sock_.peerPort();
    }
}

const App* Connection::appById(ClientProtocol::AppId appId) const
{
    auto iter = apps_.find(appId);
    if (iter == apps_.end())
        return nullptr;

    return iter->second;
}
