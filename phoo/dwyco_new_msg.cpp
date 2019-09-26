
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// call this function on "rescan msgs" and it will
// return the list of new msgs received
//

#include <QSet>
#include <QFile>
#include <QDataStream>
#include "dwyco_new_msg.h"
#include "dlli.h"
#include "pfx.h"
#include "dwycolistscoped.h"

QString dwyco_info_to_display(QByteArray uid);
static QSet<QByteArray> Got_msg_from;
static QSet<QByteArray> Got_msg_from_this_session;
typedef QHash<QByteArray, QByteArray> UID_MID_MAP;
static UID_MID_MAP Unviewed_msgs;

int
save_unviewed()
{
    QFile qf(add_pfx(User_pfx, "unviewed.qts"));
    if(!qf.open(QFile::Truncate|QFile::WriteOnly))
        return 0;
    QDataStream ds(&qf);
    ds << Unviewed_msgs;
    if(ds.status() != QDataStream::Ok)
    {
        return 0;
    }
    return 1;
}

void
add_unviewed(const QByteArray& uid, const QByteArray& mid, int no_save)
{
    QList<QByteArray> ql = Unviewed_msgs.values(uid);
    if(ql.contains(mid))
        return;
    Unviewed_msgs.insertMulti(uid, mid);
    // note: ideally we would sort it in by the time the msg was sent, tho this
    // method usually works (messages can appear from different
    // sources like server vs. direct at different times.)
    if(!no_save)
        save_unviewed();
}

void
load_inbox_tags_to_unviewed(QSet<QByteArray>& uids_out)
{
    // use this after resume to make sure new messages
    // that were received are flagged properly
    DWYCO_LIST tm;
    if(!dwyco_get_tagged_mids(&tm, "_inbox"))
        return;
    simple_scoped qtm(tm);
    for(int i = 0; i < qtm.rows(); ++i)
    {
        QByteArray uid = QByteArray::fromHex(qtm.get<QByteArray>(i, DWYCO_TAGGED_MIDS_HEX_UID));
        QByteArray mid = qtm.get<QByteArray>(i, DWYCO_TAGGED_MIDS_MID);
        add_unviewed(uid, mid);
        uids_out.insert(uid);
    }
    dwyco_unset_all_msg_tag("_inbox");
}

int
load_unviewed()
{
    QFile qf(add_pfx(User_pfx, "unviewed.qts"));
    if(!qf.open(QFile::ReadOnly))
        return 0;
    QDataStream ds(&qf);
    ds >> Unviewed_msgs;
    // this just serves to preen out any messages that might be missing
    // so the user doesn't end up with notifications about messages
    // that don't exist anymore.
    reload_msgs();
    UID_MID_MAP::const_iterator i = Unviewed_msgs.constBegin();
    while(i != Unviewed_msgs.constEnd())
    {
        Got_msg_from.insert(i.key());
        ++i;
    }
    return 1;
}

int
session_msg(const QByteArray &uid)
{
    return Got_msg_from_this_session.contains(uid);
}

void
clear_session_msg()
{
    Got_msg_from_this_session.clear();
}

int
any_unread_msg(const QByteArray& uid)
{
    return Got_msg_from.contains(uid);
}

void
del_unviewed_uid(const QByteArray& uid)
{
    Unviewed_msgs.remove(uid);
    save_unviewed();
}

void
del_unviewed_mid(const QByteArray& uid, const QByteArray& mid)
{
    UID_MID_MAP::iterator i = Unviewed_msgs.find(uid);
    while(i != Unviewed_msgs.end() && i.key() == uid)
    {
        // don't break on match, kill all mids with that uid,mid pair
        // just in case there is some corruption in the list
        if(i.value() == mid)
            i = Unviewed_msgs.erase(i);
        else
            ++i;
    }
    save_unviewed();
}

void
del_unviewed_mid(const QByteArray& mid)
{
    UID_MID_MAP::iterator i = Unviewed_msgs.begin();
    while(i != Unviewed_msgs.end())
    {
        if(i.value() == mid)
        {
            i = Unviewed_msgs.erase(i);
            break;
        }
        else
            ++i;
    }
    save_unviewed();
}

int
uid_has_unviewed_msgs(const QByteArray &uid)
{
    return Unviewed_msgs.contains(uid);
}

int
uid_unviewed_msgs_count(const QByteArray &uid)
{
    return Unviewed_msgs.count(uid);
}

int
has_unviewed_msgs()
{
    return Unviewed_msgs.count();
}

void
clear_unviewed_msgs()
{
    Unviewed_msgs.clear();
    save_unviewed();
}

static int
dwyco_get_attr(DWYCO_LIST l, int row, const char *col, QByteArray& str_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    str_out = QByteArray(val, len);
    return 1;
}


// this is used on initial start up to re-establish the
// state of the ui with msgs that you haven't viewed yet.
int
reload_msgs()
{
    QList<QByteArray> kill_list_uid;
    QList<QByteArray> kill_list_mid;

    // note: qt docs say ordering of keys and values is the same
    QList<QByteArray> k = Unviewed_msgs.keys();
    QList<QByteArray> v = Unviewed_msgs.values();
    int n = k.count();

    for(int i = 0; i < n; ++i)
    {
        DWYCO_SAVED_MSG_LIST qsm;
        if(dwyco_get_saved_message(&qsm, k[i].constData(), k[i].length(), v[i].constData()))
        {
            dwyco_list_release(qsm);
        }
        else
        {
            kill_list_uid.append(k[i]);
            kill_list_mid.append(v[i]);
        }

    }
    n = kill_list_uid.count();
    for(int i = 0; i < n; ++i)
    {
        del_unviewed_mid(kill_list_uid[i], kill_list_mid[i]);
    }
    return 1;
}



int
dwyco_process_unfetched_list(DWYCO_UNFETCHED_MSG_LIST ml, QSet<QByteArray>& uids)
{
    int n;
    const char *val;
    int len;
    int type;
    dwyco_list_numelems(ml, &n, 0);
    if(n == 0)
    {
        return 0;
    }
    QByteArray uid_out;
    QByteArray mid;
    for(int i = 0; i < n; ++i)
    {
        if(!dwyco_get_attr(ml, i, DWYCO_QMS_FROM, uid_out))
            continue;
        if(uid_out == QByteArray::fromHex("f6006af180260669eafc"))
            continue;
        if(!dwyco_get_attr(ml, i, DWYCO_QMS_ID, mid))
            continue;
        if(!dwyco_list_get(ml, i, DWYCO_QMS_IS_DIRECT, &val, &len, &type))
            continue;

        if(type != DWYCO_TYPE_NIL)
        {
            ::abort();
        }

        // ok, at least it is likely the person will see it now
        // if there are any errors above, people may not be able to see a message
        // but the user would appear towards the top of the user list, which is weird.
        // this happens sometimes when attachments are not fetchable for whatever reason.
        Got_msg_from.insert(uid_out);
        Got_msg_from_this_session.insert(uid_out);
        add_unviewed(uid_out, mid);
        uids.insert(uid_out);
    }

    return 0;
}

