#include "applist.h"

#include "protocol.pb.h"
#include "connection.h"
#include "app.h"


AppList::AppList(QObject *parent) :
    QAbstractTableModel(parent),
    numConns_(0)
{
    updateTimer_.setInterval(200);
    updateTimer_.setSingleShot(false);
    connect(&updateTimer_, &QTimer::timeout, this, &AppList::focusTimeColumnChanged);
}

const App* AppList::atIndex(const QModelIndex& index) const
{
    if (index.parent().isValid())
        return nullptr;

    if (index.row() >= order_.size())
        return nullptr;
    return order_.at(index.row());
}

int AppList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())  // parent != invisible root
        return 0;

    return order_.size();
}

int AppList::columnCount(const QModelIndex &parent) const
{
    // `parent` should be the index of the invisible root
    return parent.isValid() ? 0 : 5;
}

QVariant AppList::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid())
        return QVariant();

    const auto* app = atIndex(index);
    int col = index.column();
    if (app == nullptr || col >= 5)
        return QVariant();

    if (col == 0 && role == Qt::DisplayRole) {
        return app->parentConn()->endpointAddress();

    } else if (col == 1) {
        if (role == Qt::DisplayRole)
            return app->processPath().fileName();
        else if (role == Qt::DecorationRole)
            return app->icon();

    } else if (col == 2) {
        if (role == Qt::DisplayRole)
            return app->isFocused() ? QString("●") : QString();
        else if (role == Qt::TextAlignmentRole)
            return Qt::AlignHCenter;

    } else if (col == 3 && role == Qt::DisplayRole) {
        auto timeConnected = app->parentConn()->timeConnectedMS();
        if (timeConnected > 0) {
            double timePerc = (double) app->focusTimeMS() / timeConnected * 100;
            return QString("%1 %").arg(timePerc, 0, 'f', 2);
        } else {
            return  " - ";
        }

    } else if (col == 4 && role == Qt::DisplayRole) {
        auto title = app->title();
        return title.size() == 0 ? " - " : title;
    }

    return QVariant();
}

QVariant AppList::headerData(int section, Qt::Orientation orientation, int role) const
{
    static const char* const headers[] = {
        "Server",
        "Process",
        "Focused",
        "Time focused (since connection)",
        "Window title",
    };
    static const size_t n_headers = sizeof(headers)/sizeof(headers[0]);

    if (orientation != Qt::Horizontal
            || role != Qt::DisplayRole
            || section >= n_headers)
        return {};

    return headers[section];
}

void AppList::addApp(const App *app)
{
    if (app == nullptr)
        return;

    connect(app, &QObject::destroyed, this, [=]() { removeApp(app); });

    int index = order_.size();
    beginInsertRows(QModelIndex(), index, index);
    order_.append(app);
    endInsertRows();
}

void AppList::removeApp(const App *app)
{
    // here, the App object may be about to be destroyed
    int row = order_.indexOf(app);
    if (row == -1)
        return;
    beginRemoveRows(QModelIndex(), row, row);
    order_.removeAt(row);
    endRemoveRows();
}

void AppList::addConnection(const Connection *conn)
{
    if (conn == nullptr)
        return;

    auto appList = conn->apps();
    for (const App* app : appList)
        addApp(app);

    connect(conn, &Connection::appCreated, this, &AppList::addApp);
    connect(conn, &Connection::destroyed, this, &AppList::decreaseConnCount);

    updateTimer_.start();
}

void AppList::decreaseConnCount()
{
    if (numConns_ == 0)
        return;
    numConns_--;
    if (numConns_ == 0)
        updateTimer_.stop();
}


void AppList::focusTimeColumnChanged()
{
    QModelIndex root;
    QModelIndex topLeft = index(0, 0);
    QModelIndex bottomRight = index(rowCount(root) - 1,  columnCount(root) - 1);
    dataChanged(topLeft, bottomRight);
}
