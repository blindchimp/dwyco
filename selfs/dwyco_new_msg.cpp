
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
#include "pfx.h"
#include "dwycolistscoped.h"

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

void
load_unviewed()
{
    QSet<QByteArray> dum;
    load_inbox_tags_to_unviewed(dum);
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

void
del_unviewed_mid(const QByteArray& mid)
{
    dwyco_unset_msg_tag(mid.constData(), "unviewed");
}

bool
uid_has_unviewed_msgs(const QByteArray &uid)
{
    return uid_has_unfetched(uid) > 0 || dwyco_uid_has_tag(uid.constData(), uid.length(), "unviewed")
            || dwyco_uid_has_tag(uid.constData(), uid.length(), "_inbox");
}

int
uid_unviewed_msgs_count(const QByteArray &uid)
{
    return uid_has_unfetched(uid) + dwyco_uid_count_tag(uid.constData(), uid.length(), "unviewed") +
            dwyco_uid_count_tag(uid.constData(), uid.length(), "_inbox");
}

int
total_unviewed_msgs_count()
{
    return dwyco_count_tag("unviewed");
}

void
clear_unviewed_msgs()
{
    dwyco_unset_all_msg_tag("unviewed");
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
        if(Already_processed.contains(mid))
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
        Got_msg_from_this_session.insert(uid_out);
        Already_processed.insert(mid);
        add_unviewed(uid_out, mid);
        uids.insert(uid_out);
    }

    return 0;
}
