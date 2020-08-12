
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "se.h"
#include "dlli.h"
#include "dwvec.h"
#include "dwstr.h"
#include "dwrtlog.h"
#include "vc.h"

extern DwycoSystemEventCallback dwyco_system_event_callback;

static DwVec<vc> Se_q;
// this table is a quick hack, it is indexed by the enum in se.h
// if you change that, you need to change this accordingly.
// need to get rid of this
static int Se_cmd_to_api[] =
{
    -1,
    DWYCO_SE_USER_STATUS_CHANGE,
    DWYCO_SE_USER_ADD,
    DWYCO_SE_USER_DEL,
//DWYCO_SE_SERVER_CONNECTING,
//DWYCO_SE_SERVER_CONNECTION_SUCCESSFUL,
//DWYCO_SE_SERVER_DISCONNECT,
//DWYCO_SE_SERVER_LOGIN,
//DWYCO_SE_SERVER_LOGIN_FAILED,
    DWYCO_SE_USER_MSG_RECEIVED,
    DWYCO_SE_USER_UID_RESOLVED,
    DWYCO_SE_USER_PROFILE_INVALIDATE,
    DWYCO_SE_USER_MSG_IDX_UPDATED,
    DWYCO_SE_MSG_SEND_START,
    DWYCO_SE_MSG_SEND_FAIL,
    DWYCO_SE_MSG_SEND_SUCCESS,
    DWYCO_SE_MSG_SEND_CANCELED,
    DWYCO_SE_MSG_SEND_DELIVERY_SUCCESS,
    DWYCO_SE_MSG_SEND_STATUS,

    DWYCO_SE_MSG_DOWNLOAD_START,
    DWYCO_SE_MSG_DOWNLOAD_FAILED,
    DWYCO_SE_MSG_DOWNLOAD_FETCHING_ATTACHMENT,
    DWYCO_SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED,
    DWYCO_SE_MSG_DOWNLOAD_OK,
    DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED,
    DWYCO_SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED_DECRYPT_FAILED,

    DWYCO_SE_USER_MSG_IDX_UPDATED_PREPEND,

    DWYCO_SE_CHAT_SERVER_CONNECTING,
    DWYCO_SE_CHAT_SERVER_CONNECTION_SUCCESSFUL,
    DWYCO_SE_CHAT_SERVER_DISCONNECT,
    DWYCO_SE_CHAT_SERVER_LOGIN,
    DWYCO_SE_CHAT_SERVER_LOGIN_FAILED,

    DWYCO_SE_MSG_DOWNLOAD_PROGRESS,
};

void
se_emit(dwyco_sys_event cmd, vc id)
{
    vc v(VC_VECTOR);
    v[0] = cmd;
    v[1] = id;

    // don't filter out dups for now.
    // there may be ordering dependencies that are
    // more important than just processing a dup command
    //
#if 0
    int n = Se_q.num_elems();
    for(int i = 0; i < n; ++i)
    {
        if((int)Se_q[i][0] == cmd && Se_q[i][1] == id)
            return;
    }
#endif
    Se_q.append(v);
    GRTLOG("se_emit ", 0, 0);
    GRTLOGVC(v);
}

void
se_emit_msg(dwyco_sys_event cmd, DwString qid, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = cmd;
    v[1] = uid;
    v[2] = qid.c_str();
    Se_q.append(v);
    GRTLOG("se_emit_msg ", 0, 0);
    GRTLOGVC(v);
}

void
se_emit_msg(dwyco_sys_event cmd, vc qid, vc uid)
{
    se_emit_msg(cmd, DwString((const char *)qid, 0, qid.len()), uid);
}

void
se_emit_msg_status(DwString qid, vc ruid, DwString msg, int percent)
{
    vc v(VC_VECTOR);
    v[0] = SE_MSG_SEND_STATUS;
    v[1] = ruid;
    v[2] = qid.c_str();
    v[3] = msg.c_str();
    v[4] = percent;
    Se_q.append(v);
    GRTLOG("se_emit_msg_status ", 0, 0);
    GRTLOGVC(v);

}

void
se_emit_msg_progress(DwString mid, vc ruid, DwString msg, int percent)
{
    vc v(VC_VECTOR);
    v[0] = SE_MSG_DOWNLOAD_PROGRESS;
    v[1] = ruid;
    v[2] = mid.c_str();
    v[3] = msg.c_str();
    v[4] = percent;
    Se_q.append(v);
    GRTLOG("se_emit_msg_progress ", 0, 0);
    GRTLOGVC(v);

}

int
se_process()
{
    if(!dwyco_system_event_callback)
    {
        if(Se_q.num_elems() > 0)
        {
            GRTLOG("no se handler, dropped %d", Se_q.num_elems(), 0);
            Se_q.set_size(0);
        }
        return 0;
    }
    int n = Se_q.num_elems();
    for(int i = 0; i < n; ++i)
    {
        GRTLOG("se deliver", 0, 0);
        GRTLOGVC(Se_q[i]);
        int cmd = (int)Se_q[i][0];
        int api_cmd = Se_cmd_to_api[cmd];
        switch(cmd)
        {
        case SE_STATUS_CHANGE:
        case SE_USER_ADD:
        case SE_USER_REMOVE:
        case SE_USER_MSG_RECEIVED:
        case SE_USER_UID_RESOLVED:
        case SE_USER_PROFILE_INVALIDATE:
        case SE_USER_MSG_IDX_UPDATED:
        case SE_USER_MSG_IDX_UPDATED_PREPEND:
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           Se_q[i][1], Se_q[i][1].len(),
                                           0, 0,
                                           0, 0, 0,
                                           0, 0
                                          );
            break;

        case     SE_CHAT_SERVER_CONNECTING:
        case     SE_CHAT_SERVER_CONNECTION_SUCCESSFUL:
        case     SE_CHAT_SERVER_DISCONNECT:
        case     SE_CHAT_SERVER_LOGIN:
        case     SE_CHAT_SERVER_LOGIN_FAILED:
        {
            vc v;
            if(Se_q[i][1].type() == VC_INT)
            {
                v = Se_q[i][1].peek_str();
            }
            else
                v = Se_q[i][1];
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           v, v.len(),
                                           0, 0,
                                           0, 0, 0,
                                           0, 0
                                          );
        }
            break;

        case SE_MSG_SEND_START:
        case SE_MSG_SEND_FAIL:
        case SE_MSG_SEND_SUCCESS:
        case SE_MSG_SEND_CANCELED:
        case SE_MSG_SEND_DELIVERY_SUCCESS:
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           Se_q[i][1], Se_q[i][1].len(),
                                           0, 0,
                                           DWYCO_TYPE_STRING, (const char *)Se_q[i][2], Se_q[i][2].len(),
                                           0, 0
                                          );
            break;

        case SE_MSG_SEND_STATUS:
        case SE_MSG_DOWNLOAD_PROGRESS:
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           Se_q[i][1], Se_q[i][1].len(),
                                           Se_q[i][3], Se_q[i][3].len(),
                                           DWYCO_TYPE_STRING, (const char *)Se_q[i][2], Se_q[i][2].len(),
                                           0, Se_q[i][4]
                                          );
            break;

        case SE_MSG_DOWNLOAD_START:
        case SE_MSG_DOWNLOAD_FAILED:
        case SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_START:
        case SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED:
        case SE_MSG_DOWNLOAD_OK:
        case SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED:
        case SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED_DECRYPT_FAILED:
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           Se_q[i][1], Se_q[i][1].len(),
                                           0, 0,
                                           DWYCO_TYPE_STRING, (const char *)Se_q[i][2], Se_q[i][2].len(),
                                           0, 0
                                          );
            break;

        default:
            oopanic("bad se cmd");
        }
    }
    Se_q.set_size(0);
    if(n > 0)
        return 1;
    return 0;
}
