#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "keystrokeselector.h"
#include "connection.h"
#include "connectdialog.h"
#include "keyconv.h"
#include "protocol.pb.h"
#include "app.h"

#include <QCloseEvent>
#include <QItemSelectionModel>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow)
{
    proxyModel_.setSourceModel(&appListModel_);

    ui_->setupUi(this);
    ui_->appListView->setModel(&proxyModel_);
    ui_->serverListView->setModel(&serverListModel_);

    connect(ui_->btnSend, &QPushButton::clicked, this, &MainWindow::sendKeystroke);
    // connect(ui_->btnConnect, &QPushButton::clicked, this, &MainWindow::openConnectDialog);
    connect(this, &MainWindow::connectionAdded, &serverListModel_, &ServerListModel::addConnection);
    connect(this, &MainWindow::connectionAdded, &appListModel_, &AppList::addConnection);

    connect(&msgBox_, &QMessageBox::finished, this, [this](int result){
        msgBox_.setText("");
    });

    // updatePendingReqMsg();
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
        if (app == nullptr || !app->isFocused()) 
            continue;

        // Using a unique_ptr in preparation for pending requests tracking,
        // which will be based on storing unique_ptrs
        auto req_inst = std::make_unique<msgs::KeystrokeRequest>(req);
        req_inst->set_app_id(app->id());
        app->parentConn()->sendRequest(*req_inst);
    }

    // updatePendingReqMsg();
}

#if 0

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
    // STUB
//    if (proto_.isStarted())
//        statusBar()->showMessage(QString("%1 requests pending").arg(numPendingReqs_));
//    else
//        statusBar()->showMessage("Disconnected");
}

void MainWindow::showResponse(const msgs::KeystrokeRequest &req,
                              const msgs::Response &res)
{
    updatePendingReqMsg();

    if (res.status() == msgs::Response::Success)
        return;

    auto dialogText = msgBox_.text();
    if (dialogText.size() == 0)
        dialogText = "The following requests failed:\n";

    auto iter = order_.find(req.app_id());
    if (iter != appListModel_.apps().end()) {
        const auto& app = *iter;
        dialogText += QString("- %1: %2\n")
                .arg(app.title())
                .arg(statusMessage(res.status()));
    }

    msgBox_.setText(dialogText);
    msgBox_.show();
}

void MainWindow::openConnectDialog()
{
    auto dialog = new ConnectDialog(this);
    connect(dialog, &QDialog::accepted, [=]() {
        // move the connection object out of the dialog,
        // claim ownership, and add to collection
        std::unique_ptr<Connection> conn = std::move(dialog->giveConnection());
        connections_.emplace_back(std::move(conn));

        emit connectionAdded(connections_.back().get());
    });

    dialog->show();
}
#endif
