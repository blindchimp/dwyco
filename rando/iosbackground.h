#ifndef IOSBACKGROUND_H
#define IOSBACKGROUND_H

#ifdef DWYCO_IOS

#include <QObject>

class IOSBackground : public QObject
{
    Q_OBJECT
public:
    explicit IOSBackground(QObject *parent = nullptr);
    Q_INVOKABLE quint64 begin();
    Q_INVOKABLE void end(quint64 taskId);
signals:
    void expired();
};

#endif

#endif // IOSBACKGROUND_H