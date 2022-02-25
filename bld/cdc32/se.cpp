
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
#include "qauth.h"
#include "qmsg.h"
#include "qmsgsql.h"
#include "pgdll.h"

using namespace dwyco;

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
    DWYCO_SE_GRP_JOIN_OK,
    DWYCO_SE_GRP_JOIN_FAIL,

    DWYCO_SE_MSG_DOWNLOAD_PROGRESS,

    DWYCO_SE_MSG_PULL_OK,
    DWYCO_SE_MSG_TAG_CHANGE,
    DWYCO_SE_GRP_STATUS_CHANGE,

    DWYCO_SE_IGNORE_LIST_CHANGE,
    DWYCO_SE_IDENT_TO_UID,

    DWYCO_SE_SERVER_ATTR,
};

void
se_emit(dwyco_sys_event cmd, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = cmd;
    v[1] = map_to_representative_uid(uid);

    // this is a little dicey since we haven't really
    // thought about the exact issues related to having
    // multiple undelivered dups (like what if processing
    // a message multiple times in a row is what we want)
    // there is also issues with duplicate sequences of different
    // messages, especially if processing some message induces
    // more messages in here.
    // for now, we just check for one duplicate message on the
    // end of the q, and ignore it. this stops situations where
    // a bulk operation is stacking a bunch of the same
    // notification up, most of which aren't useful.
    // but really, this needs to be addressed on a
    // message-by-message basis.

    int n = Se_q.num_elems() - 1;
    if(n >= 0)
    {
        if((int)Se_q[n][0] == cmd && Se_q[n][1] == uid)
            return;
    }

    Se_q.append(v);
    GRTLOG("se_emit ", 0, 0);
    GRTLOGVC(v);
}

void
se_emit_chat(dwyco_sys_event cmd, vc server_id)
{
    vc v(VC_VECTOR);
    v[0] = cmd;
    v[1] = server_id;


    Se_q.append(v);
    GRTLOG("se_emit_chat ", 0, 0);
    GRTLOGVC(v);
}

void
se_emit_msg(dwyco_sys_event cmd, const DwString& qid, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = cmd;
    v[1] = map_to_representative_uid(uid);
    v[2] = vc(VC_BSTRING, qid.c_str(), qid.length());
    Se_q.append(v);
    GRTLOG("se_emit_msg ", 0, 0);
    GRTLOGVC(v);
}

void
se_emit_msg(dwyco_sys_event cmd, vc qid, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = cmd;
    v[1] = uid;
    v[2] = qid;
    Se_q.append(v);
    GRTLOG("se_emit_msg ", 0, 0);
    GRTLOGVC(v);
}

void
se_emit_msg_status(const DwString& qid, vc ruid, const DwString& msg, int percent)
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
se_emit_msg_progress(const DwString& mid, vc ruid, const DwString& msg, int percent)
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

void
se_emit_msg_pull_ok(vc mid, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = SE_MSG_PULL_OK;
    v[1] = map_to_representative_uid(uid);
    v[2] = mid;
    Se_q.append(v);
    GRTLOG("se_emit_msg_pull %s %s", (const char *)mid, (const char *)to_hex(uid));
    GRTLOGVC(v);
}

void
se_emit_msg_tag_change(vc mid, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = SE_MSG_TAG_CHANGE;
    v[1] = map_to_representative_uid(uid);
    v[2] = mid;
    Se_q.append(v);
    GRTLOG("se_emit_msg_tag_change %s", (const char *)mid, 0);
    GRTLOGVC(v);
}

void
se_emit_join(vc gname, int res)
{
    vc v(VC_VECTOR);

    v[0] = res ? SE_GRP_JOIN_OK : SE_GRP_JOIN_FAIL;
    v[1] = gname;
    Se_q.append(v);
    GRTLOGVC(v);
}

void
se_emit_group_status_change()
{
    vc v(VC_VECTOR);
    v[0] = SE_GRP_STATUS_CHANGE;
    Se_q.append(v);
    GRTLOGVC(v);
}

extern vc Cur_ignore;
extern vc Pals;

void
se_emit_uid_list_changed()
{
    Cur_ignore = get_local_ignore();
    Pals = get_local_pals();
    vc v(VC_VECTOR);
    v[0] = SE_IGNORE_LIST_CHANGE;
    Se_q.append(v);
    GRTLOGVC(v);
}

void
se_emit_server_attr(vc name, vc val)
{
    vc v(VC_VECTOR);
    v[0] = SE_SERVER_ATTR;
    v[1] = name;
    v[2] = val;
    Se_q.append(v);
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

        case SE_IDENT_TO_UID:
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

        case SE_MSG_PULL_OK:
        case SE_MSG_TAG_CHANGE:
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           Se_q[i][1], Se_q[i][1].len(),
                                           Se_q[i][2], Se_q[i][2].len(),
                                           0, 0, 0,
                                           0, 0
                                          );
            break;

        case SE_GRP_JOIN_FAIL:
        case SE_GRP_JOIN_OK:
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           Se_q[i][1], Se_q[i][1].len(),
                                           Se_q[i][1], Se_q[i][1].len(),
                                           0, 0, 0,
                                           0, 0
                                          );
            break;

        case SE_GRP_STATUS_CHANGE:
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           0, 0,
                                           0, 0,
                                           0, 0, 0,
                                           0, 0
                                          );
            break;

        case SE_IGNORE_LIST_CHANGE:
            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           0, 0,
                                           0, 0,
                                           0, 0, 0,
                                           0, 0
                                          );
            break;

        case SE_SERVER_ATTR:
        {
            const char *val;
            int len_val;
            int tp = dllify(Se_q[i][2], val, len_val);

            (*dwyco_system_event_callback)(api_cmd,
                                           0,
                                           0, 0,
                                           Se_q[i][1], Se_q[i][1].len(),
                                           tp, val, len_val,
                                           0, 0
                                          );
        }
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
