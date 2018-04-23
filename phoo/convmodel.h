
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CONVMODEL_H
#define CONVMODEL_H

#include <QObject>
#include "QSortFilterProxyModel"
#include "QQmlObjectListModel.h"
#include "QQmlVarPropertyHelpers.h"

class Conversation : public QObject
{
    Q_OBJECT

    QML_READONLY_VAR_PROPERTY(QString, uid)
    QML_READONLY_VAR_PROPERTY(QString, display)
    QML_WRITABLE_VAR_PROPERTY(bool, active)
    QML_READONLY_VAR_PROPERTY(bool, invalid)
    QML_READONLY_VAR_PROPERTY(bool, REVIEWED)
    QML_READONLY_VAR_PROPERTY(bool, REGULAR)
    QML_READONLY_VAR_PROPERTY(int, resolved_counter)
    QML_READONLY_VAR_PROPERTY(int, unseen_count)
    QML_READONLY_VAR_PROPERTY(bool, is_blocked)
    QML_READONLY_VAR_PROPERTY(bool, any_unread)
    QML_READONLY_VAR_PROPERTY(bool, session_msg)
    QML_READONLY_VAR_PROPERTY(bool, pal)
    QML_WRITABLE_VAR_PROPERTY(bool, selected)
    

public:
    Conversation(QObject *parent = 0) : QObject(parent) {
        m_active = 0;
        m_invalid = 0;
        m_REVIEWED = 0;
        m_REGULAR = 0;
        m_resolved_counter = 0;
        m_unseen_count = 0;
        m_is_blocked = false;
        m_any_unread = false;
        m_session_msg = false;
        m_selected = 0;
        m_pal = false;
        update_counter = -1;
    }
    void load_external_state(const QByteArray& uid);
    int update_counter;
};

class ConvListModel : public QQmlObjectListModel<Conversation>
{
    Q_OBJECT
public:
    explicit ConvListModel(QObject *parent = 0);
    ~ConvListModel();
    
    void load_users_to_model();
    void remove_uid_from_model(const QByteArray& uid);
    Conversation * add_uid_to_model(const QByteArray& uid);
    
    void set_all_selected(bool);
    void delete_all_selected();
    void pal_all_selected(bool);
    void block_all_selected();
    bool at_least_one_selected();

signals:

public slots:
    // note: the invalidate is needed because we need to
    // clear the internal cache of images that may refer to uid's profile.
    void uid_invalidate_profile(const QString& huid);
    void uid_resolved(const QString& huid);
    void decorate(QString huid, QString txt, QString mid);
    void decorate(QString huid);
};

// note: the source model is known to be the ConvListModel.
// the extra methods are for QML access during multiselect, which
// goes via model in the UI. avoids indexs and uses uids instead.
class ConvSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY (int count READ count NOTIFY countChanged)

public:
    ConvSortFilterModel(QObject *p = 0);
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;

    int count() const {
        if(sourceModel()) {
            return dynamic_cast<ConvListModel *>(sourceModel())->count();
        }
        return 0;
    }
    
    Q_INVOKABLE void toggle_selected(QString uid);
    Q_INVOKABLE void set_all_selected(bool);
    Q_INVOKABLE void delete_all_selected();
    Q_INVOKABLE void pal_all_selected(bool);
    Q_INVOKABLE void block_all_selected();
    Q_INVOKABLE bool at_least_one_selected();

private:
    int m_count;
signals:
    void countChanged();
    
    
};

extern ConvListModel *TheConvListModel;

#endif 
