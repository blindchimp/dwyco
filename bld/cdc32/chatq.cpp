
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// send commands to the chat server related to audio q-ing
#include "vc.h"
#include "mmchan.h"
#include "chatq.h"
#include "dirth.h"
#include "ezset.h"

extern vc My_UID;

static int
send_to_chatserver(vc v)
{
    extern int Chat_id;
    extern int Chat_online;
    if(Chat_id == -1)
        return 0;
    // note: all of these commands have to be issued
    // after the initial login sequence
    if(!Chat_online)
        return 0;
    MMChannel *m = MMChannel::channel_by_id(Chat_id);
    if(!m)
        return 0;
    m->send_ctrl(v);
    return 1;
}

int
chatq_send_addq(int q, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = "addq";
    v[1] = q;
    v[2] = uid;
    v[3] = vcnil; // always bottom for now
    v[4] = 5;
    v[5] = 30;

    return send_to_chatserver(v);
}

int
chatq_send_delq(int q, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = "delq";
    v[1] = q;
    v[2] = uid;

    return send_to_chatserver(v);
}

int
chatq_send_data(int q, vc uid, const char *buf, int len)
{
    vc v(VC_VECTOR);
    v[0] = "audio";
    v[1] = q;
    v[2] = uid;
    v[3] = vc(VC_BSTRING, buf, len);

    return send_to_chatserver(v);
}

int
chatq_send_talk(int q, vc uid)
{
    vc v(VC_VECTOR);
    v[0] = "talk";
    v[1] = q;
    v[2] = uid;
    v[3] = vctrue;

    return send_to_chatserver(v);
}

int
chatq_send_mute(int q, vc uid, vc on)
{
    vc v(VC_VECTOR);
    v[0] = "mute";
    v[1] = q;
    v[2] = uid;
    v[3] = on;

    return send_to_chatserver(v);
}

int
chatq_send_pals_only(int q, vc uid, vc on)
{
    vc v(VC_VECTOR);
    v[0] = "pals-only";
    v[1] = q;
    v[2] = uid;
    v[3] = on;

    return send_to_chatserver(v);
}

// administrative things

int
chatq_send_set_chat_filter(int q, vc only_gods, vc only_demi_gods, vc only_uid)
{
    vc v(VC_VECTOR);
    v[0] = "set-chat-filter";
    v[1] = q;
    v[2] = only_gods;
    v[3] = only_demi_gods;
    v[4] = only_uid;

    return send_to_chatserver(v);
}

int
chatq_send_set_demigod(vc uid, vc on)
{
    vc v(VC_VECTOR);
    v[0] = "set-demigod";
    v[1] = uid;
    v[2] = on;

    return send_to_chatserver(v);
}

int
chatq_send_clear_all_demigods()
{
    vc v(VC_VECTOR);
    v[0] = "clear-all-demigods";

    return send_to_chatserver(v);
}

int
chatq_send_set_unblock_time(int q, vc uid, int tm, vc reason)
{
    vc v(VC_VECTOR);
    v[0] = "set-unblock-time";
    v[1] = q;
    v[2] = uid;
    v[3] = tm;
    v[4] = reason;

    return send_to_chatserver(v);
}

int
chatq_send_get_admin_info()
{
    vc v(VC_VECTOR);
    v[0] = "get-admin-info";
    return send_to_chatserver(v);
}


int
chatq_send_set_sys_attr(vc name, vc val)
{
    vc v(VC_VECTOR);
    v[0] = "set-sys-attr";
    v[1] = name;
    v[2] = val;
    return send_to_chatserver(v);
}


int
chatq_send_update_call_accept()
{
    vc v(VC_VECTOR);
    v[0] = "set-call-accept";
    vc de = build_directory_entry();
    v[1] = de[3];
    return send_to_chatserver(v);
}

int
chatq_send_update_pals(vc pals)
{
    vc v(VC_VECTOR);
    v[0] = "update-pals";
    v[1] = pals;
    return send_to_chatserver(v);
}

int
chatq_send_activity_state(vc state)
{
    vc v(VC_VECTOR);
    v[0] = "set-activity";
    v[1] = state;
    return send_to_chatserver(v);
}

int
chatq_send_popup(vc text, int global)
{
    vc v(VC_VECTOR);
    v[0] = global ? "global-popup" : "local-popup";
    v[1] = text;
    return send_to_chatserver(v);
}

// note: the msg is sent as an opaque buffer to the
// recipients. there is no attempt to structure it.
int
chatq_send_chat(vc chan, vc chat, int pic_type, vc pic_data)
{
    vc vv(VC_VECTOR);
    vv[0] = "chatc";
    vv[1] = chan;
    vv[2] = get_settings_value("user/username");
    if(vv[2].is_nil())
        vv[2] = "<<unknown>>";
    vc v(VC_VECTOR);
    v[0] = chat;
    v[1] = pic_type;
    v[2] = pic_data;
    vv[3] = v;
    vv[4] = My_UID;

    return send_to_chatserver(vv);
}

// note: the msg is sent as an opaque buffer to the
// recipients. there is no attempt to structure it.
int
chatq_send_selective_chat(vc chan, vc recipients, vc msg)
{
    vc vv(VC_VECTOR);
    vv[0] = "schatc";
    vv[1] = chan;
    vv[2] = get_settings_value("user/username");
    if(vv[2].is_nil())
        vv[2] = "<<unknown>>";
    vv[3] = msg;
    vv[4] = My_UID;
    vv[5] = recipients;

    return send_to_chatserver(vv);
}

