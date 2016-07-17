#ifndef SERVERLISTMODEL_H
#define SERVERLISTMODEL_H

#include <QAbstractTableModel>

class Connection;


class ServerListModel : public QAbstractTableModel
{
    QVector<Connection*> conns_;

public:
    ServerListModel(QObject *parent = nullptr);

    inline Connection* atIndex(const QModelIndex &index) {
        return _atIndex(index);
    }

    inline const Connection* atIndex(const QModelIndex &index) const {
        return const_cast<const Connection*>(_atIndex(index));
    }

    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    void addConnection(Connection *conn);
    void updateConnection(Connection *conn);

private:
    void removeConnection(Connection*);
    Connection* _atIndex(const QModelIndex &index) const;
};

#endif // SERVERLISTMODEL_H
