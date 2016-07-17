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

    QTimer updateTimer_;
    QVector<const App*> order_;

public:
    explicit AppList(QObject *parent = 0);

    const App* atIndex(const QModelIndex& index) const;
    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public slots:
    void addApp(const App *app);
    void removeApp(const App *app);
    void addConnection(const Connection *);

private slots:
    void focusTimeColumnChanged();
};

#endif // APPLIST_H
