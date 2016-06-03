#include "ui_mainwindow.h"

#include "mainwindow.h"
#include "protocol.pb.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow)
{
    appListModel_.setColumnCount(1);

    ui_->setupUi(this);
    ui_->appListView->setModel(&appListModel_);
    ui_->connectBar->setSocket(&conn_);
    proto_.setClient(&conn_);

    connect(&conn_, &QTcpSocket::connected, &proto_, &ClientProtocol::start);
    connect(&proto_, &ClientProtocol::appListReceived,
            this, &MainWindow::appListReceived);
}

void MainWindow::appListReceived(const AppList &appList)
{
    appListModel_.clear();
    for (const Application& app : appList.apps()) {
        QString text = QString::fromStdString(app.name());
        auto item = new QStandardItem(text);
        item->setEditable(false);
        appListModel_.appendRow(item);
    }
}
