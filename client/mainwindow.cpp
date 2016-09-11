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

    connect(ui_->keySelector, &KeystrokeSelector::sendRequested, this, &MainWindow::sendKeystroke);
    connect(ui_->btnAddConn, &QPushButton::clicked, this, &MainWindow::addConnection);
    connect(ui_->btnRemoveConn, &QPushButton::clicked, this, &MainWindow::removeConnection);
    connect(ui_->btnReconnect, &QPushButton::clicked, this, &MainWindow::reconnectSelected);

    connect(ui_->serverListView->selectionModel(),  &QItemSelectionModel::selectionChanged,
            this, &MainWindow::connSelectionChanged);
    connect(ui_->appListView->selectionModel(),  &QItemSelectionModel::selectionChanged,
            this, &MainWindow::appSelectionChanged);

    connect(this, &MainWindow::connectionAdded, &serverListModel_, &ServerListModel::addConnection);
    connect(this, &MainWindow::connectionAdded, &appListModel_, &AppList::addConnection);

    connect(&msgBox_, &QMessageBox::finished, this, [this](int result){
        msgBox_.setText("");
    });

    appSelectionChanged();
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
    // Works fine on Linux, would you guess
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
        const App* app = appListModel_.atIndex(proxyModel_.mapToSource(index));
        if (app == nullptr || !app->isFocused()) 
            continue;

        // Using a unique_ptr in preparation for pending requests tracking,
        // which will be based on storing unique_ptrs
        auto req_inst = std::make_unique<msgs::KeystrokeRequest>(req);
        req_inst->set_app_id(app->id());
        app->parentConn()->sendRequest(std::move(req_inst));
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
    unsigned total = 0;
    for (const auto& conn : connections_) {
        assert (conn != nullptr);
        total += conn->pendingRequestsCount();
    }

    statusBar()->showMessage(QString("%1 pending requests").arg(total));
}

void MainWindow::showResponse(Connection *conn,
                              const msgs::KeystrokeRequest &req,
                              const msgs::Response &res)
{
    updatePendingReqMsg();

    if (res.status() == msgs::Response::Success)
        return;

    const auto* app = conn->appById(req.app_id());
    if (app == nullptr) {
        qWarning() << "A request failed for a now-disappeared application\n";
        return;
    }

    auto dialogText = msgBox_.text();
    if (dialogText.size() == 0)
        dialogText = "The following requests failed:\n";

    dialogText += QString("- From %1: %2 (%3): %4\n")
            .arg(conn->hostAddress().toString())
            .arg(app->processPath().fileName())
            .arg(app->title())
            .arg(statusMessage(res.status()));

    msgBox_.setText(dialogText);
    msgBox_.show();
}

void MainWindow::addConnection()
{
    auto dialog = new ConnectDialog(this);
    connect(dialog, &QDialog::accepted, [=]() {
        // move the connection object out of the dialog,
        // claim ownership, and add to collection
        std::unique_ptr<Connection> conn = std::move(dialog->giveConnection());
        if (!conn)
            return;

        connect(conn.get(), &Connection::responseReceived, this, &MainWindow::showResponse);
        connections_.push_back(conn.release());
        emit connectionAdded(connections_.back());

        dialog->deleteLater();
    });

    connect(dialog, &QDialog::rejected, dialog, &QDialog::deleteLater);

    dialog->show();
}

void MainWindow::removeConnection()
{
    QItemSelectionModel* selModel = ui_->serverListView->selectionModel();
    auto selRows = selModel->selectedRows();
    for (auto index : selRows) {
        auto *conn = serverListModel_.atIndex(index);
        conn->socket().close();
        conn->deleteLater();

        int index = connections_.indexOf(conn);
        if (index != -1)
            connections_.remove(index);
    }
}

void MainWindow::reconnectSelected()
{
    QItemSelectionModel* selModel = ui_->serverListView->selectionModel();
    auto selRows = selModel->selectedRows();
    for (auto index : selRows) {
        auto *conn = serverListModel_.atIndex(index);
        conn->socket().close();
        conn->socket().connectToHost(conn->hostAddress(), conn->port());
    }
}

void MainWindow::connSelectionChanged()
{
    QItemSelectionModel* selModel = ui_->serverListView->selectionModel();
    auto selRows = selModel->selectedRows();

    if (selRows.size() == 0) {
        ui_->btnReconnect->setEnabled(false);
        ui_->btnRemoveConn->setEnabled(false);
    } else {
        ui_->btnReconnect->setEnabled(true);
        ui_->btnRemoveConn->setEnabled(true);
    }
}

void MainWindow::appSelectionChanged()
{
    QItemSelectionModel* selModel = ui_->appListView->selectionModel();
    auto selRows = selModel->selectedRows();

    unsigned count = 0;
    for (auto index : selRows) {
        auto *app = appListModel_.atIndex(proxyModel_.mapToSource(index));
        if (app->isFocused())
            count++;
    }

    QString text;
    if (count == 0)
        text = QString("Please select one or more focused windows");
    else
        text = QString("Will be sent only to the %1 focused apps").arg(count);

    ui_->lblNumFocusedWin->setText(text);
}
