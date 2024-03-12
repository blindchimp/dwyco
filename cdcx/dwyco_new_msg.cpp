
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
#include <QFile>
#include <stdio.h>
#include "dwyco_new_msg.h"
#include "dlli.h"
#include "mainwin.h"
#include "tfhex.h"
#include "pfx.h"
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "dwycolistscoped.h"

static QList<QByteArray> fetching;
QString dwyco_info_to_display(QByteArray uid);
QSet<QByteArray> Got_msg_from;
extern QSet<DwOString> Session_remove;
static QSet<QByteArray> Got_msg_from_this_session;
static QSet<QByteArray> Dont_refetch;
static QList<QByteArray> Delete_msgs;
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
    Unviewed_msgs.insertMulti(uid, mid);
    // note: ideally we would sort it in by the time the msg was sent, tho this
    // method usually works (messages can appear from different
    // sources like server vs. direct at different times.)
    if(!no_save)
        save_unviewed();
}

int
load_unviewed()
{
    QFile qf(add_pfx(User_pfx, "unviewed.qts"));
    if(!qf.open(QFile::ReadOnly))
        return 0;
    QDataStream ds(&qf);
    ds >> Unviewed_msgs;
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
    return Unviewed_msgs.count() > 0;
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

void
DWYCOCALLCONV
msg_callback(int id, int what, const char *mid, void *)
{
//printf("id %d what %d mid %s\n", id, what, mid);
    int i = fetching.indexOf(mid);
    switch(what)
    {
    case DWYCO_MSG_DOWNLOAD_FETCHING_ATTACHMENT:
        // here is a case where it makes sense to provide the text of the message
        // immediately while the attachment is being downloaded in the background.
        return;

    case DWYCO_MSG_DOWNLOAD_RATHOLED:
    case DWYCO_MSG_DOWNLOAD_DECRYPT_FAILED:
        // if this msg is ratholed, it can never be fetched, so just delete it.
        Dont_refetch.insert(mid);
        if(i >= 0)
            fetching.removeAt(i);
        Delete_msgs.append(mid);
        del_unviewed_mid(mid);
        return;

    case DWYCO_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED:
    case DWYCO_MSG_DOWNLOAD_SAVE_FAILED:
    case DWYCO_MSG_DOWNLOAD_FAILED:

        // this is potentially just a transient failure, so maybe try refetching
        // a certain number of times. really need something from the server that says
        // whether there is any hope of getting the message or not.
        // i think i will handle this case on the server and just delete messages that
        // have not been fetched in a month or something. this will cause a bit of thrashing for
        // users that have a lot of messages that can't be fetched, but gives the best chance to get
        // a message in transient failure situations.
        Dont_refetch.insert(mid);
        del_unviewed_mid(mid);

#ifdef NO_SAVED_MSGS
        dwyco_delete_unfetched_message(mid);
#endif
        if(i >= 0)
            fetching.removeAt(i);
        return;
    case DWYCO_MSG_DOWNLOAD_OK:
    default:
        if(i >= 0)
            fetching.removeAt(i);
    }
}

// this is used on initial start up to re-establish the
// state of the ui with msgs that you haven't viewed yet.
int
reload_msgs()
{
    QList<QByteArray> kill_list_uid;
    QList<QByteArray> kill_list_mid;

    QList<QByteArray> k = Unviewed_msgs.keys();
    QList<QByteArray> v = Unviewed_msgs.values();
    int n = k.count();


    for(int i = 0; i < n; ++i)
    {
        DWYCO_SAVED_MSG_LIST sm;
        if(dwyco_get_saved_message(&sm, k[i].constData(), k[i].length(), v[i].constData()))
        {
            DWYCO_LIST bt = dwyco_get_body_text(sm);
            QByteArray txt;
            if(dwyco_get_attr(bt, 0, DWYCO_NO_COLUMN, txt))
            {
                display_msg(k[i], txt, v[i]);
                dwyco_list_release(bt);
            }
            else
            {
                kill_list_uid.append(k[i]);
                kill_list_mid.append(v[i]);

            }
            dwyco_list_release(sm);
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
dwyco_new_msg(DwOString& uid_out, DwOString& txt, int& zap_viewer, DwOString& mid)
{
    zap_viewer = 0;

    int k = Delete_msgs.count();
    for(int i = 0; i < k; ++i)
    {
        dwyco_delete_unfetched_message(Delete_msgs[i].constData());

    }
    Delete_msgs.clear();

    DWYCO_UNFETCHED_MSG_LIST ml;
    if(!dwyco_get_unfetched_messages(&ml, 0, 0))
        return 0;
    simple_scoped qml(ml);
    int n;
    const char *val;
    int len;
    int type;
    n = qml.rows();

    for(int i = 0; i < n; ++i)
    {
        if(!dwyco_get_attr(ml, i, DWYCO_QMS_FROM, uid_out))
            continue;
        if(!dwyco_get_attr(ml, i, DWYCO_QMS_ID, mid))
            continue;
        if(Dont_refetch.contains(mid) ||
                fetching.contains(mid) || fetching.count() >= 5)
            continue;
        if(!dwyco_list_get(ml, i, DWYCO_QMS_IS_DIRECT, &val, &len, &type))
            continue;
        if(type == DWYCO_TYPE_NIL)
        {
            int special_type;
            const char *uid;
            int len_uid;
            const char *dlv_mid;
            if(dwyco_is_delivery_report(mid.constData(), &uid, &len_uid, &dlv_mid, &special_type))
            {
                // process pal authorization stuff here
                if(special_type == DWYCO_SUMMARY_DELIVERED)
                {
                    // hmmm, need new api to get uid/mid of delivered msg
                    dwyco_delete_unfetched_message(mid.constData());
                    continue;
                }

            }
            // issue a server fetch, client will have to
            // come back in to get it when the fetch is done
            fetching.append(mid);
            dwyco_fetch_server_message(mid.constData(), msg_callback, 0, 0, 0);
            continue;
        }
    }

    DWYCO_LIST tm;
    dwyco_get_tagged_mids(&tm, "_inbox");
    simple_scoped qtm(tm);
    if(qtm.rows() == 0 && n == 0)
        return 0;
    if(qtm.rows() == 0)
        return 0;

    uid_out = qtm.get<QByteArray>(0, DWYCO_TAGGED_MIDS_HEX_UID);
    uid_out = QByteArray::fromHex(uid_out);
    QByteArray tmid = qtm.get<QByteArray>(0, DWYCO_TAGGED_MIDS_MID);
    mid = tmid;
    Got_msg_from.insert(uid_out);
    Got_msg_from_this_session.insert(uid_out);
    // if they were removed, but we got a message from them, re-appear them
    Session_remove.remove(uid_out);
    add_unviewed(uid_out, tmid);
    DWYCO_SAVED_MSG_LIST sm;
    if(!dwyco_get_saved_message(&sm, uid_out.constData(), uid_out.length(), tmid.constData()))
    {
        // something is really hosed, saved it, and now
        // can't read it.
        Dont_refetch.insert(tmid);
    }
    simple_scoped qsm(sm);
    DWYCO_LIST bt = dwyco_get_body_text(qsm);
    simple_scoped qbt(bt);
    if(!dwyco_get_attr(qbt, 0, DWYCO_NO_COLUMN, txt))
    {
        Dont_refetch.insert(tmid);
        txt = "error";
    }
    dwyco_save_message(tmid.constData());
    return 1;
}
