#ifndef MESSAGESTREAM_H
#define MESSAGESTREAM_H

#include <QObject>
#include <QIODevice>
#include <QByteArray>

#include <memory>

/// Wraps a QIODevice and allows to send and receive messages (typically
/// protobuf-made). It implements a simple wire protocol to delimit messages.
///
/// This class is currently NOT thread-safe.
class MessageStream : public QObject
{
    Q_OBJECT
public:
    MessageStream(QIODevice *device = 0, QObject *parent = 0);
    MessageStream(const MessageStream&) = delete;

    inline QIODevice* device() const { return dev_; }
    void setDevice(QIODevice*);

public slots:
    void sendMessage(const QByteArray&);
signals:
    void messageReceived(const QByteArray&);


private slots:
    void readyRead();
private:
    QIODevice *dev_;
    std::unique_ptr<QByteArray> msgbuf_;
    quint32 remaining_;
};

#endif // MESSAGESTREAM_H
