#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "clientprotocol.h"
#include "applist.h"

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTcpSocket>
#include <QTimer>

#include <memory>

namespace Ui { class MainWindow; }
namespace msgs { class AppList; }
class QModelIndex;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

protected:
    virtual void closeEvent(QCloseEvent *) override;

private slots:
    void sendKeystroke();

private:
    std::unique_ptr<Ui::MainWindow> ui_;
    QTcpSocket conn_;
    ClientProtocol proto_;

    AppList appListModel_;
};

#endif // MAINWINDOW_H
