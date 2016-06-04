#include "applist.h"
#include "protocol.pb.h"

App::App()
    : valid_(false)  // the rest is default-constructed
    { }

App::App(const msgs::Application &msg) :
    valid_(true),
    name_(QString::fromStdString(msg.name())),
    id_(msg.app_id())
    { }


AppList::AppList(QObject *parent) :
    QAbstractListModel(parent)
    { }

const App& AppList::atIndex(const QModelIndex& index) const
{
    if (index.parent().isValid())
        return App();

    auto id = apps_.keys().value(index.row(), -1);
    return apps_[id];
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

    const auto& app = atIndex(index);
    if (!app.isValid())
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:  {
        // Only for MSVC (tested ver. 19.0):
        //   here, the statement
        //     return app.name();
        //   triggers a runtime exception (a write at a small memory address).
        //   looks like a compiler bug.
        auto name = app.name();
        return name;
    }
    default:
        return QVariant();
    }
}

void AppList::replaceAll(const msgs::AppList &msg)
{
    const auto& apps = msg.apps();
    replaceAll(apps.cbegin(), apps.cend());
}
