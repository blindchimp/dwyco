#ifndef ANDROIDPERMS_H
#define ANDROIDPERMS_H

#include <QObject>
#include "QQmlVarPropertyHelpers.h"

class AndroidPerms : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(bool, camera_permission)
    QML_READONLY_VAR_PROPERTY(bool, external_storage_permission)

public:
    explicit AndroidPerms(QObject *parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void toggle();
    Q_INVOKABLE bool request_sync(QString);

signals:

public slots:
};

#endif // ANDROIDPERMS_H