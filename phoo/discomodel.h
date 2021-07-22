
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DISCOMODEL_H
#define DISCOMODEL_H

#include <QObject>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"

class DiscoveredUser : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, uid)
    QML_READONLY_VAR_PROPERTY(QString, display)
    QML_READONLY_VAR_PROPERTY(bool, online)
    QML_READONLY_VAR_PROPERTY(int, invalid)
    QML_READONLY_VAR_PROPERTY(int, resolved_counter)

public:
    DiscoveredUser(QObject *parent = 0) : QObject(parent) {
        m_resolved_counter = 0;
        m_invalid = 0;
        m_online = false;
    }
};

class DiscoverListModel : public QQmlObjectListModel<DiscoveredUser>
{
    Q_OBJECT
public:
    explicit DiscoverListModel(QObject *parent = 0);
    ~DiscoverListModel();

    Q_INVOKABLE void load_users_to_model();
    void remove_uid_from_model(const QByteArray& uid);
    DiscoveredUser * add_uid_to_model(const QByteArray& uid);

signals:

public slots:
    // note: the invalidate is needed because we need to
    // clear the internal cache of images that may refer to uid's profile.
    void uid_invalidate_profile(const QString& huid);
    void uid_resolved(const QString& huid);
    //void decorate(QString huid, QString txt, QString mid);
    //void decorate(QString huid);
};


extern DiscoverListModel *TheDiscoverListModel;

#endif
