#include "ui_mainwindow.h"
#include "keyconv.h"
#include "mainwindow.h"
#include "protocol.pb.h"
#include "keystrokeselector.h"

#include <QCloseEvent>
#include <QItemSelectionModel>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow),
    numPendingReqs_(0)
{
    ui_->setupUi(this);
    ui_->appListView->setModel(&appListModel_);
    ui_->connectBar->setSocket(&conn_);

    proto_.setSocket(&conn_);

    connect(ui_->btnSend, &QPushButton::clicked, this, &MainWindow::sendKeystroke);

    connect(&conn_, &QTcpSocket::connected, &proto_, &ClientProtocol::start);
    connect(&conn_, &QTcpSocket::connected, &appListModel_, &AppList::resetConnectionTime);

    // GCC: disambiguate overload
    auto replaceAll = static_cast<void (AppList::*)(const App*, size_t)>(&AppList::replaceAll);
    connect(&proto_, &ClientProtocol::appListReceived, &appListModel_, replaceAll);
    connect(&proto_, &ClientProtocol::appCreated, &appListModel_, &AppList::addApp);
    connect(&proto_, &ClientProtocol::appDestroyed, &appListModel_, &AppList::removeApp);
    connect(&proto_, &ClientProtocol::stopped, &appListModel_, &AppList::clear);
    connect(&proto_, &ClientProtocol::appGotFocus, &appListModel_, &AppList::setFocusedApp);
    connect(&proto_, &ClientProtocol::responseReceived, this, &MainWindow::showResponse);

    connect(&msgBox_, &QMessageBox::finished, this, [this](int result){
        msgBox_.setText("");
    });

    updatePendingReqMsg();
    connect(&proto_, &ClientProtocol::stopped, this, [this]() {
        numPendingReqs_ = 0;
        updatePendingReqMsg();
    });

}

void MainWindow::closeEvent(QCloseEvent *event) {
#ifdef _WIN32
    // not the dirtiest hack, but not pretty either...
    if (keyboardGrabber() == ui_->keySelector) {
        event->ignore();
        // I don't really know how to make this portable or even just not gravely shameful
        ui_->keySelector->setKey(Qt::ALT + Qt::Key_F4);
    }
#else
    (void) event;
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
    // copy a new request for each selected window
    for (QModelIndex index : selIndices) {
        const App* app = appListModel_.atIndex(index);
        if (app == nullptr)
            continue;

        req.set_app_id(app->id());
        proto_.sendRequest(std::make_unique<msgs::KeystrokeRequest>(req));
        numPendingReqs_++;
    }

    updatePendingReqMsg();
}

const char* statusMessage(msgs::Response::Status status)
{
    switch(status) {
    case msgs::Response::Success:
        return "Success";
    case msgs::Response::WindowLostFocus:
        return "Window lost focus in the meantime";
    }
    return "Unknown status";
}

void MainWindow::updatePendingReqMsg()
{
    if (proto_.isStarted())
        statusBar()->showMessage(QString("%1 requests pending").arg(numPendingReqs_));
    else
        statusBar()->showMessage("Disconnected");
}

void MainWindow::showResponse(const msgs::KeystrokeRequest &req,
                              const msgs::Response &res)
{
    numPendingReqs_--;
    updatePendingReqMsg();

    if (res.status() == msgs::Response::Success)
        return;

    auto dialogText = msgBox_.text();
    if (dialogText.size() == 0)
        dialogText = "The following requests failed:\n";

    auto iter = appListModel_.apps().find(req.app_id());
    if (iter != appListModel_.apps().end()) {
        const auto& app = *iter;
        dialogText += QString("- %1: %2\n")
                .arg(app.title())
                .arg(statusMessage(res.status()));
    }

    msgBox_.setText(dialogText);
    msgBox_.show();
}
