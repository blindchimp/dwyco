
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "chatlistmodel.h"
#include "dwyco_new_msg.h"
#include "getinfo.h"

ChatListModel *TheChatListModel;

ChatListModel::ChatListModel(QObject *parent) :
    QQmlObjectListModel<ChatUser>(parent, "display", "uid")
{
    if(TheChatListModel)
        ::abort();
    TheChatListModel = this;

}

ChatListModel::~ChatListModel()
{
    TheChatListModel = 0;
}

void 
ChatListModel::decorate(QString huid, QString txt, QString mid)
{
    QByteArray uid = QByteArray::fromHex(huid.toLatin1());
    ChatUser *c = getByUid(huid);
    if(!c)
        return;
    int cnt = uid_unviewed_msgs_count(uid);
    c->set_unseen_count(cnt);
}

void
ChatListModel::decorate(QString huid)
{
    decorate(huid, "", "");
}

void
ChatListModel::uid_resolved(const QString &huid)
{
    ChatUser *c = getByUid(huid);
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
ChatListModel::uid_invalidate_profile(const QString &huid)
{
    ChatUser *c = getByUid(huid);
    if(!c)
        return;
    c->update_invalid(true);

}

ChatUser *
ChatListModel::add_uid_to_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    ChatUser *c = getByUid(huid);
    if(!c)
    {
        c = new ChatUser(this);
        c->update_uid(huid);
        append(c);
    }
    c->update_display(dwyco_info_to_display(uid));
    int reviewed = 0;
    int regular = 0;
    get_review_status(uid, reviewed, regular);
    c->update_REGULAR(regular);
    c->update_REVIEWED(reviewed);
    c->set_unseen_count(uid_unviewed_msgs_count(uid));
    //c->update_resolved_counter(c->get_resolved_counter() + 1);
    return c;
}

void
ChatListModel::remove_uid_from_model(const QByteArray& uid)
{
    QString huid = uid.toHex();
    ChatUser *c = getByUid(huid);
    if(!c)
        return;
    // XXX note: maybe just parent to model and let it delete
    remove(c);
    //delete c;  
}

ChatSortFilterModel::ChatSortFilterModel(QObject *p)
    : QSortFilterProxyModel(p)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    // note: this is totally non-obvious from the docs that you
    // have to call sort once to effectively "tell it to use this column from now on".
    sort(0);
}

bool
ChatSortFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    ChatListModel *m = dynamic_cast<ChatListModel *>(sourceModel());
    if(!m)
        return false;

    int luc = m->data(left, m->roleForName("unseen_count")).toInt();
    int ruc = m->data(right, m->roleForName("unseen_count")).toInt();
    if(luc < ruc)
        return false;
    else if(ruc < luc)
        return true;

    bool lreg = m->data(left, m->roleForName("REGULAR")).toBool();
    bool rreg = m->data(right, m->roleForName("REGULAR")).toBool();
    if(lreg && !rreg)
        return true;
    else if(!lreg && rreg)
        return false;

    return QSortFilterProxyModel::lessThan(left, right);

#if 1
#if 0
printf("lt %s (%d, %d) %s (%d, %d)\n",
    m->data(left).toString().toAscii().constData(), left.row(), left.column(),
    m->data(right).toString().toAscii().constData(), right.row(), right.column());
#endif


//    QByteArray quid1 = m->data(left, UID).toByteArray();
//    quid1 = QByteArray::fromHex(quid1);
//    QByteArray quid2 = m->data(right, UID).toByteArray();
//    quid2 = QByteArray::fromHex(quid2);

//    if(quid1.length() == 0 || quid2.length() == 0)
//        return 0;

//    int lt = less_than(quid1, quid2);

//    if(lt == 0)
//    {
//        int ret1;
//        int ret2;
//        ret1 =  QSortFilterProxyModel::lessThan(left, right);
//        ret2 =  QSortFilterProxyModel::lessThan(right, left);

//        if(ret1 == 0 && ret2 == 0)
//        {
//            // this should stabilize the sort so users that
//            // names look the same at least always sort to the
//            // same relative place (before, they would just
//            // randomly rearrange and be a pain in the butt to figure out.)
//            if(quid1 < quid2)
//                return 1;
//            return 0;
//        }
//        return ret1;
//    }
//    return lt < 0;
#endif
}



