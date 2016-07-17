#ifndef KEYSTROKESELECTOR_H
#define KEYSTROKESELECTOR_H

#include <QPushButton>
#include <QKeySequence>

class QKeyEvent;
class QCloseEvent;


class KeystrokeSelector : public QPushButton
{
    Q_OBJECT

    // even though a QKeySequence object can store a sequence, we only ever
    // store one single key in it. We use the QKeySequence class to take
    // advantage of some of its methods
    QKeySequence key_;
    bool waitingInput_;

public:
    explicit KeystrokeSelector(QWidget *parent = 0);

    inline int key() const { return key_[0]; }
    void setKey(int key);

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;

signals:
    void keyChanged(int key);

private slots:
    void acquireKey();

private:
    void updateText();
};

#endif // KEYSTROKESELECTOR_H
