#include "applist.h"
#include "protocol.pb.h"

#include <QDebug>


App::App()
    : valid_(false)  // the rest is default-constructed
    { }

App::App(const msgs::Application &msg) :
    valid_(true),
    id_(msg.id()),
    name_(QString::fromStdString(msg.name()))
    { }


AppList::AppList(QObject *parent) :
    QAbstractListModel(parent)
    { }

const App* AppList::atIndex(const QModelIndex& index) const
{
    if (index.parent().isValid())
        return nullptr;

    auto id = apps_.keys().value(index.row(), -1);
    return &apps_.find(id).value();
}

int AppList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())  // parent != invisible root
        return 0;

    return apps_.size();
}

QVariant AppList::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid())
        return QVariant();

    const auto* app = atIndex(index);
    if (app == nullptr)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:  {
        // Only for MSVC (tested ver. 19.0):
        //   here, the statement
        //     return app->name();
        //   triggers a runtime exception (a write at a small memory address).
        //   looks like a compiler bug.
        auto text = QString("%1 - %2").arg(app->id()).arg(app->name());
        return text;
    }
    default:
        return QVariant();
    }
}

void AppList::replaceAll(const App *apps, size_t n_apps)
{
    replaceAll(apps, apps + n_apps);
}

void AppList::addApp(const App &app)
{
    if (apps_.contains(app.id())) {
        apps_[app.id()] = app;
        return;
    }

    // There really should be a way to do this that is O(log n) and
    // doesn't involve copying
    QList<App::Id> keys = apps_.keys();
    int index = qLowerBound(keys, app.id()) - keys.begin();

    beginInsertRows(QModelIndex(), index, index);
    apps_.insert(app.id(), app);
    endInsertRows();
}

void AppList::removeApp(App::Id id)
{
    if (!apps_.contains(id))
        return;

    int row = apps_.keys().indexOf(id);
    beginRemoveRows(QModelIndex(), row, row);
    apps_.remove(id);
    endRemoveRows();
}

void AppList::clear()
{
    beginResetModel();
    apps_.clear();
    endResetModel();
}
