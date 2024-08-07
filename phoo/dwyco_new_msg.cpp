
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include <QSet>
#include <QFile>
#include <QDataStream>
#include "dwyco_new_msg.h"
#include "dlli.h"
#include "dwycolist2.h"

[[noreturn]] void cdcxpanic(const char *);
extern QByteArray Clbot;
static QSet<QByteArray> Got_msg_from_this_session;
static QSet<QByteArray> Already_processed;

static
int
uid_has_unfetched(const QByteArray& uid)
{
    DWYCO_UNFETCHED_MSG_LIST uml;
    if(!dwyco_get_unfetched_messages(&uml, uid.constData(), uid.length()))
        return 0;
    simple_scoped quml(uml);
    return quml.rows();
}

void
add_unviewed(const QByteArray& uid, const QByteArray& mid)
{
    dwyco_set_msg_tag(mid.constData(), "unviewed");
    Got_msg_from_this_session.insert(uid);
}

void
add_got_msg_from(const QByteArray& uid)
{
    Got_msg_from_this_session.insert(uid);
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
        if(dwyco_mid_has_tag(mid.constData(), "_trash"))
            continue;
        add_unviewed(uid, mid);
        uids_out.insert(uid);
    }
    dwyco_unset_all_msg_tag("_inbox");
}

void
load_unviewed()
{
    dwyco_start_bulk_update();
    QSet<QByteArray> dum;
    load_inbox_tags_to_unviewed(dum);
    DWYCO_LIST tm;
    if(!dwyco_get_tagged_mids(&tm, "unviewed"))
    {
        dwyco_end_bulk_update();
        return;
    }
    simple_scoped qtm(tm);
    for(int i = 0; i < qtm.rows(); ++i)
    {
        QByteArray mid = qtm.get<QByteArray>(i, DWYCO_TAGGED_MIDS_MID);
        if(dwyco_mid_has_tag(mid.constData(), "_trash"))
            dwyco_unset_msg_tag(mid.constData(), "unviewed");
    }
    dwyco_end_bulk_update();
}

bool
got_msg_this_session(const QByteArray &uid)
{
    return Got_msg_from_this_session.contains(uid);
}

void
del_unviewed_uid(const QByteArray& uid)
{
    DWYCO_LIST tm;
    if(!dwyco_get_tagged_mids(&tm, "unviewed"))
        return;
    simple_scoped qtm(tm);
    for(int i = 0; i < qtm.rows(); ++i)
    {
        QByteArray ruid = QByteArray::fromHex(qtm.get<QByteArray>(i, DWYCO_TAGGED_MIDS_HEX_UID));
        QByteArray mid = qtm.get<QByteArray>(i, DWYCO_TAGGED_MIDS_MID);
        if(ruid == uid)
            dwyco_unset_msg_tag(mid.constData(), "unviewed");
    }
}

// WARNING: this returns uids in HEX, not binary
QList<QByteArray>
uids_with_unviewed()
{
    DWYCO_LIST tm;
    if(!dwyco_get_tagged_mids(&tm, "unviewed"))
        return QList<QByteArray>();
    simple_scoped qtm(tm);
    QList<QByteArray> ret;
    for(int i = 0; i < qtm.rows(); ++i)
    {
        QByteArray ruid = qtm.get<QByteArray>(i, DWYCO_TAGGED_MIDS_HEX_UID);
        if(!ret.contains(ruid))
            ret.append(ruid);
    }
    DWYCO_UNFETCHED_MSG_LIST uml;
    if(!dwyco_get_unfetched_messages(&uml, 0, 0))
        return ret;
    simple_scoped quml(uml);
    for(int i = 0; i < quml.rows(); ++i)
    {
        QByteArray huid = quml.get<QByteArray>(i, DWYCO_QMS_FROM).toHex();
        if(!ret.contains(huid))
            ret.append(huid);
    }
    return ret;
}

void
del_unviewed_mid(const QByteArray& mid)
{
    dwyco_unset_msg_tag(mid.constData(), "unviewed");
}

bool
uid_has_unviewed_msgs(const QByteArray &uid)
{
    return uid_has_unfetched(uid) > 0 ||
            dwyco_uid_has_tag(uid.constData(), uid.length(), "unviewed")
            ;
#if 0
    ||
            dwyco_uid_has_tag(uid.constData(), uid.length(), "_inbox");
#endif
}

#if 0
int
uid_unviewed_msgs_count(const QByteArray &uid)
{
    return uid_has_unfetched(uid) + dwyco_uid_count_tag(uid.constData(), uid.length(), "unviewed") +
            dwyco_uid_count_tag(uid.constData(), uid.length(), "_inbox");
}
#endif

bool
any_unviewed_msgs()
{
    return dwyco_valid_tag_exists("unviewed");
}

void
clear_unviewed_msgs()
{
    dwyco_unset_all_msg_tag("unviewed");
}


int
dwyco_process_unfetched_list(DWYCO_UNFETCHED_MSG_LIST ml, QSet<QByteArray>& uids)
{
    int n;
    dwyco_list qml(ml);
    n = qml.rows();
    if(n == 0)
    {
        return 0;
    }
    QByteArray uid_out;
    QByteArray mid;
    for(int i = 0; i < n; ++i)
    {
        uid_out = qml.get<QByteArray>(i, DWYCO_QMS_FROM);
        if(uid_out == Clbot)
            continue;
        mid = qml.get<QByteArray>(i, DWYCO_QMS_ID);
        if(Already_processed.contains(mid))
            continue;
        if(!qml.is_nil(i, DWYCO_QMS_IS_DIRECT))
        {
            cdcxpanic("direct msgs api is different now");
        }

        // ok, at least it is likely the person will see it now
        // if there are any errors above, people may not be able to see a message
        // but the user would appear towards the top of the user list, which is weird.
        // this happens sometimes when attachments are not fetchable for whatever reason.
        Already_processed.insert(mid);
        if(dwyco_mid_has_tag(mid.constData(), "_trash"))
            continue;
        if(dwyco_mid_disposition(mid.constData()) == 0)
        {
            // this corresponds to the case where the index doesn't have any
            // record of this mid anywhere else. so we tag it in a way that will
            // show it to the user as a "new message"

            add_unviewed(uid_out, mid);
            uids.insert(uid_out);
        }
    }

    return 0;
}

