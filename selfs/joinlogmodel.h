
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef JOINLOGMODEL_H
#define JOINLOGMODEL_H

#include <QObject>
#include <QDateTime>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"

class Join_log_entry : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, uid)
    QML_READONLY_VAR_PROPERTY(QString, handle)
    QML_READONLY_VAR_PROPERTY(QString, msg)
    QML_READONLY_VAR_PROPERTY(QDateTime, time)


public:
    Join_log_entry(QObject *parent = 0) : QObject(parent) {

    }
    void load_external_state(const QByteArray& uid);

};

class JoinLogModel : public QQmlObjectListModel<Join_log_entry>
{
    Q_OBJECT

public:
    explicit JoinLogModel(QObject *parent = 0);
    ~JoinLogModel();

    Q_INVOKABLE void load_model();

public slots:
    void uid_resolved(QString huid);

};


extern JoinLogModel *TheJoinLogModel;

#endif
