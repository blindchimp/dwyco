
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CHATLISTMODEL_H
#define CHATLISTMODEL_H

#include <QObject>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"

class ChatUser : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, uid)
    QML_READONLY_VAR_PROPERTY(QString, display)
    QML_READONLY_VAR_PROPERTY(bool, active)
    QML_READONLY_VAR_PROPERTY(bool, invalid)
    QML_READONLY_VAR_PROPERTY(bool, REVIEWED)
    QML_READONLY_VAR_PROPERTY(bool, REGULAR)
    QML_READONLY_VAR_PROPERTY(int, resolved_counter)
    QML_WRITABLE_VAR_PROPERTY(int, unseen_count)

public:
    ChatUser(QObject *parent = 0) : QObject(parent) {
        m_active = false;
        m_invalid = false;
        m_REVIEWED = false;
        m_REGULAR = false;
        m_resolved_counter = 0;
        m_unseen_count = 0;
    }

};

class ChatListModel : public QQmlObjectListModel<ChatUser>
{
    Q_OBJECT
public:
    explicit ChatListModel(QObject *parent = 0);
    ~ChatListModel();

    void remove_uid_from_model(const QByteArray& uid);
    ChatUser *add_uid_to_model(const QByteArray& uid);

public slots:
    // note: the invalidate is needed because we need to
    // clear the internal cache of images that may refer to uid's profile.
    void uid_invalidate_profile(const QString& huid);
    void uid_resolved(const QString& huid);
    void decorate(QString huid, QString txt, QString mid);
    void decorate(QString huid);
};

class ChatSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ChatSortFilterModel(QObject *p = 0);
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

};

extern ChatListModel *TheChatListModel;

#endif // CHATLISTMODEL_H
