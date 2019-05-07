
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
extern QMap<QByteArray,QByteArray> Hash_to_review;

static int
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

        if(!dwyco_list_get(ml, i, DWYCO_QMS_IS_DIRECT, &val, &len, &type))
            continue;
        int special_type;

        if(dwyco_is_special_message(0, 0, mid.constData(), &special_type))
            continue;

        int do_add_unviewed = 1;
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
                            QJsonValue rev = qjo.value("review");

                            if(!loc.isUndefined())
                                Hash_to_loc.insert(QByteArray::fromHex(h.toString().toLatin1()), loc.toString().toLatin1());
                            if(!rev.isUndefined())
                                Hash_to_review.insert(QByteArray::fromHex(h.toString().toLatin1()), rev.toString().toLatin1());

                            mlm->invalidate_sent_to();
                            // these little json control messages don't get seen by the user directly
                            do_add_unviewed = 0;
                            // note: this is a hack. we favorite the message so it doesn't
                            // get removed during "clear-non favorites". this keeps us
                            // from having to check hashes while we are deleting pictures.
                            // down side is that some of the geo-info stays around across clears.
                            // we'll fix this eventually.
                            if(qsml.is_nil(DWYCO_QM_BODY_ATTACHMENT))
                            {
                                dwyco_set_fav_msg(mid.constData(), 1);
                                // note: this is a hack... we set the msg tag to "hidden",  but
                                // we actually mean "new". we then display the annotation differentlhy
                                // in the ui so the user knows it is new. then when they have seen it,
                                // we unset the tag. only reason we do this is because we already have
                                // the infrastructure for "hidden" setup, which we aren't using for that
                                // purpose in this app.
                                dwyco_set_msg_tag(mid.constData(), "_hid");

                            }
                        }
                    }
                }
            }
        }

        // ok, at least it is likely the person will see it now
        // if there are any errors above, people may not be able to see a message
        // but the user would appear towards the top of the user list, which is weird.
        // this happens sometimes when attachments are not fetchable for whatever reason.
        if(do_add_unviewed)
        {
            Got_msg_from.insert(uid_out);
            Got_msg_from_this_session.insert(uid_out);
            add_unviewed(uid_out, mid);
        }
        uids.insert(uid_out);
    }

    return 0;
}

