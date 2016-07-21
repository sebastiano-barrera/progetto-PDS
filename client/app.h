#ifndef APP_H
#define APP_H

#include <QPixmap>
#include <QString>
#include <QFileInfo>
#include <QElapsedTimer>

namespace msgs {
class Application;
}

class Connection;


// `App` objects are small, lightweight data structures about a single
// application window in a remote server.
//
// NOTE: AppList depends on the address of App objects: if an App object is copied,
//       AppLists will treat them as distinct objects (despite having the same
//       parent Connection and id). This will be fixed, but for now it's a sad fact of life.
class App : public QObject {
    Q_OBJECT
public:
    typedef ClientProtocol::AppId Id;
    enum {
        INVALID_ID = (Id) -1
    };

    App(Connection *conn, const msgs::Application& msg, QObject *parent = 0);
    App& operator=(App&&);

    inline Connection* parentConn() const { return parentConn_; }
    inline Id id() const { return id_; }
    inline const QString& title() const { return title_; }
    inline const QFileInfo& processPath() const { return processPath_; }
    inline const QPixmap& icon() const { return icon_; }

    void setFocused(bool focused);
    inline bool isFocused() const { return focused_; }
    quint64 focusTimeMS() const;

private:
    Id id_;
    Connection *parentConn_;
    QString title_;
    QFileInfo processPath_;
    QPixmap icon_;

    bool focused_;
    QElapsedTimer focusTimer_;
    quint64 totalFocusTime_;
};

#endif // APP_H
