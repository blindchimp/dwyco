
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SE_H
#define SE_H
// note: in order to keep our promise that callbacks
// are not invoked except via service channels, we q up the
// emitted commands and then send them out on the next
// service_channels calls. note that i'm *pretty sure*
// in most cases we promise callbacks aren't called directly
// when you issue a command to the dll (there might be one
// case where this isn't true, the status callback for zap_send2.)
// note that we do this to avoid
// cases where in a UI in some button event, you call a command
// that immediately changes the status of a UID, which results
// in another callback that wants to change the same object...
#include "vc.h"
#include "dwstr.h"

// at this point, the id can be a uid or a mid
void se_emit(int cmd, vc id);
void se_emit_msg(int cmd, DwString qid, vc uid);
void se_emit_msg(int cmd, vc qid, vc uid);
void se_emit_msg_status(DwString qid, vc ruid, DwString msg, int percent);
int se_process();
enum dwyco_sys_event {
    SE_STATUS_CHANGE = 1,
    SE_USER_ADD,
    SE_USER_REMOVE,
    SE_USER_MSG_RECEIVED,
    SE_USER_UID_RESOLVED,
    SE_USER_PROFILE_INVALIDATE,
    SE_USER_MSG_IDX_UPDATED,

// message send events
// NOTE: really need to distinguish between "transient fail" and
// "permanent fail", perma fail being because msg is corrupted or something
// so it can never be delivered.
    SE_MSG_SEND_START,
    SE_MSG_SEND_FAIL,
    SE_MSG_SEND_SUCCESS,
    SE_MSG_SEND_CANCELED,

// for delivery success, the persistent id is the mid of the message
// as it is stored on the sender, rather than the persistent id
// used during the q processing.
    SE_MSG_SEND_DELIVERY_SUCCESS,

    SE_MSG_SEND_STATUS,

    SE_MSG_DOWNLOAD_START,
    SE_MSG_DOWNLOAD_FAILED,
    SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_START,
    SE_MSG_DOWNLOAD_ATTACHMENT_FETCH_FAILED,
    SE_MSG_DOWNLOAD_OK,
    SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED,
    SE_MSG_DOWNLOAD_FAILED_PERMANENT_DELETED_DECRYPT_FAILED,

// special, but common case, where msg is just prepended to
// top of msg index, no need to completely reset models in this case
    SE_USER_MSG_IDX_UPDATED_PREPEND,

    DWYCO_SE_CHAT_SERVER_CONNECTING,
    DWYCO_SE_CHAT_SERVER_CONNECTION_SUCCESSFUL,
    DWYCO_SE_CHAT_SERVER_DISCONNECT,
    DWYCO_SE_CHAT_SERVER_LOGIN,
    DWYCO_SE_CHAT_SERVER_LOGIN_FAILED
};

#endif
