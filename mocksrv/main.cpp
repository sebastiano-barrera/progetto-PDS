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
        { "Google Chrome - YouTube" },
        { "Sony Japan | ソニーグループ ポータルサイト" },
        { "Git Bash" },
        { "Adobe Photoshop" },
    };

    QTcpServer server;

    if (!server.listen(QHostAddress::Any, 3000)) {
        qInfo() << "Couldn't start server: " << server.errorString();
        return 1;
    }

    QObject::connect(&server, &QTcpServer::newConnection, [&]() {
        // the protocol object is simply created, set up and started.
        // we let Qt's main loop do the spinning
        auto client = server.nextPendingConnection();
        auto proto = new MockServerProtocol(windows, client);
        QObject::connect (client, &QTcpSocket::aboutToClose,
                          proto, &MockServerProtocol::deleteLater);
        proto->start();
    });

    return a.exec();
}
