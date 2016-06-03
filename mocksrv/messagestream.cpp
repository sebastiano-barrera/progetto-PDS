#include "messagestream.h"
#include <cassert>


MessageStream::MessageStream(QIODevice *device, QObject *parent) :
    QObject(parent),
    dev_(nullptr)
{
    setDevice(device);
}

void MessageStream::setDevice(QIODevice *dev)
{
    if (dev_) {
        // discard temporary state
        dev_->disconnect(this);
        msgbuf_ = nullptr;
        remaining_ = 0;
    }

    dev_ = dev;

    if (dev_) {
        connect(dev_, &QIODevice::readyRead, this, &MessageStream::readyRead);
    }
}

void MessageStream::readyRead()
{
    // This class implements a simple protocol to make Protobuf messages
    // delimited on the network. Since we're using TCP, 95% of the problem is
    // solved already. We just extend the protocol by simply sending, before
    // each message, its length encoded as uint32.
    //    This simple protocol for delimiting messages can be modeled as a
    // finite state machine with only two states:
    //    - no message yet =>
    //        len <- read the length
    //        go to state [$len bytes remaining]
    //    - $len bytes remaining =
    //        read at the most $len bytes from the socket
    //        N bytes have been read,
    //          go to state [($len - N) bytes remaining]
    //    - 0 bytes remaining =
    //        give the message to whoever wants it through the signal,
    //

    if (!msgbuf_) {
        // No message
        quint32 msg_len;
        int ret = dev_->read((char*) &msg_len, sizeof(msg_len));
        if (ret == -1)
            goto err;

        msgbuf_ = std::unique_ptr<QByteArray> { new QByteArray() };
        remaining_ = msg_len;
    }

    {
        static const quint32 MAX_READ = 1024;
        char buf[MAX_READ];
        int nbytesread = dev_->read(buf, std::min(remaining_, MAX_READ));
        if (nbytesread == -1)
            goto err;

        assert ((quint32)nbytesread <= remaining_);
        remaining_ -= nbytesread;
        if (remaining_ == 0) {
            emit messageReceived(*msgbuf_);
            msgbuf_ = nullptr;  // back to original state
        }
    }
    return;

err:
    // free our current buffer (if any existed) and hope that the users
    // of this object are handling errors (the `error' signal and getter)
    msgbuf_ = nullptr;
}

void MessageStream::sendMessage(const QByteArray &msg)
{
    quint32 msg_len = msg.size();
    dev_->write((char*) &msg_len, sizeof(msg_len));
    dev_->write(msg);
}
