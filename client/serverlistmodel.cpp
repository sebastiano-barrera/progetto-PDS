#include "serverlistmodel.h"
#include "connection.h"

ServerListModel::ServerListModel(QObject *parent) :
    QAbstractTableModel(parent)
{ }

void ServerListModel::addConnection(Connection *conn)
{
    if (conn == nullptr)
        return;

    connect(conn, &Connection::destroyed,    this, [=]() { removeConnection(conn); });
    connect(&conn->socket(), &QAbstractSocket::stateChanged, this, [=]() { updateConnection(conn); });

    int index = conns_.size();
    beginInsertRows(QModelIndex(), index, index);
    conns_.append(conn);
    endInsertRows();
}

void ServerListModel::removeConnection(Connection *conn)
{
    int row = conns_.indexOf(conn);
    if (row == -1)
        return;

    beginRemoveRows(QModelIndex(), row, row);
    conns_.remove(row);
    endRemoveRows();
}

void ServerListModel::updateConnection(Connection *conn)
{
    int row = conns_.indexOf(conn);
    if (row == -1)
        return;

    int lastColumn = columnCount(QModelIndex()) - 1;
    QModelIndex topLeft = index(row, 0);
    QModelIndex bottomRight = index(row, lastColumn);
    dataChanged(topLeft, bottomRight);
}

int ServerListModel::columnCount(const QModelIndex &parent) const
{
    // parent should be the invisible root ( == the invalid index)
    return parent.isValid() ? 0 : 2;
}

int ServerListModel::rowCount(const QModelIndex &parent) const
{
    // parent should be the invisible root ( == the invalid index)
    if (parent.isValid())
        return 0;
    return conns_.size();
}

Connection* ServerListModel::_atIndex(const QModelIndex& index) const
{
    if (index.parent().isValid())
        return nullptr;

    int i = index.row();
    if (i >= conns_.size())
        return nullptr;
    return conns_[i];
}

QString sockStateMessage(QAbstractSocket::SocketState state)
{
    switch(state) {
    case QAbstractSocket::UnconnectedState:
        return "Not connected";
    case QAbstractSocket::HostLookupState:
        return "Looking up host...";
    case QAbstractSocket::ConnectingState:
        return "Connecting...";
    case QAbstractSocket::ConnectedState:
        return "Connected";
    case QAbstractSocket::ClosingState:
        return "Closing...";
    default:
        return "?";
    }
}

QVariant ServerListModel::data(const QModelIndex &index, int role) const
{
    const Connection* conn = atIndex(index);
    if (conn == nullptr)
        return QVariant();

    int col = index.column();
    if (col == 0 && role == Qt::DisplayRole) {
        QString addr = conn->endpointAddress();
        if (addr.isNull())
            return " - ";
        return addr;
    } else if (col == 1 && role == Qt::DisplayRole) {
        return sockStateMessage(conn->socket().state());
    }

    return QVariant();
}

QVariant ServerListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    if (section == 0)
        return "Address";
    else if (section == 1)
        return "State";

    return QVariant();
}
