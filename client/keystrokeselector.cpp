#include "keystrokeselector.h"
#include "ui_keystrokeselector.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <iostream>


KeystrokeSelector::KeystrokeSelector(QWidget *parent) :
    QWidget(parent),
    waitingInput_(false),
    ui_(new Ui::KeystrokeSelector())
{
    ui_->setupUi(this);

    connect(ui_->btnGrab, &QPushButton::clicked, this, &KeystrokeSelector::acquireKey);
    connect(ui_->comboBox, &QComboBox::editTextChanged, this, &KeystrokeSelector::setKeyFromString);

    successPalette_ = ui_->comboBox->lineEdit()->palette();
    successPalette_.setColor(QPalette::Text, Qt::darkGreen);
    failurePalette_ = ui_->comboBox->lineEdit()->palette();
    failurePalette_.setColor(QPalette::Text, Qt::darkRed);
}

KeystrokeSelector::~KeystrokeSelector() { }

void KeystrokeSelector::setKey(int key)
{
    waitingInput_ = false;
    ui_->btnGrab->setText("Grab");
    releaseKeyboard();

    key_ = QKeySequence(key);
    Q_ASSERT (key_.count() == 1);

    ui_->comboBox->blockSignals(true);
    ui_->comboBox->setPalette(successPalette_);
    ui_->comboBox->lineEdit()->setText(key_.toString());
    ui_->comboBox->blockSignals(false);

    ui_->comboBox->addItem(key_.toString());

    emit keyChanged(key_[0]);
}

void KeystrokeSelector::keyPressEvent(QKeyEvent* event)
{
    if (!waitingInput_)
        return QWidget::keyPressEvent(event);

    // the purpose of this array is simply to discard keystrokes amounting to a
    // press of just a modifier key.
    static const Qt::Key pure_modifiers[] = {
        Qt::Key_Control, Qt::Key_Shift, Qt::Key_Alt, Qt::Key_Meta, Qt::Key_AltGr,
    };

    qWarning("Key: <0x%08x>", event->key());

    for (Qt::Key pure_mod : pure_modifiers) {
        if (event->key() == pure_mod) {
            qWarning("discarded");
            return QWidget::keyPressEvent(event);
        }
    }

    setKey(event->key() + event->modifiers());
}

void KeystrokeSelector::acquireKey()
{
    waitingInput_ = true;
    ui_->btnGrab->setText("Press a key combination...");
    grabKeyboard();
}

void KeystrokeSelector::setKeyFromString(const QString &keyDesc)
{
    auto keySeq = QKeySequence::fromString(keyDesc, QKeySequence::PortableText);

    bool invalid = keySeq.isEmpty() || keySeq[0] == Qt::Key_unknown;
    std::cerr << "Parsing: " << keyDesc.toStdString()
              << " -> " << keySeq.toString().toStdString()
              << " invalid: " << invalid << '\n';

    if (invalid) {
        ui_->comboBox->setPalette(failurePalette_);
        return;
    }

    setKey(keySeq[0]);
}
