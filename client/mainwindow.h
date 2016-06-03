#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "clientprotocol.h"

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTcpSocket>

#include <memory>

namespace Ui {
    class MainWindow;
}

namespace msgs {
class AppList;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

public slots:
    void appListReceived(const msgs::AppList&);

protected:
    virtual void closeEvent(QCloseEvent *) override;

private:
    std::unique_ptr<Ui::MainWindow> ui_;
    QTcpSocket conn_;
    ClientProtocol proto_;

    QStandardItemModel appListModel_;
};

#endif // MAINWINDOW_H
