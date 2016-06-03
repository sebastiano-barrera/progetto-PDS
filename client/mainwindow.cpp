#include "ui_mainwindow.h"

#include "mainwindow.h"
#include "protocol.pb.h"
#include "keystrokeselector.h"

#include <QCloseEvent>

using msgs::Application;
using msgs::AppList;


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

void MainWindow::closeEvent(QCloseEvent *event) {
#ifdef _WIN32
    // not the dirtiest hack, but not pretty either...
    if (keyboardGrabber() == ui_->keySelector) {
        event->ignore();
        // I don't really know how to make this portable or even just not gravely shameful
        ui_->keySelector->setKey(Qt::ALT + Qt::Key_F4);
    }
#endif
}
