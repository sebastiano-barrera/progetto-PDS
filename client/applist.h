#ifndef APPLIST_H
#define APPLIST_H

#include <QAbstractListModel>
#include <QString>
#include <QMap>

namespace msgs {
    class Application;
    class AppList;
}

class App {
    // TODO: extract icon data
public:
    typedef quint32 Id;

    App();
    App(const msgs::Application& msg);
    App(Id id, QString name) :
        valid_(true), id_(id), name_(name) { }

    inline bool isValid() const { return valid_; }
    inline Id id() const { return id_; }
    inline const QString& name() const { return name_; }

private:
    bool valid_;
    QString name_;
    Id id_;
};


class AppList : public QAbstractListModel
{
    Q_OBJECT

    // using a map keeps the keys ordered, and allows us to predict
    // the order of the rows in the model when calling beginInsertRows
    QMap<App::Id, App> apps_;

public:
    explicit AppList(QObject *parent = 0);

    inline const QMap<App::Id, App>& apps() const { return apps_; }

    template <typename Iter> void replaceAll(Iter start, Iter end)
    {
        beginResetModel();
        apps_.clear();
        for(Iter iter=start; iter != end; iter++) {
            const auto& app = *iter;
            apps_.insert(app.id(), app);
        }
        endResetModel();
    }

    const App* atIndex(const QModelIndex& index) const;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void replaceAll(const App* apps, size_t n_apps);
    void addApp(const App &);
    void removeApp(App::Id);
    void clear();
};

#endif // APPLIST_H
