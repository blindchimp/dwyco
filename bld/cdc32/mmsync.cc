
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/mmsync.cc 1.11 1999/01/10 16:09:49 dwight Checkpoint $

#include "mmchan.h"

void
MMChannel::sync_send()
{
    vc updates;

    updates = sync_manager.diff();
    if(updates.num_elems() == 0)
        return;
    vc msg(VC_VECTOR);
    msg.append("syncvar");
    msg.append(updates);
    send_ctrl(msg);
}

void
MMChannel::sync_recv(const vc& updates)
{
    int nupdates = updates.num_elems();

    GRTLOG("chansync %d", nupdates, 0);
    GRTLOGVC(updates);

    for(int i = 0; i < nupdates; ++i)
    {
        const vc u = updates[i];
        int action = ((const char *)u[0])[0];
        const vc k = u[1];
        const vc v = u[2];
        switch(action)
        {
        case 'a':
            remote_vars.map->add(k, v);
            break;
        case 'r':
            remote_vars.map->replace(k, v);
            break;
        case 'd':
            remote_vars.map->del(k);
            break;
        }
    }
}
