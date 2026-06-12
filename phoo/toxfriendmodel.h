
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef TOXFRIENDMODEL_H
#define TOXFRIENDMODEL_H

#include <QObject>
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"

class ToxFriend : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(int, friend_number)
    QML_READONLY_VAR_PROPERTY(QString, pubkey)
    QML_READONLY_VAR_PROPERTY(QString, name)
    QML_READONLY_VAR_PROPERTY(QString, status)
    QML_READONLY_VAR_PROPERTY(QString, user_status)

public:
    ToxFriend(QObject *parent = 0) : QObject(parent) {
        m_friend_number = 0;
    }
    int update_counter = 0;
};

class ToxFriendModel : public QQmlObjectListModel<ToxFriend>
{
    Q_OBJECT
public:
    explicit ToxFriendModel(QObject *parent = 0);
    ~ToxFriendModel();

    Q_INVOKABLE void load_friends();
};

#endif
