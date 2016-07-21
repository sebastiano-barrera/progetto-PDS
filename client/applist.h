#ifndef APPLIST_H
#define APPLIST_H

#include <QAbstractTableModel>
#include <QString>
#include <QFileInfo>
#include <QMap>
#include <QElapsedTimer>
#include <QTimer>
#include <QPixmap>

namespace msgs {
    class AppList;
    class Application;
}

class App;
class Connection;


class AppList : public QAbstractTableModel
{
    Q_OBJECT

    QTimer updateTimer_;
    QVector<const App*> order_;
    unsigned numConns_;

public:
    explicit AppList(QObject *parent = 0);

    const App* atIndex(const QModelIndex& index) const;
    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    void addApp(const App *app);
    void addConnection(const Connection *);

private slots:
    void removeApp(const App *app);
    void decreaseConnCount();
    void focusTimeColumnChanged();
};

#endif // APPLIST_H
