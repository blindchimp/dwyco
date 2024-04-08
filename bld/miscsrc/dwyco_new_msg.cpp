/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/


#include "dwyco_new_msg.h"
#include "dlli.h"
#include <QSet>
#include <QByteArray>
#include <QList>
#include "dwycolist2.h"

static QList<QByteArray> fetching;
static QSet<QByteArray> Dont_refetch;
static QList<QByteArray> Delete_msgs;
static QSet<QByteArray> Already_returned;

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
        // if this msg is ratholed, it can never be fetched, so just delete it.
        Dont_refetch.insert(mid);
        if(i >= 0)
            fetching.removeAt(i);
        Delete_msgs.append(mid);
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
        if(i >= 0)
            fetching.removeAt(i);
        return;
    case DWYCO_MSG_DOWNLOAD_OK:
    default:
        if(i >= 0)
            fetching.removeAt(i);
    }
}

int
process_remote_msgs()
{
    for(int i = 0; i < Delete_msgs.count(); ++i)
    {
        dwyco_delete_unfetched_message(Delete_msgs[i]);
    }
    Delete_msgs.clear();

    if(fetching.count() >= 3)
        return 0;
    DWYCO_UNFETCHED_MSG_LIST ufml;
    if(!dwyco_get_unfetched_messages(&ufml, 0, 0))
        return 0;
    simple_scoped qufml(ufml);
    int n = qufml.rows();
    if(n == 0)
        return 0;
    for(int i = 0; i < n; ++i)
    {
        QByteArray mid = qufml.get<QByteArray>(i, DWYCO_QMS_ID);
        if(fetching.contains(mid) || Dont_refetch.contains(mid))
            continue;
        if(dwyco_fetch_server_message(mid.constData(), msg_callback, 0, 0, 0))
        {
            fetching.append(mid);
            if(fetching.count() >= 3)
                return 1;
        }
    }
    return 0;
}


int
dwyco_new_msg2(QByteArray& uid_out, QByteArray& txt, int& zap_viewer, QByteArray& mid, int& has_att, int& is_file, QByteArray& creator_uid)
{
    DWYCO_LIST inbox_mids;
    if(!dwyco_get_tagged_idx(&inbox_mids, "_inbox", 0))
        return 0;
    simple_scoped qim(inbox_mids);
    int n = qim.rows();
    if(n == 0)
        return 0;
    for(int i = 0; i < n; ++i)
    {
        mid = qim.get<QByteArray>(i, DWYCO_MSG_IDX_MID);
        if(Already_returned.contains(mid))
            continue;
        Already_returned.insert(mid);
        uid_out = qim.get<QByteArray>(i, DWYCO_MSG_IDX_ASSOC_UID);
        uid_out = QByteArray::fromHex(uid_out);
        has_att = !qim.is_nil(i, DWYCO_MSG_IDX_HAS_ATTACHMENT);
        is_file = !qim.is_nil(i, DWYCO_MSG_IDX_IS_FILE);
        DWYCO_SAVED_MSG_LIST sm;
        if(!dwyco_get_saved_message(&sm, uid_out.constData(), uid_out.length(), mid.constData()))
            continue;
        simple_scoped qsm(sm);
        DWYCO_LIST bt = dwyco_get_body_text(qsm);
        simple_scoped qbt(bt);
        txt = qbt.get<QByteArray>(0);

        DWYCO_LIST ba = dwyco_get_body_array(qsm);
        simple_scoped qba(ba);
        if(qba.rows() == 1)
            creator_uid = uid_out;
        else
        {
            int crow = qba.rows() - 1;
            creator_uid = qba.get<QByteArray>(crow, DWYCO_QM_BODY_FROM);
        }

        return 1;
    }
    return 0;
}

int
dwyco_new_msg(QByteArray& uid_out, QByteArray& txt, int& zap_viewer, QByteArray& mid, int& has_att)
{
    QByteArray dum1;
    int dum2;
    return dwyco_new_msg2(uid_out, txt, zap_viewer, mid, has_att, dum2, dum1);
}

void
processed_msg(QByteArray& mid)
{
    dwyco_unset_msg_tag(mid.constData(), "_inbox");
}
