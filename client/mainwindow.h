#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "clientprotocol.h"
#include "applist.h"

#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QStandardItemModel>
#include <QTcpSocket>
#include <QTimer>
#include <QMessageBox>

#include <memory>

namespace Ui { class MainWindow; }
namespace msgs {
    class AppList;
    class KeystrokeRequest;
    class Response;
}
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
    void showResponse(const msgs::KeystrokeRequest&,
                      const msgs::Response&);

private:
    void updatePendingReqMsg();

    std::unique_ptr<Ui::MainWindow> ui_;
    QTcpSocket conn_;
    ClientProtocol proto_;
    AppList appListModel_;
    QMessageBox msgBox_;
    int numPendingReqs_;
};

#endif // MAINWINDOW_H
