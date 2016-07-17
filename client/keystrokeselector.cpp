#include "keystrokeselector.h"
#include <QKeyEvent>

KeystrokeSelector::KeystrokeSelector(QWidget *parent) :
    QPushButton(parent),
    waitingInput_(false)
{
    connect(this, &QPushButton::clicked, this, &KeystrokeSelector::acquireKey);
}

void KeystrokeSelector::setKey(int key)
{
    waitingInput_ = false;
    releaseKeyboard();

    key_ = QKeySequence(key);
    Q_ASSERT (key_.count() == 1);

    setText(key_.toString());
    emit keyChanged(key_[0]);
}

void KeystrokeSelector::keyPressEvent(QKeyEvent* event)
{
    if (!waitingInput_)
        return QPushButton::keyPressEvent(event);

    // the purpose of this array is simply to discard keystrokes amounting to a
    // press of just a modifier key.
    static const Qt::Key pure_modifiers[] = {
        Qt::Key_Control, Qt::Key_Shift, Qt::Key_Alt, Qt::Key_Meta, Qt::Key_AltGr,
    };

    qWarning("Key: <0x%08x>", event->key());

    for (Qt::Key pure_mod : pure_modifiers) {
        if (event->key() == pure_mod) {
            qWarning("discarded");
            return QPushButton::keyPressEvent(event);
        }
    }

    setKey(event->key() + event->modifiers());
}

void KeystrokeSelector::acquireKey()
{
    waitingInput_ = true;
    grabKeyboard();
    setText("Press a key combination...");
}
