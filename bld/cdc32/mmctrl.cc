
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/mmctrl.cc 1.11 1999/01/10 16:09:45 dwight Checkpoint $
#include "mmchan.h"
#include "chatdisp.h"
#include "dwrtlog.h"
#include "qmsg.h"
#include "qauth.h"
#include "servass.h"
#include "fnmod.h"

using namespace dwyco;

void
MMChannel::react_to_droppage(int ndropped)
{
#if 0
    // should schedule a reference frame to
    // clean things up...
    // simple for now... just decrease the frame
    // rate.
    frame_interval *= 2;
    if(frame_interval > 2000)
        frame_interval = 2000;
    frame_timer.set_interval(frame_interval);
#endif
    // note: this is a percentage, not a
    // count...
    MMChannel *mc = find_xmitter();
    if(!mc)
        return;
    if(mc != this)
    {
        mc->react_to_droppage(ndropped); // forward to coder object
        return;
    }

    remote_percent_dropped = ndropped;
    if(remote_percent_dropped > 2)
    {
        ref_timer.set_interval(0);
        ref_timer.reset();
        ref_timer.start();
        auto_quality_boost = 1;
    }
    else
    {
        ref_timer.set_interval(10000);
        ref_timer.reset();
        ref_timer.start();
        auto_quality_boost = 0;
    }
}

void
MMChannel::display_chat(vc who, vc msg, vc from_uid)
{
    if(!chat_display)
        return;
    if(uid_ignored(from_uid))
        return;
    chat_display->output(who, who.len(), msg, msg.len(), from_uid, chatbox_id);
}

void
MMChannel::display_chat_private(vc msg)
{
    if(!private_chat_display)
        return;
    private_chat_display->output(msg, private_chatbox_id);
}

void
MMChannel::send_chat(vc msg)
{
    vc vv(VC_VECTOR);
    vv[0] = "chat";
    vv[1] = username();
    vv[2] = msg;
    vv[3] = My_UID;
    send_ctrl(vv);
    last_msg = vv;
    ++last_msg_index;
}

void
MMChannel::send_selective_chat(vc msg, int send_to_remote)
{
    vc vv(VC_VECTOR);
    vv[0] = "chat";
    DwString a("<p>");
    a += (const char *)username();
    vv[1] = a.c_str();
    vv[2] = msg;
    if(send_to_remote)
        send_ctrl(vv);
    last_msg = vv;
    ++last_msg_index;
}

void
MMChannel::send_chat_private(vc msg)
{
    vc vv(VC_VECTOR);
    vv[0] = "c";
    vv[1] = msg;
    send_ctrl(vv);
}

void
MMChannel::send_ctrl(vc v)
{
    GRTLOG("send ctrl on chan %d", myid, 0);
    GRTLOGVC(v);
    ctrl_q.append(v);
    // note: we had a kluge in here before that just
    // dropped the connection when the q got above a fixed
    // count, which was bad since once clients got enough users
    // in their user list, the q could grow quickly with all the
    // public key fetches that happen at program startup.
    // replaced it by a watchdog that actually is time based... if
    // we haven't sent anything in awhile and there are q items waiting,
    // we drop the channel, assuming it is broken.
}

void
MMChannel::ctrl_flush()
{
    GRTLOG("ctrl flush chan %d, items flushed: %d", myid, ctrl_q.num_elems());
    GRTLOGVC(ctrl_q);
    ctrl_q = vc(VC_VECTOR);
}

void
MMChannel::send_error(vc s)
{
    vc v(VC_VECTOR);
    v[0] = "nok";
    v[1] = s;
    send_ctrl(v);
}

void
MMChannel::react_to_delay_change(vc v)
{

}

void
MMChannel::send_decrypt()
{
    // after this message, everything we send will be encrypted with the agreed_key
    // so set that up now. note: we need to wait until we actually send
    // the "decrypt" command. this just q's it up.
    send_ctrl("decrypt");
}

void
MMChannel::regular_control_procedures(vc v)
{
    if(msg_chan || user_control_chan || is_sync_chan)
    {
        regular_control_procedures2(v);
        return;
    }

    if(established_callback_called || call_accepted_callback_called)
    {
        int n = deferred_delivery_q.num_elems();
        if(n == 0)
        {
            regular_control_procedures2(v);
            return;
        }
        for(int i = 0; i < n; ++i)
        {
            regular_control_procedures2(deferred_delivery_q[i]);
        }
        deferred_delivery_q.set_size(0);
        regular_control_procedures2(v);
        return;
    }

    switch(v.type())
    {
    case VC_VECTOR:
    {
        const char *what = v[0];
        if(strcmp(what, "chat") == 0)
        {
            deferred_delivery_q.append(v);
        }
        else if(strcmp(what, "c") == 0)
        {
            deferred_delivery_q.append(v);
        }
        else if(strcmp(what, "msg") == 0)
        {
            deferred_delivery_q.append(v);
        }
        else if(strcmp(what, "uc") == 0)
        {
            deferred_delivery_q.append(v);
        }
        return;
    }
    default:
        break;
        // note: no default case, just ignore
        // unknown messages
    }
    regular_control_procedures2(v);

}

void
MMChannel::regular_control_procedures2(vc v)
{
    GRTLOG("reg ctrl on chan %d", myid, 0);
    GRTLOGVC(v);
    switch(v.type())
    {
    case VC_VECTOR:
    {
        if(v[0].type() == VC_VECTOR)
            process_with_response(v);
        else
        {
            // this is older stuff
            const char *what = v[0];
            if(strcmp(what, "drop") == 0)
            {
                react_to_droppage((long)v[1]);
            }
            else if(strcmp(what, "chat") == 0)
            {
                display_chat(v[1], v[2], v[3]);
            }
            else if(strcmp(what, "c") == 0)
            {
                display_chat_private(v[1]);
            }
            else if(strcmp(what, "audio-recv-delay") == 0)
            {
                react_to_delay_change(v[1]);
            }
            else if(strcmp(what, "syncvar") == 0)
            {
                sync_recv(v[1]);
            }
            else if(strcmp(what, "msg") == 0)
            {
                if(store_message_callback)
                {
                    // let the callback send the response
                    // if any.
                    (*store_message_callback)(this, v[1], smcb_arg2);
                }
            }
            else if(strcmp(what, "uc") == 0)
            {
                if(uc_message_callback)
                {
                    (*uc_message_callback)(this, v[1], ucmcb_arg2);
                }
            }
            else if(strcmp(what, "aux_r") == 0)
            {
                aux_channel_setup(this, v);
            }
            else if(strcmp(what, "stun") == 0)
            {
                //got_peer_stun(v);
            }
        }

        break;
    }
    case VC_STRING:
        if(strcmp(v, "pause") == 0)
            pause = 1;
        else if(strcmp(v, "continue") == 0)
        {
            // probably want to force the xmitter to
            // send a reference frame here...
            pause = 0;
        }
        else if(strcmp(v, "ok") == 0)
        {
            if(got_ok_callback)
                (*got_ok_callback)(this, gcb_arg1, gcb_arg2, gcb_arg3);
        }
        else if(strcmp(v, "decrypt"))
        {
            tube->set_key_iv(agreed_key, 0);
            if(!tube->start_decrypt_ctrl())
            {
                schedule_destroy();
            }
        }
        break;
    default:
        break;
        // note: no default case, just ignore
        // unknown messages
    }
}

static
void
save_small_attachment(vc qqm, vc att_contents)
{
    vc file_basename = qqm[QQM_MSG_VEC][QQM_BODY_ATTACHMENT];
    if(!is_attachment(file_basename))
        return;
    DwString actual_filename = newfn(file_basename);
    // if an error happens in here, we can't really do much about it
    // except avoid leaving crumbs... when the message is eventually
    // displayed, an error will occur there.
    FILE *f = fopen(actual_filename.c_str(), "wb");
    if(!f)
    {
        DeleteFile(actual_filename.c_str());
        return;
    }
    if(fwrite((const char *)att_contents, att_contents.len(), 1, f) != 1)
    {
        fclose(f);
        DeleteFile(actual_filename.c_str());
        return;
    }
    fclose(f);
}

void
MMChannel::process_with_response(vc v)
{
    vc mtp = v[0];
    vc cmd = mtp[0];
    //vc serial = mtp[1];

    if(cmd == vc("msg"))
    {
        // store a message, and return a response
        if(store_message_callback)
        {
            // if there is a small inline attachment, drop that out now
            if(!v[2].is_nil())
            {
                save_small_attachment(v[1], v[2]);
            }
            (*store_message_callback)(this, v[1], smcb_arg2);
        }
        vc resp(VC_VECTOR);
        vc tp(VC_VECTOR);
        tp[0] = "resp";
        tp[1] = 0;
        resp[0] = tp;
        resp[1] = mtp; // (msg N)
        resp[2] = "t"; // we always say it worked, helps avoid info leaks
        send_ctrl(resp);
    }
    else if(cmd == vc("resp"))
    {
        // this comes in like this vector(vector(resp 0) vector(cmd N) results...)
        v.remove_first();
        server_response(v);
    }

}
