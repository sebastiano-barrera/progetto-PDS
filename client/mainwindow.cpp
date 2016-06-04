#include "ui_mainwindow.h"

#include "mainwindow.h"
#include "protocol.pb.h"
#include "keystrokeselector.h"

#include <QCloseEvent>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    ui_->appListView->setModel(&appListModel_);
    ui_->connectBar->setSocket(&conn_);
    proto_.setSocket(&conn_);

    connect(&conn_, &QTcpSocket::connected, &proto_, &ClientProtocol::start);

    connect(&proto_, &ClientProtocol::appListReceived, &appListModel_, &AppList::replaceAll);
    connect(&proto_, &ClientProtocol::appCreated, &appListModel_, &AppList::addApp);
    connect(&proto_, &ClientProtocol::appDestroyed, &appListModel_, &AppList::removeApp);
    connect(&proto_, &ClientProtocol::stopped, &appListModel_, &AppList::clear);
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
