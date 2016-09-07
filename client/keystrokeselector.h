#ifndef KEYSTROKESELECTOR_H
#define KEYSTROKESELECTOR_H

#include <QComboBox>
#include <QKeySequence>

#include <memory>

class QKeyEvent;
class QCloseEvent;
namespace Ui { class KeystrokeSelector; }


class KeystrokeSelector : public QWidget
{
    Q_OBJECT

    // even though a QKeySequence object can store a sequence, we only ever
    // store one single key in it. We use the QKeySequence class to take
    // advantage of some of its methods
    QKeySequence key_;
    bool waitingInput_;
    std::unique_ptr<Ui::KeystrokeSelector> ui_;
    QPalette successPalette_, failurePalette_;

public:
    explicit KeystrokeSelector(QWidget *parent = 0);
    ~KeystrokeSelector();

    inline int key() const { return key_[0]; }
    void setKey(int key);

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;

signals:
    void keyChanged(int key);

private slots:
    void acquireKey();
    void setKeyFromString(const QString&);

private:
    void updateText();
};

#endif // KEYSTROKESELECTOR_H
