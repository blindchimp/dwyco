/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// call this function on "rescan msgs" and it will
// return the list of new msgs received
//

#include <stdio.h>
#include "dwyco_new_msg.h"
#include "dwycolistscoped.h"
#include "dlli.h"
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include <QSet>
#include <QByteArray>
#include <QList>

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
dwyco_new_msg(QByteArray& uid_out, QByteArray& txt, int& zap_viewer, QByteArray& mid, int& has_att, int& is_file)
{
    zap_viewer = 0;

    int k = Delete_msgs.count();
    for(int i = 0; i < k; ++i)
    {
        dwyco_delete_unsaved_message(Delete_msgs[i].constData());

    }
    Delete_msgs.clear();

    DWYCO_UNSAVED_MSG_LIST ml;
    if(!dwyco_get_unsaved_messages(&ml, 0, 0))
        return 0;
    simple_scoped qml(ml);
    int n;

    n = qml.rows();
    if(n == 0)
        return 0;
    for(int i = 0; i < n; ++i)
    {
        try {

            uid_out = qml.get<QByteArray>(i, DWYCO_QMS_FROM);
            mid = qml.get<QByteArray>(i, DWYCO_QMS_ID);

            if(Already_returned.contains(mid))
                continue;
            if(Dont_refetch.contains(mid) ||
                    fetching.contains(mid) || fetching.count() >= 5)
                continue;
            int direct_nil = qml.is_nil(i, DWYCO_QMS_IS_DIRECT);

            if(direct_nil)
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


            DWYCO_SAVED_MSG_LIST sm;
            if(!dwyco_unsaved_message_to_body(&sm, mid.constData()))
                continue;
            simple_scoped qsm(sm);
            DWYCO_LIST bt = dwyco_get_body_text(qsm);
            simple_scoped qbt(bt);
            txt = qbt.get<QByteArray>(DWYCO_NO_COLUMN);
            has_att = !qsm.is_nil(DWYCO_QM_BODY_ATTACHMENT);
            is_file = !qsm.is_nil(DWYCO_QM_BODY_FILE_ATTACHMENT);

            Already_returned.insert(mid);
            return 1;
        }
        catch(...)
        {
            continue;
        }
    }

    return 0;
}
