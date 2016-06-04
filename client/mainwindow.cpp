#include "ui_mainwindow.h"
#include "keyconv.h"
#include "mainwindow.h"
#include "protocol.pb.h"
#include "keystrokeselector.h"

#include <QCloseEvent>
#include <QItemSelectionModel>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    ui_->appListView->setModel(&appListModel_);
    ui_->connectBar->setSocket(&conn_);
    proto_.setSocket(&conn_);

    connect(ui_->btnSend, &QPushButton::clicked, this, &MainWindow::sendKeystroke);

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

void MainWindow::sendKeystroke()
{
    int qkey = ui_->keySelector->key();
    // key without the modifiers
    int qkey_nomods = qkey & ~Qt::KeyboardModifierMask;
    msgs::Keycode keycode = keyconv::toKeycode(qkey_nomods);

    msgs::KeystrokeRequest req;
    req.set_key(keycode);
    req.set_ctrl ((qkey & Qt::ControlModifier) != 0);
    req.set_alt  ((qkey & Qt::AltModifier) != 0);
    req.set_shift((qkey & Qt::ShiftModifier) != 0);
    req.set_meta ((qkey & Qt::MetaModifier) != 0);

    auto selIndices = ui_->appListView->selectionModel()->selectedRows();
    qDebug() << "sending keystroke for "
             << selIndices.size()
             << " selected items";
    for (QModelIndex index : selIndices) {
        const App* app = appListModel_.atIndex(index);
        if (app == nullptr)
            continue;

        req.set_app_id(app->id());
        proto_.sendRequest(req);
    }
}
