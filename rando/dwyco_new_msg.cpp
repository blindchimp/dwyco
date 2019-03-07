
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
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include "dwyco_new_msg.h"
#include "dlli.h"
#include "pfx.h"
#include "dwycolistscoped.h"
#include "msglistmodel.h"

static QSet<QByteArray> Got_msg_from;
static QSet<QByteArray> Got_msg_from_this_session;
typedef QHash<QByteArray, QByteArray> UID_MID_MAP;
static UID_MID_MAP Unviewed_msgs;
extern QMap<QByteArray,QByteArray> Hash_to_loc;

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

static void
add_unviewed(const QByteArray& uid, const QByteArray& mid)
{
    QList<QByteArray> ql = Unviewed_msgs.values(uid);
    if(ql.contains(mid))
        return;
    Unviewed_msgs.insertMulti(uid, mid);
    // note: ideally we would sort it in by the time the msg was sent, tho this
    // method usually works (messages can appear from different
    // sources like server vs. direct at different times.)

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

int
dwyco_process_unsaved_list(DWYCO_UNSAVED_MSG_LIST ml, QSet<QByteArray>& uids)
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
//        if(Dont_refetch.contains(mid) ||
//            fetching.contains(mid) || fetching.count() > 2)
//            continue;
        if(!dwyco_list_get(ml, i, DWYCO_QMS_IS_DIRECT, &val, &len, &type))
            continue;
        int special_type;

        if(dwyco_is_special_message(0, 0, mid.constData(), &special_type))
            continue;

#if 0
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
        if(type != DWYCO_TYPE_NIL)
        {
            if(dwyco_save_message(mid.constData()))
            {
                DWYCO_SAVED_MSG_LIST sml;
                if(dwyco_get_saved_message(&sml, uid_out.constData(), uid_out.length(), mid.constData()))
                {
                    simple_scoped qsml(sml);
                    QByteArray txt = qsml.get<QByteArray>(DWYCO_QM_BODY_NEW_TEXT2);
                    QJsonDocument qjd = QJsonDocument::fromJson(txt);
                    if(!qjd.isNull())
                    {
                        QJsonObject qjo = qjd.object();
                        if(!qjo.isEmpty())
                        {
                            QJsonValue h = qjo.value("hash");
                            QJsonValue loc = qjo.value("loc");
                            Hash_to_loc.insert(QByteArray::fromHex(h.toString().toLatin1()), loc.toString().toLatin1());
                            mlm->invalidate_sent_to();
                        }
                    }
                }
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

