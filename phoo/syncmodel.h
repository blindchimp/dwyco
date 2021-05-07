
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SYNCMODEL_H
#define SYNCMODEL_H

#include <QObject>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"

class Sync_desc : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, uid)
    QML_READONLY_VAR_PROPERTY(QString, handle)
    QML_READONLY_VAR_PROPERTY(QString, status)
    QML_READONLY_VAR_PROPERTY(QString, ip) // actual connected ip
    QML_READONLY_VAR_PROPERTY(bool, proxy)
    QML_READONLY_VAR_PROPERTY(int, asserts)
    QML_READONLY_VAR_PROPERTY(int, sendq_count)
    QML_READONLY_VAR_PROPERTY(int, percent_synced)
    QML_READONLY_VAR_PROPERTY(QString, adv_ip) // advertised ip


public:
    Sync_desc(QObject *parent = 0) : QObject(parent) {
        m_asserts = 0;
        m_proxy = false;
        m_sendq_count = 0;
        m_percent_synced = 0;
        update_counter = -1;
    }
    void load_external_state(const QByteArray& uid);
    int update_counter;
};

class SyncDescModel : public QQmlObjectListModel<Sync_desc>
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(int, connection_count)

public:
    explicit SyncDescModel(QObject *parent = 0);
    ~SyncDescModel();

    Q_INVOKABLE void load_model();

public slots:
    void update_connections(QString status);
    void uid_resolved(QString huid);

};


extern SyncDescModel *TheSyncDescModel;

#endif
