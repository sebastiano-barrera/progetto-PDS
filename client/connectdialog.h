#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QAbstractSocket>
#include <QDialog>

#include <memory>

namespace Ui {
class ConnectDialog;
}

class Connection;


class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = 0);
    ~ConnectDialog();

    std::unique_ptr<Connection> giveConnection();

private slots:
    void socketStateChanged(QAbstractSocket::SocketState);

private:
    std::unique_ptr<Connection> conn_;
    Ui::ConnectDialog *ui;
};

#endif // CONNECTDIALOG_H
