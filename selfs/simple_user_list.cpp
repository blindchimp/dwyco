
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// this model is a one-off read-only model for the user list
// that can be instantiated and used to select users, for example.
#include <QFile>
#include <QDataStream>
#include "simple_user_list.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "getinfo.h"
#include "dwyco_top.h"
#include "dwycolist2.h"

class DwycoCore;
extern DwycoCore *TheDwycoCore;

void hack_unread_count();
QByteArray get_cq_results_filename();

SimpleUserModel::SimpleUserModel(QObject *parent) :
    QQmlObjectListModel<SimpleUser>(parent, "display", "uid")
{
    m_selected_count = 0;
}

SimpleUserModel::~SimpleUserModel()
{
}

static
int
send_forward(QString recipient, QString mid_to_forward)
{
    QByteArray bmid = mid_to_forward.toLatin1();
    int compid = dwyco_make_forward_zap_composition2(bmid.constData(), 1);
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

#if 0
void
SimpleUserModel::delete_all_selected()
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        SimpleUser *c = at(i);
        if(c->get_selected())
        {
            QByteArray buid = c->get_uid().toLatin1();
            buid = QByteArray::fromHex(buid);
            if(dwyco_is_pal(buid.constData(), buid.length()))
                continue;
            dwyco_delete_user(buid.constData(), buid.length());
            del_unviewed_uid(buid);
        }
    }
    hack_unread_count();

    int total = 0;
    dwyco_load_users2(!TheDwycoCore->get_use_archived(), &total);
    TheDwycoCore->update_total_users(total);
    load_users_to_model();
}
#endif

void
SimpleUserModel::send_forward_selected(QString mid_to_forward)
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        SimpleUser *c = at(i);
        if(c->get_selected())
        {
            QString uid = c->get_uid();

            send_forward(uid, mid_to_forward);
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
    c->update_session_msg(got_msg_this_session(uid));
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
    dwyco_load_users2(1, 0);
    dwyco_get_user_list2(&l, &n);
    simple_scoped ql(l);
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = ql.get<QByteArray>(i, DWYCO_NO_COLUMN);
        if(uid.length() == 0)
            continue;
        if(!dwyco_is_ignored(uid.constData(), uid.length()))
        {
            add_uid_to_model(uid);
        }
    }
}

template<class T>
int
load_it(T& out, const char *filename)
{
    try
    {
        QFile f(filename);
        if(!f.exists())
            return 0;
        f.open(QIODevice::ReadOnly);
        QDataStream in(&f);
        in >> out;
        f.close();
        if(in.status() == QDataStream::Ok)
        {
            return 1;
        }
        f.remove();
        return 0;
    }
    catch(...)
    {
        return 0;
    }
}

void
SimpleUserModel::load_from_cq_file()
{
    int n;
    clear();
    QObject::connect(TheDwycoCore, SIGNAL(sys_uid_resolved(QString)), this, SLOT(uid_resolved(QString)), Qt::UniqueConnection);
    QList<QPair<QString, QString>> cqlist;
    if(!load_it(cqlist, get_cq_results_filename()))
        return;
    n = cqlist.count();
    for(int i = 0; i < n; ++i)
    {
        QString huid = cqlist[i].first;
        QByteArray uid = QByteArray::fromHex(huid.toLatin1());
        QString email = cqlist[i].second;
        if(uid.length() == 0)
            continue;
        if(!dwyco_is_ignored(uid.constData(), uid.length()))
        {
            auto su = add_uid_to_model(uid);
            su->update_email(email);
        }
    }
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
    c->update_invalid(false);
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
    c->update_invalid(true);

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
    connect(m, SIGNAL(countChanged()), this, SIGNAL(countChanged()));
    m_count = 0;
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
SimpleUserSortFilterModel::load_from_cq_file()
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->load_from_cq_file();
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

#if 0
void
SimpleUserSortFilterModel::delete_all_selected()
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->delete_all_selected();

}
#endif

void
SimpleUserSortFilterModel::send_forward_selected(QString mid_to_forward)
{
    SimpleUserModel *m = dynamic_cast<SimpleUserModel *>(sourceModel());
    if(!m)
        ::abort();
    m->send_forward_selected(mid_to_forward);

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

//    bool lreg = m->data(left, m->roleForName("REGULAR")).toBool();
//    bool rreg = m->data(right, m->roleForName("REGULAR")).toBool();
//    if(lreg && !rreg)
//        return true;
//    else if(!lreg && rreg)
//        return false;

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



