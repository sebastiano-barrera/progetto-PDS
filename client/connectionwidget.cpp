#include "connectionwidget.h"
#include "ui_connectionwidget.h"

#include <QHostAddress>
#include <QPushButton>

ConnectionWidget::ConnectionWidget(QWidget *parent) :
    QWidget(parent),
    sock_(nullptr),
    ui_(new Ui::ConnectionWidget)
{
    ui_->setupUi(this);

    connect(ui_->lneHostName, &QLineEdit::returnPressed,
            this, &ConnectionWidget::openConn);
    connect(ui_->btnConnect, &QPushButton::clicked,
            this, &ConnectionWidget::connectClicked);
}

ConnectionWidget::~ConnectionWidget()
{
    delete ui_;
}

void ConnectionWidget::setSocket(QAbstractSocket *sock)
{
    if (sock_ == sock)
        return;

    if (sock_ != nullptr) {
        sock_->disconnect(this);
    }

    sock_ = sock;

    if (sock_ != nullptr) {
        connect(sock_, &QAbstractSocket::stateChanged,
                this, &ConnectionWidget::stateChanged);
        // need to do this to disambiguate with the `error()` getter
        auto sig_error = static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error);
        connect(sock_, sig_error, this, &ConnectionWidget::error);
    }

    stateChanged();
}

void ConnectionWidget::connectClicked()
{
    if (sock_ == nullptr)
        return;

    if (sock_->state() == QAbstractSocket::UnconnectedState)
        openConn();    // unconnected => connect
    else if (sock_->state() == QAbstractSocket::ConnectedState)
        closeConn();   // connected => disconnect

    // all other states are transitory (in which case, just do nothing)
}

void ConnectionWidget::openConn()
{
    if (!sock_ || sock_->isOpen())
        return;
    QHostAddress addr(ui_->lneHostName->text());
    quint16 port = ui_->spnPort->value();
    sock_->connectToHost(addr, port);
}

void ConnectionWidget::closeConn()
{
    if (sock_)
        sock_->close();
}

void ConnectionWidget::stateChanged()
{
    if (sock_ == nullptr) {
        this->setEnabled(false);
        return;
    }

    switch(sock_->state()) {
    case QAbstractSocket::ConnectedState:
        setEnabled(true);
        ui_->lblError->setText("Connected");
        ui_->btnConnect->setText("Disconnect");
        break;

    case QAbstractSocket::HostLookupState:
        setEnabled(false);
        ui_->lblError->setText("Looking up host name");
        break;

    case QAbstractSocket::ConnectingState:
        setEnabled(false);
        ui_->lblError->setText("Connecting...");
        break;

    case QAbstractSocket::UnconnectedState:
        setEnabled(true);
        ui_->lblError->setText("Not connected");
        ui_->btnConnect->setText("Connect");
        break;


    case QAbstractSocket::ClosingState:
        setEnabled(true);
        ui_->lblError->setText("Closing..");
        break;

    default:
        // all other states (in QAbstractSocket::SocketState)
        // are never observable (socket is a client)
        setEnabled(false);
        break;
    }
}

void ConnectionWidget::error(QAbstractSocket::SocketError err)
{
    (void) err;
    if (!sock_)
        return;

    ui_->lblError->setText(sock_->errorString());
}

