#include "applist.h"
#include "protocol.pb.h"

#include <QPixmap>
#include <QImage>
#include <QDebug>


App::App() :
    valid_(false)
    { }

App::App(const msgs::Application &msg) :
    valid_(true),
    id_(msg.id()),
    focused_(false),
    totalFocusTime_(0)
{
    assert (id_ != INVALID_ID);

    if (msg.has_win_title())
        title_ = QString::fromStdString(msg.win_title());

    if (msg.has_name())
        processPath_ = QFileInfo(QString::fromStdString(msg.name()));

    if (msg.has_icon()) {
        auto& icon = msg.icon();
        QImage image(reinterpret_cast<const uchar*>(icon.pixels().data()),
                     icon.width(), icon.height(),
                     QImage::Format_ARGB32);
        icon_ = QPixmap::fromImage(image.scaledToHeight(16));
    }
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



AppList::AppList(QObject *parent) :
    QAbstractTableModel(parent),
    focusedApp_(App::INVALID_ID)
{
    updateTimer_.setInterval(200);
    updateTimer_.setSingleShot(false);
    connect(&updateTimer_, &QTimer::timeout, this, &AppList::focusTimeColumnChanged);
    // updateTimer_ is started by resetConnectionTime()
}

const App* AppList::atIndex(const QModelIndex& index) const
{
    if (index.parent().isValid())
        return nullptr;

    auto id = apps_.keys().value(index.row(), App::INVALID_ID);
    if (id == App::INVALID_ID)
        return nullptr;
    return &apps_.find(id).value();
}

QModelIndex AppList::indexOf(App::Id appId, int column) const
{
    int row = apps_.keys().indexOf(appId);
    if (row == -1)
        return {};  // return invalid index

    return this->index(row, column);
}

int AppList::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())  // parent != invisible root
        return 0;

    return apps_.size();
}

int AppList::columnCount(const QModelIndex &parent) const
{
    // `parent` should be the index of the invisible root
    return parent.isValid() ? 0 : 4;
}

QVariant AppList::data(const QModelIndex &index, int role) const
{
    if (index.parent().isValid())
        return QVariant();

    const auto* app = atIndex(index);
    int col = index.column();
    if (app == nullptr || col >= 3)
        return QVariant();

    if (col == 0) {
        if (role == Qt::DisplayRole)
            return app->processPath().fileName();
        else if (role == Qt::DecorationRole)
            return app->icon();

    } else if (col == 1 && role == Qt::CheckStateRole) {
            return app->isFocused();

    } else if (col == 2 && role == Qt::DisplayRole) {
        if (connectionTimer_.isValid()) {
            double timePerc = (double) app->focusTimeMS() / connectionTimer_.elapsed() * 100;
            return QString("%1 %").arg(timePerc, 0, 'g', 2);
        } else {
            return  " - ";
        }

    } else if (col == 3 && role == Qt::DisplayRole) {
        auto title = app->title();
        return title.size() == 0 ? " - " : title;
    }

    return QVariant();
}

QVariant AppList::headerData(int section, Qt::Orientation orientation, int role) const
{
    static const char* const headers[] = {
        "Process",
        "Focused",
        "Time focused (since connection)"
        "Window title",
    };
    static const size_t n_headers = sizeof(headers)/sizeof(headers[0]);

    if (orientation != Qt::Horizontal
            || role != Qt::DisplayRole
            || section >= n_headers)
        return {};

    return headers[section];
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

void AppList::setFocusedApp(App::Id appId)
{
    auto iter = apps_.find(focusedApp_);
    if (iter != apps_.end())
        iter->setFocused(false);

    focusedApp_ = appId;

    iter = apps_.find(appId);
    if (iter != apps_.end())
        iter->setFocused(true);
}

void AppList::focusTimeColumnChanged()
{
    QModelIndex root;
    QModelIndex topLeft = index(0, 1);
    QModelIndex bottomRight = index(rowCount(root) - 1,  columnCount(root) - 1);
    dataChanged(topLeft, bottomRight);
}

void AppList::resetConnectionTime()
{
    updateTimer_.start();
    connectionTimer_.restart();
    assert (connectionTimer_.isValid());
}
