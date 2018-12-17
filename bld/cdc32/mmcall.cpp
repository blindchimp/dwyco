
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// call objects
// these objects encapsulate a call that is in progress.
// the main reason we need these objects is because we
// now have a complicated set of things we try in order to
// get a call going between two computers. in the past, we just
// attempted one direct connect, and if it failed, that was it.
// now we can ask servers for setup, proxies of various kinds for
// NAT/firewall info, etc.etc. MMChannels are already too complicated
// to hang all this off of them, and it is better if they are just
// lefts as "one shot" attempt objects rather than trying to extend them.
//
// in addition, we eventually want to have some kinda outgoing call
// q in order to limit the number of simultaneous calls, and these
// objects could be used in that area as well.
//
// each call object represents a single call setup. once the call
// is going (does/doesn't) the object get destroyed, not sure.

//

#include "mmcall.h"
#include "calllive.h"
#include "dwrtlog.h"
#include "qauth.h"
#include "pval.h"
#include "qmsg.h"

using namespace dwyco;

DwVec<ValidPtr> MMCall::MMCalls;
DwQueryByMember<MMCall> MMCall::MMCalls_qbm;

MMCall::MMCall(vc auid, vc ahost, vc aport, vc aproxinfo, int amsg_chan,
               int auser_control,
               CallStatusCallback ascb, void *ascb_arg1, ValidPtr ascb_arg2)
    : vp(this)
{
    uid = auid;
    host = ahost;
    port = aport;
    proxinfo = aproxinfo;
    msg_chan = amsg_chan;
    scb = ascb;
    scb_arg1 = ascb_arg1;
    scb_arg2 = ascb_arg2;
    cancel = 0;
    chan_id = -1;
    media_select = MEDIA_TCP_DIRECT;
    send_video = 0;
    recv_video = 0;
    send_audio = 0;
    recv_audio = 0;
    priv_chat = 0;
    pub_chat = 0;
    user_control_chan = auser_control;
    established = 0;
    MMCalls.append(vp);
    MMCalls_qbm.add(this);
}

MMCall::~MMCall()
{
    int i = MMCalls.index(vp);
    if(i == -1)
        oopanic("mmcall not on list");
    MMCalls.del(i);
#ifdef DWYCO_TRACE
    void invalidate_cb_ctx(int);
    invalidate_cb_ctx(vp.cookie);
#endif
    vp.invalidate();
    MMCalls_qbm.del(this);
}

MMCall *
MMCall::channel_to_call(int chan_id)
{
    int n = MMCalls.num_elems();
    for(int i = 0; i < n; ++i)
    {
        if(MMCalls[i].is_valid())
        {
            MMCall *mmc = (MMCall *)(void *)MMCalls[i];
            if(mmc->chan_id == chan_id)
                return mmc;
        }
    }
    return 0;
}

// this is called after a physical connect has occurred on a direct
// connection attempt. we boost the timeout to something larger
// so that the user has a chance to respond to accept/reject
// dialogs and so on.
//
// ca. 2015, this is goofy because we might be talking to a proxy
// and trying to imply that a dialog is popped up on the other
// side based on the connection status isn't a good idea.
// for now, we'll just give it a little more time because we
// at least know it might be worth spending more time to get the
// connection set up.
static void
timer1_boost(MMChannel *mc, vc succ, void *, ValidPtr vp)
{
    if(succ.is_nil())
    {
        // connect failed, do nothing
        mc->low_level_connect_callback = 0;
        return;
    }
    if(vp.is_valid())
    {
        GRTLOG("boosting timer1 on direct connect for %d", mc->myid, 0);
        mc->timer1.load(CALLLIVE_DIRECT_TIMEOUT);
        mc->timer1.start();
    }
    mc->low_level_connect_callback = 0;
}

int
MMCall::start_call(int media_sel)
{
    MMChannel *mc = new MMChannel;

    mc->msg_output = get_ui_msg_output();
    mc->start_service();
    mc->attempt_uid = uid;
    mc->port = (int)port;
    mc->proxy_info = proxinfo;
    mc->destroy_callback = ::direct_call_failed;
    mc->dcb_arg3 = vp;
    mc->established_callback = ::direct_call_ok;
    mc->ecb_arg3 = vp;
    // i couldn't decide if this would be a good idea or not:
    // in the case of msgs, it is good because the result is
    // faster delivery of all subsequent msgs, but in the case of
    // calls, you might be rendering cdc32 crippled to call someone
    // that had a temporary failure (requiring a restart of cdc32).
    // so, as a compromise, we'll just halve the timeout to speed things
    // up since *probably* you can't call them directly.
    if(No_direct_msgs.contains(mc->attempt_uid))
    {
        mc->timer1.load(CALLLIVE_DIRECT_TIMEOUT / 2);
    }
    else
        mc->timer1.load(CALLLIVE_DIRECT_TIMEOUT);
    mc->timer1.set_oneshot(1);
    mc->timer1.start();
    mc->timer1_callback = ::timer1_expired;
    mc->t1cb_arg3 = vp;
    // XXX NOTE: setup low level connect callback to cause
    // timer1 to be incremented by some 30 seconds or so to
    // allow the negotation *and* the user to accept/reject the call.
    mc->low_level_connect_callback = timer1_boost;
    mc->llcc_arg1 = vcnil;
    mc->llcc_arg2 = 0;
    mc->llcc_arg3 = vp;
    GRTLOGA("start call to %s id: %d chan id %d, %s:%d", (const char *)to_hex(uid), vp.cookie, mc->myid, (const char *)host, (int)port);

    mc->msg_chan = msg_chan;
    mc->user_control_chan = user_control_chan;
    if(msg_chan)
        mc->store_message_callback = ::store_direct;

    chan_id = mc->myid;
    this->media_select = media_sel;
    mc->call_type = call_type;

    in_addr_t addr;
    if((addr = inet_addr((const char *)host)) == INADDR_NONE)
    {
        // start connect process at resolve stage
        if(!mc->start_resolve(MMChannel::BYNAME, 0, (const char *)host))
        {
            mc->schedule_destroy(MMChannel::HARD);
            return 0;
        }


    }
    else
    {
        // start connect process with ip
        mc->addr_out.s_addr = addr;
        if(!mc->start_connect())
        {
            mc->schedule_destroy(MMChannel::HARD);
            return 0;
        }

    }
    call_started();
    return 1;
}

int
MMCall::cancel_call()
{
    // this is a little tough, depends on what stage the
    // call is in, how we cancel it.
    cancel = 1;
    MMChannel *mc = MMChannel::channel_by_id(chan_id);
    if(mc)
        mc->schedule_destroy();
    return 1;
}

void
MMCall::call_started()
{
    GRTLOG("call started callback call_id %d chan_id %d", vp.cookie, chan_id);
    if(scb)
        (*scb)(this, MMCALL_STARTED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_STARTED, scb_arg1, scb_arg2);
}
void
MMCall::stun_connect_failed()
{
    GRTLOG("stun_connect_failed call_id %d chan_id %d", vp.cookie, chan_id);
    if(scb)
        (*scb)(this, MMCALL_FAILED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_FAILED, scb_arg1, scb_arg2);
}
void
MMCall::stun_setup_timeout()
{
    GRTLOG("stun_setup_timeout call_id %d chan_id %d", vp.cookie, chan_id);
    if(scb)
        (*scb)(this, MMCALL_FAILED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_FAILED, scb_arg1, scb_arg2);
}
void
MMCall::timer1_stun_expired()
{
    GRTLOG("timer1_stun_expired call_id %d chan_id %d", vp.cookie, chan_id);
    if(scb)
        (*scb)(this, MMCALL_FAILED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_FAILED, scb_arg1, scb_arg2);
}
void
MMCall::call_locally_canceled()
{
    GRTLOG("call_locally_canceled call_id %d chan_id %d", vp.cookie, chan_id);
    if(scb)
        (*scb)(this, MMCALL_CANCELED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_CANCELED, scb_arg1, scb_arg2);
}
void
MMCall::direct_call_failed(int stop)
{
    GRTLOG("direct_call_failed call_id %d chan_id %d", vp.cookie, chan_id);
    if(scb && stop)
        (*scb)(this, MMCALL_FAILED, scb_arg1, scb_arg2);
    if(stop)
        call_sig.emit(this, MMCALL_FAILED, scb_arg1, scb_arg2);
}
void
MMCall::direct_call_ok()
{
    GRTLOG("direct_call_ok call_id %d chan_id %d", vp.cookie, chan_id);
    established = 1;
    if(scb)
        (*scb)(this, MMCALL_ESTABLISHED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_ESTABLISHED, scb_arg1, scb_arg2);

}
void
MMCall::stun_established()
{
    GRTLOG("stun_established call_id %d chan_id %d", vp.cookie, chan_id);
    established = 1;
    if(scb)
        (*scb)(this, MMCALL_ESTABLISHED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_ESTABLISHED, scb_arg1, scb_arg2);

}
void
MMCall::timer1_expired()
{
    GRTLOG("timer1_expired call_id %d chan_id %d", vp.cookie, chan_id);
}

void
MMCall::call_terminated()
{
    GRTLOG("call_terminated call_id %d chan_id %d", vp.cookie, chan_id);
    if(scb)
        (*scb)(this, MMCALL_TERMINATED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_TERMINATED, scb_arg1, scb_arg2);
}
void
MMCall::stun_servass_failure()
{
    GRTLOG("stun_servass_failure call_id %d chan_id %d", vp.cookie, chan_id);
    if(scb)
        (*scb)(this, MMCALL_FAILED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_FAILED, scb_arg1, scb_arg2);
}

void
MMCall::call_rejected(const char *why)
{
    GRTLOGA("call_rejected call_id %d chan_id %d (%s)", vp.cookie, chan_id, why, 0, 0);
    reject_reason = why;
    if(scb)
        (*scb)(this, MMCALL_REJECTED, scb_arg1, scb_arg2);
    call_sig.emit(this, MMCALL_REJECTED, scb_arg1, scb_arg2);
}
