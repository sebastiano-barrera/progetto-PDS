#ifndef WINDOWINFO_H
#define WINDOWINFO_H

class QString;

class WindowInfo
{
public:
    WindowInfo(const QString& name) : name_(name) { }

    inline QString name() const { return name_; }

private:
    QString name_;
};

#endif // WINDOWINFO_H
