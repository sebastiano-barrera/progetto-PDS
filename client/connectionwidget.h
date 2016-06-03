#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QAbstractSocket>

namespace Ui { class ConnectionWidget; }


class ConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWidget(QWidget *parent = 0);
    ~ConnectionWidget();

    inline QAbstractSocket* socket() const { return sock_; }
    void setSocket(QAbstractSocket *sock);

protected slots:
    void openConn();
    void closeConn();
    void connectClicked();
    void stateChanged();
    void error(QAbstractSocket::SocketError err);

private:
    QAbstractSocket *sock_;
    Ui::ConnectionWidget *ui_;
};

#endif // CONNECTIONWIDGET_H
