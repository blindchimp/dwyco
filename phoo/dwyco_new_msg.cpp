
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
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include "dwycolistscoped.h"

static QList<QByteArray> fetching;
QString dwyco_info_to_display(QByteArray uid);
static QSet<QByteArray> Got_msg_from;
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
load_inbox_tags_to_unviewed()
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


static void
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
        //Dont_refetch.insert(mid);
        del_unviewed_mid(mid);

#ifdef NO_SAVED_MSGS
        dwyco_delete_unsaved_message(mid);
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

    // note: qt docs say ordering of keys and values is the same
    QList<QByteArray> k = Unviewed_msgs.keys();
    QList<QByteArray> v = Unviewed_msgs.values();
    int n = k.count();


    for(int i = 0; i < n; ++i)
    {

        {
            DWYCO_SAVED_MSG_LIST qsm;
            if(dwyco_get_saved_message(&qsm, k[i].constData(), k[i].length(), v[i].constData()))
            {
                simple_scoped sm(qsm);
                // don't really need this extra check unless we are reloading
                // some visible text
#if 0
                DWYCO_LIST qbt = dwyco_get_body_text(sm);
                simple_scoped bt(qbt);
                QByteArray txt;
                if(dwyco_get_attr(bt, 0, DWYCO_NO_COLUMN, txt))
                {
                    //display_msg(k[i], txt, v[i]);
                }
                else
                {
                    kill_list_uid.append(k[i]);
                    kill_list_mid.append(v[i]);

                }
#endif
            }
            else
            {
                kill_list_uid.append(k[i]);
                kill_list_mid.append(v[i]);
            }
        }
    }
    n = kill_list_uid.count();
    for(int i = 0; i < n; ++i)
    {
        del_unviewed_mid(kill_list_uid[i], kill_list_mid[i]);
    }
    return 1;
}

#if 0

int
dwyco_new_msg(QByteArray& uid_out, QByteArray& txt, QByteArray& mid)
{
    return 0;

    int k = Delete_msgs.count();
    for(int i = 0; i < k; ++i)
    {
        dwyco_delete_unsaved_message(Delete_msgs[i].constData());

    }
    Delete_msgs.clear();

    DWYCO_UNSAVED_MSG_LIST qml;
    if(!dwyco_get_unsaved_messages(&qml, 0, 0))
        return 0;
    simple_scoped ml(qml);
    int n;
    const char *val;
    int len;
    int type;
    dwyco_list_numelems(ml, &n, 0);
    if(n == 0)
    {
        return 0;
    }
    for(int i = 0; i < n; ++i)
    {
        if(!dwyco_get_attr(ml, i, DWYCO_QMS_FROM, uid_out))
            continue;
        if(!dwyco_get_attr(ml, i, DWYCO_QMS_ID, mid))
            continue;
        if(Dont_refetch.contains(mid) ||
                fetching.contains(mid) || fetching.count() > 2)
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
                    // NOTE: uid, dlv_mid must be copied out before next
                    // dll call
                    // hmmm, need new api to get uid/mid_out of delivered msg
                    dwyco_delete_unsaved_message(mid.constData());
                    continue;
                }

            }
            // issue a server fetch, client will have to
            // come back in to get it when the fetch is done
            fetching.append(mid);
            dwyco_fetch_server_message(mid.constData(), msg_callback, 0, 0, 0);
            continue;
        }
        int special_type;
#if 0
        if(dwyco_is_special_message(0, 0, mid.constData(), &special_type))
        {
            // process pal authorization stuff here
            switch(special_type)
            {
            case DWYCO_PAL_AUTH_REQ:
                // note that most of the "special message" stuff only
                // works on unsaved messages. which is a pain.
                display_msg(uid_out, "Pal authorization request", 0, QByteArray(""));
                // yuck, this chat_uids stuff needs to be encapsulated
                {
                    int i = chat_uids.index(uid_out);
                    if(i == -1)
                        break;
                    chatform2 *c = chat_wins[i];
                    dwyco_get_unsaved_message(&c->pal_auth_req_msg, mid.constData());
                    // note: we can't delete it if it has an attachment...
                    // we can *save* it ok, but the unsaved msg's attachment
                    // will become unreadable, but since we don't need the
                    // attachment for pal auth purposes, we'll ignore this "bug"
                }
                goto save_it;
                break;
            case DWYCO_PAL_OK:
                dwyco_handle_pal_auth(0, 0, mid.constData(), 1);
                display_msg(uid_out, "Pal authorization accepted", 0, QByteArray(""));
                dwyco_delete_unsaved_message(mid.constData());
                break;
            case DWYCO_PAL_REJECT:
                dwyco_handle_pal_auth(0, 0, mid.constData(), 1);
                dwyco_pal_delete(uid_out.constData(), uid_out.length());
                display_msg(uid_out, "Pal authorization rejected", 0, QByteArray(""));
                dwyco_delete_unsaved_message(mid.constData());
                break;
            default:
                dwyco_delete_unsaved_message(mid.constData());
                break;// do nothing, ignore it.
            }
            continue;
        }
#endif
save_it:
        ;
        DWYCO_SAVED_MSG_LIST qsm;
#ifndef NO_SAVED_MSGS
        int delete_unsaved = 0;
        if(!dwyco_save_message(mid.constData()))
        {
            // keep from constantly refetching it
            // this happens sometimes when attachments get lost
            // for direct messages, things are read-only, etc.
            Dont_refetch.insert(mid);
            // see if we can continue with the unsaved msg, just not
            // saving it.
            if(!dwyco_unsaved_message_to_body(&qsm, mid.constData()))
            {
                dwyco_delete_unsaved_message(mid.constData());
                continue;
            }
            else
                delete_unsaved = 1;
        }
        else
        {
            if(!dwyco_get_saved_message(&qsm, uid_out.constData(), uid_out.length(), mid.constData()))
            {
                // something is really hosed, saved it, and now
                // can't read it.
                Dont_refetch.insert(mid);
                continue;
            }
        }


#else
        if(!dwyco_unsaved_message_to_body(&qsm, mid.constData()))
            continue;
#endif
        simple_scoped sm(qsm);
        DWYCO_LIST qbt = dwyco_get_body_text(sm);
        simple_scoped bt(qbt);
        if(!dwyco_get_attr(bt, 0, DWYCO_NO_COLUMN, txt))
            continue;
#ifdef NO_SAVED_MSGS
        int dont_delete = 0;
#endif


        // need to just q it for delete, since we want to play
        // the video that came with it potentially

        if(delete_unsaved)
            dwyco_delete_unsaved_message(mid.constData());
#ifdef NO_SAVED_MSGS
        if(!dont_delete)
            dwyco_delete_unsaved_message(mid.constData());
#endif
        // ok, at least it is likely the person will see it now
        // if there are any errors above, people may not be able to see a message
        // but the user would appear towards the top of the user list, which is weird.
        // this happens sometimes when attachments are not fetchable for whatever reason.
        Got_msg_from.insert(uid_out);
        Got_msg_from_this_session.insert(uid_out);
        add_unviewed(uid_out, mid);
        return 1;
    }

    return 0;
}
#endif

int
dwyco_process_unsaved_list(DWYCO_UNFETCHED_MSG_LIST ml, QSet<QByteArray>& uids)
{
    int k = Delete_msgs.count();
    for(int i = 0; i < k; ++i)
    {
        dwyco_delete_unfetched_message(Delete_msgs[i].constData());
    }
    Delete_msgs.clear();

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
//        if(Dont_refetch.contains(mid) ||
//            fetching.contains(mid) || fetching.count() > 2)
//            continue;
        if(!dwyco_list_get(ml, i, DWYCO_QMS_IS_DIRECT, &val, &len, &type))
            continue;

        if(type != DWYCO_TYPE_NIL)
        {
            ::abort();
            if(!dwyco_save_message(mid.constData()))
            {
                // keep from constantly refetching it
                // this happens sometimes when attachments get lost
                // for direct messages, things are read-only, etc.
                Dont_refetch.insert(mid);
            }
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

