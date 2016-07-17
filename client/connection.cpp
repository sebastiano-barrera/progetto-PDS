#include "connection.h"
#include "protocol.pb.h"

#include <cassert>


//  App

App::App(Connection *conn, const msgs::Application &msg, QObject *parent) :
    QObject(parent),
    valid_(true),
    id_(msg.id()),
    name_(QString::fromStdString(msg.name())),
    parentConn_(conn),
    focused_(false),
    totalFocusTime_(0)
{
    assert (conn != nullptr);
    assert (id_ != INVALID_ID);
}

App& App::operator=(App&& rhs)
{
    std::swap(valid_, rhs.valid_);
    std::swap(id_, rhs.id_);
    std::swap(name_, rhs.name_);
    if (parentConn_ != rhs.parentConn_) {
        std::swap(parentConn_, rhs.parentConn_);
        std::swap(focusTimer_, rhs.focusTimer_);
        std::swap(totalFocusTime_, rhs.totalFocusTime_);
    }
    setFocused(rhs.focused_);
    return *this;
}

void App::setFocused(bool focused)
{
    if (focused_ == focused)
        return;

    focused_ = focused;
    if (focused_) {
        if (valid_)
            focusTimer_.start();
    } else {
        if (focusTimer_.isValid()) {
            totalFocusTime_ += focusTimer_.elapsed();
            focusTimer_.invalidate();
        }
    }
}

quint64 App::focusTimeMS() const
{
    quint64 elapTime = totalFocusTime_;
    if (focused_) {
        assert (focusTimer_.isValid());
        elapTime += focusTimer_.elapsed();
    }
    return elapTime;
}


//  Connection

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

void Connection::reset()
{
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
    auto iter = apps_.find(msg.id());
    if (iter == apps_.end()) {
        // NOTE: The `App` must be created with parent = NULL
        // The unordered_map & unique_ptr classes will perform the destruction
        // correctly without Qt's object hierarchy information.
        // If parent was != NULL, the object could potentially be destroyed twice
        // (once by unique_ptr and once by destructor we inherit by QObject)
        auto app = std::make_unique<App>(this, msg);
        auto appId = app->id();
        apps_.emplace(appId, std::move(app));
        emit appCreated(apps_[appId].get());
    } else {
        *iter->second = App {this, msg};
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
    return iter->second.get();
}

QVector<const App*> Connection::apps() const
{
    QVector<const App*> apps;
    for (const auto& itemPair : apps_)
        apps.append(itemPair.second.get());
    return apps;
}

quint64 Connection::timeConnectedMS() const
{
    if (sock_.state() == QAbstractSocket::ConnectedState)
        return connectionTimer_.elapsed();
    return 0;
}

void Connection::sendRequest(const msgs::KeystrokeRequest &req)
{
    proto_.sendRequest(req);
}

void Connection::socketStateChanged()
{
    // cache peer address and port, so we don't lose that information after disconnecting
    if (sock_.state() == QAbstractSocket::ConnectedState) {
        addr_ = sock_.peerAddress();
        port_ = sock_.peerPort();
    }
}
