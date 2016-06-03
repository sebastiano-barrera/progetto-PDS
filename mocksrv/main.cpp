#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QVector>

#include "windowinfo.h"
#include "mockserverprotocol.h"
#include "protocol.pb.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QVector<WindowInfo> windows = {
        { "Google Chrome - Spotify - Kyuss  Supa Scoopa Mighty Scoop" },
        { "main.cpp [master] - mocksrv - QtCreator" },
        { "protobuf - Microsoft Visual Studio" },
        { "Gestione attività" },
    };

    QTcpServer server;

    if (!server.listen(QHostAddress::Any, 3000)) {
        qInfo() << "Couldn't start server: " << server.errorString();
        return 1;
    }

    QObject::connect(&server, &QTcpServer::newConnection, [&]() {
        auto client = server.nextPendingConnection();
        auto proto = new MockServerProtocol(client, windows);
        QObject::connect (client, &QTcpSocket::aboutToClose,
                          proto, &MockServerProtocol::deleteLater);

        QThread *thread = new QThread();
        proto->moveToThread(thread);
        thread->start();

        proto->start();
    });

    return a.exec();
}
