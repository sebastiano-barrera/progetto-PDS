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
    class Application;
    class AppList;
}

class App {
    // TODO: extract icon data
public:
    typedef quint32 Id;
    enum {
        INVALID_ID = (Id) -1
    };

    App();
    App(const msgs::Application& msg);
    App(Id id, QString name) :
        valid_(true), id_(id), title_(name) { }

    inline bool isValid() const { return valid_; }
    inline Id id() const { return id_; }
    inline const QString& title() const { return title_; }
    inline const QFileInfo& processPath() const { return processPath_; }
    inline const QPixmap& icon() const { return icon_; }

    void setFocused(bool focused);
    inline bool isFocused() const { return focused_; }
    quint64 focusTimeMS() const;

private:
    bool valid_;
    Id id_;
    QString title_;
    QFileInfo processPath_;
    QPixmap icon_;

    bool focused_;
    QElapsedTimer focusTimer_;
    quint64 totalFocusTime_;
};


class AppList : public QAbstractTableModel
{
    Q_OBJECT

    // using a map keeps the keys ordered, and allows us to predict
    // the order of the rows in the model when calling beginInsertRows
    QMap<App::Id, App> apps_;
    App::Id focusedApp_;
    QTimer updateTimer_;
    QElapsedTimer connectionTimer_;

    QModelIndex indexOf(App::Id, int column = 0) const;

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
    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;


public slots:
    void replaceAll(const App* apps, size_t n_apps);
    void addApp(const App &);
    void removeApp(App::Id);
    void clear();
    void setFocusedApp(App::Id);
    void focusTimeColumnChanged();
    void resetConnectionTime();
};

#endif // APPLIST_H
