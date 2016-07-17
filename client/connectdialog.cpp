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
    connect(this, &QDialog::finished, this, &QObject::deleteLater);
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

std::unique_ptr<Connection> ConnectDialog::giveConnection()
{
    return std::move(conn_);
}

