
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "convmodel.h"
#include "dlli.h"
#include "dwyco_new_msg.h"
#include "getinfo.h"
#include "ignoremodel.h"
#include "dwycolist2.h"
#ifdef DWYCO_MODEL_TEST
#include <QAbstractItemModelTester>
#endif

void hack_unread_count();
void reload_conv_list();

ConvListModel *TheConvListModel;

void
Conversation::load_external_state(const QByteArray& uid)
{
    update_display(dwyco_info_to_display(uid));
    update_is_blocked(dwyco_is_ignored(uid.constData(), uid.length()));
    int reviewed = 0;
    int regular = 0;
    get_review_status(uid, reviewed, regular);
    update_REGULAR(regular);
    update_REVIEWED(reviewed);
    //update_unseen_count(uid_unviewed_msgs_count(uid));
    update_any_unread(uid_has_unviewed_msgs(uid));
    update_session_msg(got_msg_this_session(uid));
    update_pal(dwyco_is_pal(uid.constData(), uid.length()));
    update_has_hidden(dwyco_uid_has_tag(uid.constData(), uid.length(), "_hid"));
    //update_has_hidden(false);
}

ConvListModel::ConvListModel(QObject *parent) :
    QQmlObjectListModel<Conversation>(parent, "display", "uid")
{
    if(TheConvListModel)
        ::abort();
    TheConvListModel = this;
#ifdef DWYCO_MODEL_TEST
    new QAbstractItemModelTester(this);
#endif

}

ConvListModel::~ConvListModel()
{
    TheConvListModel = 0;
}



void
init_convlist_model()
{

}
void
ConvListModel::redecorate()
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        Conversation *c = at(i);
        QByteArray buid = QByteArray::fromHex(c->get_uid().toLatin1());
        c->load_external_state(buid);
    }
}

void
ConvListModel::set_all_selected(bool b)
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        Conversation *c = at(i);
        c->set_selected(b);
    }

}

bool
ConvListModel::at_least_one_selected()
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        Conversation *c = at(i);
        if(c->get_selected())
            return true;
    }
    return false;

}



void
ConvListModel::delete_all_selected()
{
    int n = count();
    QList<Conversation *> to_remove;
    for(int i = 0; i < n; ++i)
    {
        Conversation *c = at(i);
        if(c->get_selected())
        {
            to_remove.append(c);
            QByteArray buid = c->get_uid().toLatin1();
            buid = QByteArray::fromHex(buid);
            if(dwyco_is_pal(buid.constData(), buid.length()))
                continue;
            dwyco_delete_user(buid.constData(), buid.length());
            del_unviewed_uid(buid);
        }
    }

    hack_unread_count();
    reload_conv_list();

}

void
ConvListModel::pal_all_selected(bool b)
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        Conversation *c = at(i);
        if(c->get_selected())
        {
            QByteArray buid = c->get_uid().toLatin1();
            buid = QByteArray::fromHex(buid);
            if(b)
                dwyco_pal_add(buid.constData(), buid.length());
            else
                dwyco_pal_delete(buid.constData(), buid.length());
            c->update_pal(b);

        }
    }
}

void
ConvListModel::block_all_selected()
{
    int n = count();
    for(int i = 0; i < n; ++i)
    {
        Conversation *c = at(i);
        if(c->get_selected())
        {
            QByteArray buid = c->get_uid().toLatin1();
            buid = QByteArray::fromHex(buid);
            dwyco_ignore(buid.constData(), buid.length());
            TheIgnoreListModel->add_uid_to_model(buid);
            c->update_is_blocked(true);
        }
    }
}

void
ConvListModel::decorate(QString huid, QString txt, QString mid)
{
    QByteArray uid = QByteArray::fromHex(huid.toLatin1());
    Conversation *c = getByUid(huid);
    if(!c)
        return;
    //int cnt = uid_unviewed_msgs_count(uid);
    //c->update_unseen_count(cnt);
    c->update_any_unread(uid_has_unviewed_msgs(uid));
    c->update_is_blocked(dwyco_is_ignored(uid.constData(), uid.length()));
    c->update_has_hidden(dwyco_uid_has_tag(uid.constData(), uid.length(), "_hid"));
    c->update_session_msg(got_msg_this_session(uid));
}

void
ConvListModel::decorate(QString huid)
{
    decorate(huid, "", "");
}

Conversation *
ConvListModel::add_uid_to_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    Conversation *c = getByUid(huid);
    if(!c)
    {
        c = new Conversation(this);
        c->update_uid(huid);
        append(c);
    }
    c->load_external_state(uid);
    return c;
}

void
ConvListModel::remove_uid_from_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    Conversation *c = getByUid(huid);
    if(!c)
        return;
    remove(c);
}

void
ConvListModel::load_users_to_model()
{
    DWYCO_LIST l;
    int n;
    static int cnt;
    ++cnt;

    dwyco_get_user_list2(&l, &n);
    simple_scoped sl(l);
    for(int i = 0; i < n; ++i)
    {
        QByteArray uid = sl.get<QByteArray>(i);
        Conversation *c = add_uid_to_model(uid);
        c->update_counter = cnt;
    }
#if 0
    DWYCO_LIST pl = dwyco_pal_get_list();
    simple_scoped qpl(pl);
    for(int i = 0; i < qpl.rows(); ++i)
    {
        QByteArray uid = qpl.get<QByteArray>(i);
        Conversation *c = add_uid_to_model(uid);
        c->update_counter = cnt;
    }
#endif

    // find removed items
    // there is probably a faster way of doing this, but
    // given this should not happen often or with very long lists,
    // might not be worth the effort
    QList<Conversation *> dead;
    for(int i = 0; i < count(); ++i)
    {
        Conversation *c = at(i);
        if(c->update_counter < cnt)
            dead.append(c);

    }
    for(int i = 0; i < dead.count(); ++i)
    {
        Conversation *c = dead[i];
        remove(c);
    }
}



void
ConvListModel::uid_resolved(const QString &huid)
{
    Conversation *c = getByUid(huid);
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
ConvListModel::uid_invalidate_profile(const QString &huid)
{
    Conversation *c = getByUid(huid);
    if(!c)
        return;
    c->update_invalid(true);

}

ConvSortFilterModel::ConvSortFilterModel(QObject *p)
    : QSortFilterProxyModel(p)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    sort(0);
    m_count = 0;
#ifdef DWYCO_MODEL_TEST
    new QAbstractItemModelTester(this);
#endif
}

// WARNING: the index is interpreted as a SOURCE model index
// we is kinda useless in QML, now that i think about it.
// when i need this, will have to provide some mapping
QObject *
ConvSortFilterModel::get(int source_idx)
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        ::abort();
    return m->get(source_idx);
}

int
ConvSortFilterModel::get_by_uid(QString uid)
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        ::abort();
    int i = m->indexOf(uid);
    if(i == -1)
        return -1;
    return mapFromSource(m->index(i)).row();
}

void
ConvSortFilterModel::toggle_selected(QString uid)
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        ::abort();
    Conversation *c = m->getByUid(uid);
    if(!c)
        return;
    c->set_selected(!c->get_selected());
}

void
ConvSortFilterModel::set_all_selected(bool b)
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        ::abort();
    m->set_all_selected(b);
}

void
ConvSortFilterModel::delete_all_selected()
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        ::abort();
    m->delete_all_selected();

}

void
ConvSortFilterModel::block_all_selected()
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        ::abort();
    m->block_all_selected();

}

void
ConvSortFilterModel::pal_all_selected(bool b)
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        ::abort();
    m->pal_all_selected(b);

}

bool
ConvSortFilterModel::at_least_one_selected()
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        ::abort();
    return m->at_least_one_selected();

}

bool
ConvSortFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    ConvListModel *m = dynamic_cast<ConvListModel *>(sourceModel());
    if(!m)
        return false;


    bool lsm = m->data(left, m->roleForName("session_msg")).toBool();
    bool rsm = m->data(right, m->roleForName("session_msg")).toBool();
    if(lsm && !rsm)
        return true;
    else if(!lsm && rsm)
        return false;

    bool lau = m->data(left, m->roleForName("any_unread")).toBool();
    bool rau = m->data(right, m->roleForName("any_unread")).toBool();
    if(lau && !rau)
        return true;
    else if(!lau && rau)
        return false;

    bool lsp = m->data(left, m->roleForName("pal")).toBool();
    bool rsp = m->data(right, m->roleForName("pal")).toBool();
    if(lsp && !rsp)
        return true;
    else if(!lsp && rsp)
        return false;

//    bool lreg = m->data(left, m->roleForName("REGULAR")).toBool();
//    bool rreg = m->data(right, m->roleForName("REGULAR")).toBool();
//    if(lreg && !rreg)
//        return true;
//    else if(!lreg && rreg)
//        return false;

    // put blocked users down at the bottom?
    bool lreg = m->data(left, m->roleForName("is_blocked")).toBool();
    bool rreg = m->data(right, m->roleForName("is_blocked")).toBool();
    if(lreg && !rreg)
        return false;
    else if(!lreg && rreg)
        return true;

    bool ret1 = QSortFilterProxyModel::lessThan(left, right);
    bool ret2 = QSortFilterProxyModel::lessThan(right, left);
    if(ret1 == false && ret2 == false)
    {
        // stabilize the sort with uid tie breaker
        QString uidl = m->data(left, m->roleForName("uid")).toString();
        QString uidr = m->data(right, m->roleForName("uid")).toString();
        if(uidl < uidr)
            return true;
        return false;
    }
    return ret1;
}



