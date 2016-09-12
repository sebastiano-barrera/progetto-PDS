#include "connectdialog.h"
#include "ui_connectdialog.h"
#include "connection.h"


ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    conn_(new Connection(0)),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    ui->connWidget->setSocket(&conn_->socket());
    connect(ui->btnConnect,   &QPushButton::clicked, ui->connWidget, &ConnectionWidget::openConn);
    connect(ui->btnCancel,    &QPushButton::clicked, this,           &QDialog::reject);
    connect(&conn_->socket(), &QAbstractSocket::stateChanged, this, &ConnectDialog::socketStateChanged);
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

std::unique_ptr<Connection> ConnectDialog::giveConnection()
{
    return std::move(conn_);
}

void ConnectDialog::socketStateChanged(QAbstractSocket::SocketState state)
{
    if (state == QAbstractSocket::ConnectedState)
        this->accept();
}
