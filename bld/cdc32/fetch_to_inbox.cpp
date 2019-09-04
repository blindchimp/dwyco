
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// call this function on "rescan msgs" and it will
// return the next new message received. keep calling it until
// it returns 0 to get all the messages.
//

#include "fetch_to_inbox.h"
#include "dlli.h"
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
//#include "dwsetr.h"
#include "dwstr.h"
#include "dwvec.h"
#include "dwtree2.h"

namespace ns_dwyco_background_processing {

struct simple_scoped
{
    DWYCO_LIST value;
    simple_scoped(DWYCO_LIST v) {
        value = v;
    }
    ~simple_scoped() {
        dwyco_list_release(value);
    }
    operator DWYCO_LIST() {
        return value;
    }
};

static DwVec<DwString> fetching;
static DwTreeKaz<int, DwString> Dont_refetch(0);
static DwVec<DwString> Delete_msgs;
static DwTreeKaz<int, DwString> Already_returned(0);


static int
dwyco_get_attr(DWYCO_LIST l, int row, const char *col, DwString& str_out)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type != DWYCO_TYPE_STRING)
        return 0;
    str_out = DwString(val, len);
    return 1;
}

static int
dwyco_test_attr(DWYCO_LIST l, int row, const char *col)
{
    const char *val;
    int len;
    int type;
    if(!dwyco_list_get(l, row, col, &val, &len, &type))
        return 0;
    if(type == DWYCO_TYPE_NIL)
        return 0;

    return 1;
}

void
DWYCOCALLCONV
msg_callback(int id, int what, const char *mid, void *)
{
//printf("id %d what %d mid %s\n", id, what, mid);
    int i = fetching.index(mid);
    switch(what)
    {
    case DWYCO_MSG_DOWNLOAD_FETCHING_ATTACHMENT:
        // here is a case where it makes sense to provide the text of the message
        // immediately while the attachment is being downloaded in the background.
        return;

    case DWYCO_MSG_DOWNLOAD_RATHOLED:
    case DWYCO_MSG_DOWNLOAD_DECRYPT_FAILED:
        // if this msg is ratholed, it can never be fetched, so just delete it.
        Dont_refetch.add(mid, 0);
        if(i >= 0)
            fetching.del(i);
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
        Dont_refetch.add(mid, 0);
        if(i >= 0)
            fetching.del(i);
        return;
    case DWYCO_MSG_DOWNLOAD_OK:
    default:
        if(i >= 0)
            fetching.del(i);
    }
}

int
fetch_to_inbox(DwString& uid_out, DwString& mid_out)
{
    int k = Delete_msgs.num_elems();
    for(int i = 0; i < k; ++i)
    {
        dwyco_delete_unsaved_message(Delete_msgs[i].c_str());

    }
    Delete_msgs.set_size(0);

    DWYCO_UNFETCHED_MSG_LIST qml;
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
        //dwyco_list_release(ml);
        return 0;
    }
    for(int i = 0; i < n; ++i)
    {
        if(!dwyco_get_attr(ml, i, DWYCO_QMS_FROM, uid_out))
            continue;
        if(!dwyco_get_attr(ml, i, DWYCO_QMS_ID, mid_out))
            continue;
        if(Already_returned.contains(mid_out))
            continue;
        if(Dont_refetch.contains(mid_out) ||
                fetching.contains(mid_out) || fetching.num_elems() >= 5)
            continue;
        if(!dwyco_list_get(ml, i, DWYCO_QMS_IS_DIRECT, &val, &len, &type))
            continue;
        if(type == DWYCO_TYPE_NIL)
        {
            int special_type;
            const char *uid;
            int len_uid;
            const char *dlv_mid;
            if(dwyco_is_delivery_report(mid_out.c_str(), &uid, &len_uid, &dlv_mid, &special_type))
            {
                // process pal authorization stuff here
                if(special_type == DWYCO_SUMMARY_DELIVERED)
                {
                    // NOTE: uid, dlv_mid must be copied out before next
                    // dll call
                    // hmmm, need new api to get uid/mid_out of delivered msg
                    dwyco_delete_unsaved_message(mid_out.c_str());
                    continue;
                }

            }
            // issue a server fetch, client will have to
            // come back in to get it when the fetch is done
            fetching.append(mid_out);
            dwyco_fetch_server_message(mid_out.c_str(), msg_callback, 0, 0, 0);
            continue;
        }

#if 0
        if(dwyco_is_special_message(0, 0, mid_out.constData(), &special_type))
        {
            // process pal authorization stuff here
            switch(special_type)
            {
            case DWYCO_SUMMARY_DELIVERED:
                // hmmm, need new api to get uid/mid_out of delivered msg
                dwyco_delete_unsaved_message(mid_out.constData());
                break;

#if 0
            case DWYCO_PAL_AUTH_REQ:
                // note that most of the "special message" stuff only
                // works on unsaved messages. which is a pain.
                display_msg(uid_out, "Pal authorization request", 0, DwString(""));
                // yuck, this chat_uids stuff needs to be encapsulated
                {
                    int i = chat_uids.index(uid_out);
                    if(i == -1)
                        break;
                    chatform2 *c = chat_wins[i];
                    dwyco_get_unsaved_message(&c->pal_auth_req_msg, mid_out.constData());
                    // note: we can't delete it if it has an attachment...
                    // we can *save* it ok, but the unsaved msg's attachment
                    // will become unreadable, but since we don't need the
                    // attachment for pal auth purposes, we'll ignore this "bug"
                }
                goto save_it;
                break;
            case DWYCO_PAL_OK:
                dwyco_handle_pal_auth(0, 0, mid_out.constData(), 1);
                display_msg(uid_out, "Pal authorization accepted", 0, DwString(""));
                dwyco_delete_unsaved_message(mid_out.constData());
                break;
            case DWYCO_PAL_REJECT:
                dwyco_handle_pal_auth(0, 0, mid_out.constData(), 1);
                dwyco_pal_delete(uid_out.constData(), uid_out.length());
                display_msg(uid_out, "Pal authorization rejected", 0, DwString(""));
                dwyco_delete_unsaved_message(mid_out.constData());
                break;
#endif
            default:
                dwyco_delete_unsaved_message(mid_out.constData());
                break;// do nothing, ignore it.
            }
            continue;
        }
#endif
#if 0
// don't need to do any of this if we are just fetching to inbox
        DWYCO_SAVED_MSG_LIST sm;
        if(!dwyco_unsaved_message_to_body(&sm, mid_out.c_str()))
            continue;
        DWYCO_LIST bt = dwyco_get_body_text(sm);
        if(!dwyco_get_attr(bt, 0, DWYCO_NO_COLUMN, txt))
            continue;

        has_att = dwyco_test_attr(sm, 0, DWYCO_QM_BODY_ATTACHMENT);

        dwyco_list_release(bt);
        dwyco_list_release(sm);
#endif
        //dwyco_list_release(ml);
        Already_returned.add(mid_out, 0);
        return 1;
    }
    // dwyco_list_release(ml);
    return 0;
}
}
