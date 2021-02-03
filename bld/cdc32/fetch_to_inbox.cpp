
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
#include "dwstr.h"
#include "dwvec.h"
#include "dwtree2.h"
#include "dwycolist2.h"

namespace ns_dwyco_background_processing {

static DwVec<DwString> fetching;
static DwTreeKaz<int, DwString> Dont_refetch(0);
static DwVec<DwString> Delete_msgs;
static DwTreeKaz<int, DwString> Already_returned(0);

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
        if(dwyco_is_special_message(mid, &what))
            dwyco_set_msg_tag(mid, "_special");
        // FALLTHRU
    default:
        if(i >= 0)
            fetching.del(i);
    }
}

int
fetch_to_inbox()
{
    int k = Delete_msgs.num_elems();
    for(int i = 0; i < k; ++i)
    {
        dwyco_delete_unfetched_message(Delete_msgs[i].c_str());
    }
    Delete_msgs.set_size(0);

    DWYCO_UNFETCHED_MSG_LIST qml;
    if(!dwyco_get_unfetched_messages(&qml, 0, 0))
        return 0;
    simple_scoped ml(qml);
    int n = ml.rows();
    if(n == 0)
    {
        return 0;
    }
    DwString mid_out;
    for(int i = 0; i < n; ++i)
    {
        //if(!dwyco_get_attr(ml, i, DWYCO_QMS_FROM, uid_out))
        //    continue;
        //if(!dwyco_get_attr(ml, i, DWYCO_QMS_ID, mid_out))
        //    continue;
        mid_out = ml.get<DwString>(i, DWYCO_QMS_ID);
        if(mid_out.length() == 0)
            continue;

        if(Already_returned.contains(mid_out))
            continue;
        if(Dont_refetch.contains(mid_out) ||
                fetching.contains(mid_out))
            continue;

        // issue a server fetch, client will have to
        // come back in to get it when the fetch is done
        fetching.append(mid_out);
        dwyco_fetch_server_message(mid_out.c_str(), msg_callback, 0, 0, 0);

        Already_returned.add(mid_out, 0);
        if(fetching.num_elems() >= 3)
            break;
        return 1;
    }
    return 0;
}
}
