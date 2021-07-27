
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CTLIST_H
#define CTLIST_H

#include <QObject>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"

class SimpleContact : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, uid)
    QML_READONLY_VAR_PROPERTY(QString, display)
    QML_READONLY_VAR_PROPERTY(QString, phone)
    QML_READONLY_VAR_PROPERTY(QString, email)
    QML_WRITABLE_VAR_PROPERTY(bool, selected)


public:
    SimpleContact(QObject *parent = 0) : QObject(parent) {
        m_selected = false;
    }
};

class SimpleContactModel : public QQmlObjectListModel<SimpleContact>
{
    Q_OBJECT
    QML_READONLY_VAR_PROPERTY(int, selected_count)

public:
    explicit SimpleContactModel(QObject *parent = 0);
    ~SimpleContactModel();

    Q_INVOKABLE void load_users_to_model();
    SimpleContact * add_contact_to_model(const QByteArray& name, const QByteArray& phone, const QByteArray& email);

    void set_all_selected(bool);

    Q_INVOKABLE void toggle_selected(int r);
    Q_INVOKABLE void send_query();

public slots:
    void decr_selected_count(QObject*);
    void mod_selected_count(bool newval);
};

#endif
