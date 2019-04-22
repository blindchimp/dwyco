
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// this model is a one-off read-only model for the user list
// that can be instantiated and used to select users, for example.
#include "simple_user_list.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "getinfo.h"
#include "dwyco_top.h"

class DwycoCore;
extern DwycoCore *TheDwycoCore;

void hack_unread_count();

SimpleUserModel::SimpleUserModel(QObject *parent) :
    QQmlObjectListModel<SimpleUser>(parent, "display", "uid")
{
    m_selected_count = 0;
}

SimpleUserModel::~SimpleUserModel()
{
}

static QByteArray
dwyco_get_attr(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        ::abort();
    if(type != DWYCO_TYPE_STRING && type != DWYCO_TYPE_NIL)
        ::abort();
    return QByteArray(val, len);
}

static int
dwyco_get_attr_int(DWYCO_LIST l, int row, const char *col, int& int_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_INT)
        return 0;
    QByteArray str_out = QByteArray(val, len);
    int_out = str_out.toInt();
    return 1;
}

static
int
send_forward(QString recipient, QString uid_folder, QString mid_to_forward)
{
    QByteArray uid_f = QByteArray::fromHex(uid_folder.toLatin1());
    QByteArray bmid = mid_to_forward.toLatin1();
    int compid = dwyco_make_forward_zap_composition(uid_f.constData(), uid_f.length(), bmid.constData(), 1);
    if(compid == 0)
        return 0;
    QByteArray ruid = QByteArray::fromHex(recipient.toLatin1());
    QByteArray txt;
    if(!dwyco_zap_send4(compid, ruid.constData(), ruid.length(),
                        txt.constData(), txt.length(), 0,
                        0, 0)
      )

    {
        dwyco_delete_zap_composition(compid);
        return 0;
    }
    return compid;

}



void
SimpleUserModel::set_all_selected(bool b)
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        SimpleUser *c = at(i);
        c->set_selected(b);
    }

}

void
SimpleUserModel::delete_all_selected()
{
    int n = count();
    //QList<SimpleUser *> to_remove;
    for(int i = 0; i < n; ++i)
    {
        SimpleUser *c = at(i);
        if(c->get_selected())
        {
            //to_remove.append(c);
            QByteArray buid = c->get_uid().toLatin1();
            buid = QByteArray::fromHex(buid);
            if(dwyco_is_pal(buid.constData(), buid.length()))
                continue;
            dwyco_delete_user(buid.constData(), buid.length());
            del_unviewed_uid(buid);
        }
    }
    hack_unread_count();

    //dwyco_load_users2(1, 0);
    int total = 0;
    dwyco_load_users2(TheDwycoCore->get_use_archived() ? 0 : 1, &total);
    TheDwycoCore->update_total_users(total);
    load_users_to_model();

}

void
SimpleUserModel::send_forward_selected(QString uid_folder, QString mid_to_forward)
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        SimpleUser *c = at(i);
        if(c->get_selected())
        {
            QString uid = c->get_uid();

            send_forward(uid, uid_folder, mid_to_forward);
        }
    }

}

void
SimpleUserModel::toggle_selected(QString uid)
{
    SimpleUser *c = getByUid(uid);
    if(!c)
        return;
    c->set_selected(!c->get_selected());
}

void
SimpleUserModel::decorate(QString huid, QString txt, QString mid)
{
    QByteArray uid = QByteArray::fromHex(huid.toLatin1());
    SimpleUser *c = getByUid(huid);
    if(!c)
        return;


}

void
SimpleUserModel::decorate(QString huid)
{
    decorate(huid, "", "");
}

void
SimpleUserModel::decr_selected_count(QObject *o)
{
    SimpleUser *c = dynamic_cast<SimpleUser *>(o);
    if(!c)
        return;
    if(c->get_selected())
        update_selected_count(get_selected_count() - 1);
}

void
SimpleUserModel::mod_selected_count(bool newval)
{
    update_selected_count(get_selected_count() + (newval ? 1 : -1));
}

SimpleUser *
SimpleUserModel::add_uid_to_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    SimpleUser *c = getByUid(huid);
    if(!c)
    {
        c = new SimpleUser(this);
        connect(c, SIGNAL(destroyed(QObject*)), this, SLOT(decr_selected_count(QObject*)));
        connect(c, SIGNAL(selectedChanged(bool)), this, SLOT(mod_selected_count(bool)));

        c->update_uid(huid);
        append(c);
    }
    c->update_display(dwyco_info_to_display(uid));
    int reviewed = 0;
    int regular = 0;
    get_review_status(uid, reviewed, regular);
    c->update_REGULAR(regular);
    c->update_REVIEWED(reviewed);
    c->update_session_msg(session_msg(uid));
    return c;
}

void
SimpleUserModel::remove_uid_from_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    SimpleUser *c = getByUid(huid);
    if(!c)
        return;
    remove(c);
}

void
SimpleUserModel::load_users_to_model()
{
    DWYCO_LIST l;
    int n;
    clear();
    QObject::connect(TheDwycoCore, SIGNAL(sys_uid_resolved(QString)), this, SLOT(uid_resolved(QString)), Qt::UniqueConnection);
    dwyco_get_user_list2(&l, &n);
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = dwyco_get_attr(l, i, DWYCO_NO_COLUMN);
        if(!dwyco_is_ignored(uid.constData(), uid.length()))
        {
            add_uid_to_model(uid);
        }
    }
    dwyco_list_release(l);
}

void
SimpleUserModel::load_admin_users_to_model()
{
    clear();
    connect(TheDwycoCore, SIGNAL(sys_uid_resolved(QString)), this, SLOT(uid_resolved(QString)), Qt::UniqueConnection);
    add_uid_to_model(QByteArray::fromHex("5a098f3df49015331d74"));
}



void
SimpleUserModel::uid_resolved(const QString &huid)
{
    SimpleUser *c = getByUid(huid);
    if(!c)
        return;

    QByteArray buid = QByteArray::fromHex(huid.toLatin1());
    c->update_display(dwyco_info_to_display(buid));
    c->update_invalid(0);
    int regular = 0;
    int reviewed = 0;
    get_review_status(buid, reviewed, regular);
    c->update_REGULAR(regular);
    c->update_REVIEWED(reviewed);
    c->update_resolved_counter(c->get_resolved_counter() + 1);
}

void
SimpleUserModel::uid_invalidate_profile(const QString &huid)
{
    SimpleUser *c = getByUid(huid);
    if(!c)
        return;
    c->update_invalid(1);

}

SimpleUserSortFilterModel::SimpleUserSortFilterModel(QObject *p)
    : QSortFilterProxyModel(p)
{
    SimpleUserModel *m = new SimpleUserModel(this);
    setSourceModel(m);
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0);
    connect(m, SIGNAL(selected_countChanged(int)), this, SIGNAL(selected_countChanged(int)));
}

int
SimpleUserSortFilterModel::get_selected_count()
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    return m->get_selected_count();
}

void
SimpleUserSortFilterModel::load_users_to_model()
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->load_users_to_model();
}

void
SimpleUserSortFilterModel::load_admin_users_to_model()
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->load_admin_users_to_model();
}

void
SimpleUserSortFilterModel::toggle_selected(QString uid)
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->toggle_selected(uid);
}

void
SimpleUserSortFilterModel::set_all_selected(bool b)
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->set_all_selected(b);
}

void
SimpleUserSortFilterModel::delete_all_selected()
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->delete_all_selected();

}

void
SimpleUserSortFilterModel::send_forward_selected(QString uid_folder, QString mid_to_forward)
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->send_forward_selected(uid_folder, mid_to_forward);

}


bool
SimpleUserSortFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        return false;

    bool lsm = m->data(left, m->roleForName("session_msg")).toBool();
    bool rsm = m->data(right, m->roleForName("session_msg")).toBool();
    if(lsm && !rsm)
        return true;
    else if(!lsm && rsm)
        return false;

    bool lreg = m->data(left, m->roleForName("REGULAR")).toBool();
    bool rreg = m->data(right, m->roleForName("REGULAR")).toBool();
    if(lreg && !rreg)
        return true;
    else if(!lreg && rreg)
        return false;

    int ret1 = QSortFilterProxyModel::lessThan(left, right);
    int ret2 = QSortFilterProxyModel::lessThan(right, left);
    if(ret1 == 0 && ret2 == 0)
    {
        // stabilize the sort with uid tie breaker
        QString uidl = m->data(left, m->roleForName("uid")).toString();
        QString uidr = m->data(right, m->roleForName("uid")).toString();
        if(uidl < uidr)
            return 1;
        return 0;
    }
    return ret1;
}



