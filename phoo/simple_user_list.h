
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SIMPLEUSERMODEL_H
#define SIMPLEUSERMODEL_H

#include <QObject>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"

class SimpleUser : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, uid)
    QML_READONLY_VAR_PROPERTY(QString, display)
    QML_WRITABLE_VAR_PROPERTY(bool, active)
    QML_READONLY_VAR_PROPERTY(bool, invalid)
    QML_READONLY_VAR_PROPERTY(bool, REVIEWED)
    QML_READONLY_VAR_PROPERTY(bool, REGULAR)
    QML_READONLY_VAR_PROPERTY(bool, session_msg)
    QML_READONLY_VAR_PROPERTY(int, resolved_counter)
    QML_WRITABLE_VAR_PROPERTY(bool, selected)


public:
    SimpleUser(QObject *parent = 0) : QObject(parent) {
        m_active = false;
        m_invalid = false;
        m_REVIEWED = false;
        m_REGULAR = false;
        m_session_msg = false;
        m_resolved_counter = 0;
        m_selected = false;
        update_counter = -1;
    }
    int update_counter;
};

class SimpleUserModel : public QQmlObjectListModel<SimpleUser>
{
    Q_OBJECT
    QML_READONLY_VAR_PROPERTY(int, selected_count)

public:
    explicit SimpleUserModel(QObject *parent = 0);
    ~SimpleUserModel();

    Q_INVOKABLE void load_users_to_model();
    Q_INVOKABLE void load_admin_users_to_model();
    void remove_uid_from_model(const QByteArray& uid);
    SimpleUser * add_uid_to_model(const QByteArray& uid);

    void set_all_selected(bool);
    void delete_all_selected();
    Q_INVOKABLE void toggle_selected(QString uid);
    Q_INVOKABLE void send_forward_selected(QString uid_folder, QString mid_to_forward);

signals:

public slots:
    // note: the invalidate is needed because we need to
    // clear the internal cache of images that may refer to uid's profile.
    void uid_invalidate_profile(const QString& huid);
    void uid_resolved(const QString& huid);
    void decorate(QString huid, QString txt, QString mid);
    void decorate(QString huid);
    void decr_selected_count(QObject*);
    void mod_selected_count(bool newval);
};

// note: the source model is known to be the SimpleUserModel.
// the extra methods are for QML access during multiselect, which
// goes via model in the UI. avoids indexs and uses uids instead.
class SimpleUserSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int selected_count READ get_selected_count NOTIFY selected_countChanged)


public:
    SimpleUserSortFilterModel(QObject *p = 0);
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

    Q_INVOKABLE void load_users_to_model();
    Q_INVOKABLE void load_admin_users_to_model();
    Q_INVOKABLE void toggle_selected(QString uid);
    Q_INVOKABLE void set_all_selected(bool);
    Q_INVOKABLE void delete_all_selected();
    Q_INVOKABLE void send_forward_selected(QString uid_folder, QString mid_to_forward);

    Q_INVOKABLE int get_selected_count();

signals:
    void selected_countChanged(int);

};

#endif
