#ifndef IOSCAMERA_H
#define IOSCAMERA_H

#ifdef DWYCO_IOS

#include <QObject>

class IOSCamera : public QObject
{
    Q_OBJECT
public:
    explicit IOSCamera(QObject *parent = nullptr);
    Q_INVOKABLE void takePicture();
};

#endif

#endif // IOSCAMERA_H