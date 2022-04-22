
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// this file contains the calling sequence to
// setup a multi-media call.
//
// in the past, we started by trying to set up the
// TCP control channel first directly to the recipient, and if
// that failed, the call failed. sometime after that i added
// the option of doing all-TCP calls, but didn't change the call
// setup, it was basically "can't get control channel, call fails".
// i *did* however, have fallback to UDP media if the TCP media
// channels failed to set up. all calls were defaulted to TCP, and
// that works fine in direct calls. the UDP fall back was never
// engaged except in wierd cases were the firewall setup allowed the
// TCP control channel, but not the media channels.
//
// now, circa 2005, so many people have NAT boxes, that it makes
// sense to go back to UDP, because we can use STUN-style protocols
// to figure out if we can traverse the NAT box using UDP (in a lot of
// cases, we can.) so, we have a composite setup if we can't get a
// direct TCP control channel set up, we set up the control channel
// via external server, and exchange info on how to traverse the
// NAT box. if the NAT box traversal fails, we can finally breakdown and
// send the media thru the external server as well. one hopes that is
// rare (or we have to charge money for it.)
//
//
// the basic chain of events is:
//
// * attempt direct control channel setup.
// 		if it works, groovy, continue on as in the past with direct call.
// * if that fails, setup a control channel via server rendevous.
//		if that fails, the intended recipient is probably offline.
//		if it works, both clients determine their NAT's capabilities
//		via STUN-like protocol. in virtually all cases, organizing
//		the UDP ports for a port-restricted style nat will allow
//		UDP packets to cross ok.
//		if one or the other NAT is symmetric, then we have to
//		give up and set up the media via the server.
// *
// UDP setup (i think this works for everything but symmetric NAT):
// caller punch hole: send UDP packet to IP-callee:A source port B
// packet will be dropped
// callee sends packet to IP-caller:B source port A
// punches hole in callee, and caller gets the packet.
// this can be done for pt-pt conferences on each connection setup as well.
//

// 7/2006: new calling scheme is extended like this:
// * try a direct TCP connect.
// * if that fails, *always* try connecting a control channel
//	via the proxy, unless it is inhibited.
// * wait for the connection to be established, and examine
//	the "my connection" item, and then decide what to do based on that:
//	(1) if they claim to be "open" or "no-udp", there is either a problem with
//		a firewall (since the tcp connect above didn't work) or
//		a tcp proxy might work.
//	(2) if they claim to be "nat-symmetric", then go straight for tcp
//		proxy.
//	(3) if they claim to be "nat-restricted", try STUN, unless stun
//		is inhibited, in which case, go for tcp proxy.
//
// note that there are a lot of cases where it might make sense to
// try and "reverse" a call if we are "open", but that isn't attempted here.
//
// also might want to ferret out routers that reject UDP packets
// based on size (there are some that seem to limit UDP packets to about
// 5k in size, which means hole-punching works, but then streaming the media
// doesn't work. sigh.) that isn't attempted here either.
//
// note: after a bit of use in the field,
// as a practical matter, STUN/UDP caused too many problems with users
// because if it didn't work, tech support would be left with no option other
// than helping them setup their firewall/NAT router. which is nearly impossible
// with most consumers. as a result, it is better to just set the TCP_ONLY option
// which tries direct, and if that fails, attempts TCP server assisted calling.
//

// this one is long because it includes whatever timeout is needed for
// waiting on the callee to accept/reject the call
#define CALLLIVE_SERVER_TIMEOUT (30000)
// timeout in case stun servers are not responding. this includes
// the time it takes to swap the stun data between the two clients
// during the last media setup.
#define CALLLIVE_STUN_TIMEOUT (10000)

#include "mmchan.h"

#include "servass.h"
#include "gvchild.h"
#include "ta.h"
#include "qauth.h"
#include "mmcall.h"
#include "calllive.h"
#include "qmsg.h"
#include "mmcall.h"
#include "dwrtlog.h"
#include "sysattr.h"
#include "netlog.h"

#ifdef _Windows
typedef unsigned long in_addr_t;
#endif

using namespace dwyco;


int Media_select = MEDIA_VIA_HANDSHAKE;
namespace dwyco {

int Disable_outgoing_SAC = 0;
void
call_terminated(MMChannel *mc, vc, void *, ValidPtr vp)
{
    mc->msg_out("Call Ended.");
    if(vp.is_valid())
    {
        MMCall *mmc = (MMCall *)(void *)vp;
        mmc->call_terminated();
        delete mmc;
    }
//this ends the "call", but the problem is, we really need to "end the channel"
//too, since old software assumes that when a channel is destroyed, that
//is the end of the call.
}

void
stun_connect_failed(MMChannel *mc, vc, void *, ValidPtr vp)
{
    // call totally fails at this point, can't get to proxy.
    // kluge: call screening errors like "password incorrect"
    // might have been displayed before this callback, so
    // we don't want to erase them.
    if(mc->fail_reason.is_nil())
    {
        TRACK_ADD(CL_stun_connect_failed, 1);
        mc->msg_out("Call failed. No Answer.");
        if(vp.is_valid())
        {
            MMCall *mmc = (MMCall *)(void *)vp;
            mmc->stun_connect_failed();
            delete mmc;
        }
    }
    else
    {
        TRACK_ADD(CL_call_screening_reject, 1);
        if(vp.is_valid())
        {
            MMCall *mmc = (MMCall *)(void *)vp;
            mmc->call_rejected(mc->fail_reason);
            delete mmc;
        }
    }
}

void
stun_setup_timeout(MMChannel *mc, vc, void *, ValidPtr vp)
{
    mc->msg_out("Can't contact STUN. Call failed.");
    mc->schedule_destroy();
    if(vp.is_valid())
    {
        MMCall *mmc = (MMCall *)(void *)vp;
        mmc->stun_setup_timeout();
        delete mmc;
    }
    TRACK_ADD(CL_STUN_timeout, 1);
}

void
stun_established(MMChannel *mc, vc, void *, ValidPtr vp)
{
    if(!mc->use_stun)
    {
        mc->msg_out("Connected via proxy.");
        mc->proxy_outgoing_bw_limit = sysattr_get_int("us-proxy-bw-limit");
        TRACK_ADD(CL_stun_established_tcp_proxy, 1);
    }
    else
    {
        mc->msg_out("Connected via STUN.");
        TRACK_ADD(CL_stun_established_stun, 1);
    }
    mc->timer1.stop();
    mc->destroy_callback = call_terminated;
    mc->dcb_arg3 = vp;
    if(vp.is_valid())
    {
        MMCall *mmc = (MMCall *)(void *)vp;
        if(mmc->cancel)
        {
            mmc->call_locally_canceled();
            delete mmc;
            return;
        }
        mmc->stun_established();
        mc->msg_chan = mmc->msg_chan;
        mc->user_control_chan = mmc->user_control_chan;
        // XXX NOTE: what other callbacks might we need to import here?
        // this is used in cases where we want to transport messages
        // over the control channel as well (previously, we assumed
        // we were *not* sending anything other than channel control
        // messages.) note that the proxy server setup doesn't grok
        // the "send the attachment via new connection" protocol, and
        // if you want to send small images or whatever via this
        // transport, you'll need to do it inline.
        if(mc->msg_chan)
            mc->store_message_callback = ::store_direct;
        // note: call exists until it is terminated somehow
    }
}

void
stun_connect_ok(MMChannel *mc, vc, void *, ValidPtr vp)
{
    MMCall *mmc = 0;
    if(vp.is_valid())
        mmc = (MMCall *)(void *)vp;
    if(mmc && mmc->cancel)
    {
        mc->schedule_destroy();
        mmc->call_locally_canceled();
        delete mmc;
        return;
    }
    // got to proxy ok, initiate a UDP connection with ports
    // setup for most typical NAT setups. a STUN probe at this point
    // could provide us with info on whether to send media via the
    // servers.
    // this would already have been done in the initial negotiations
    mc->force_unreliable_audio = 1;
    mc->force_unreliable_video = 1;

    mc->timer1.stop();
    mc->timer1.load(CALLLIVE_STUN_TIMEOUT);
    mc->timer1.start();
    mc->timer1_callback = stun_setup_timeout;
    mc->t1cb_arg3 = vp;
    mc->destroy_callback = 0;

    mc->msg_out("Proxy contacted...");

    // the channel should be in "established" state where the
    // initial negotiations are all done. at this point, we need to
    // query the STUN server to get some mapped addresses on the
    // outside of our NAT box, then whip those over to the
    // callee. assuming we have anything but a symmetric NAT
    // we should be in business when the callee sends us their
    // mapped addresses.
    // NOTE: we have to wait this long to get the STUN sockets
    // because they usually have a short timeout in a NAT situation.
    // and since this is the point where we know the user has
    // accepted the call, and the media is about to be set up,
    // this is a good place to do it. if we did it earlier, the
    // sockets might time out while we were waiting around for the user
    // to click "ok" on the call screening dialog.

    // note: removing this so it will compile, as soon as we enter
    // the WAIT_FOR_STUN state, the channel will be closed (STUN
    // is deprecated
#if 0
    // see if we can get all the sockets we need right now
    for(int i = 0; i < MM_STUN_SOCK_COUNT; ++i)
    {
        vc s = stun_pool_get_sock();
        if(s.is_nil())
            break;
        mc->stun_socks[i] = s;
    }
#endif
    mc->pstate = MMChannel::WAIT_FOR_STUN;

    mc->stun_established_callback = stun_established;
    mc->sec_arg3 = vp;
    TRACK_ADD(CL_stun_proxy_connect_ok, 1);
}

// note: this established-callback is called right at the point where
// initial negotiations are done, *and* before the media and network
// channels are setup.
void
tcp_proxy_connect_and_decide(MMChannel *mc, vc, void *, ValidPtr vp)
{
    MMCall *mmc = 0;
    if(vp.is_valid())
        mmc = (MMCall *)(void *)vp;
    if(mmc && mmc->cancel)
    {
        mc->schedule_destroy();
        mmc->call_locally_canceled();
        delete mmc;
        return;
    }

    // the channel should be in "established" state where the
    // initial negotiations are all done.
    //

    mc->timer1.stop();

    // decide what to do based on the remote callers connection
    if(mc->nego_media_select == vc("tcp via proxy"))
    {
        // tcp proxy
        // might want to check to see if they are registered here,
        // reduce the video quality to a reasonable level if not.
        TRACK_ADD(CL_tcp_handshake, 1);
        mc->force_unreliable_config = 0;
        mc->use_stun = 0;
        mc->force_unreliable_audio = 0;
        mc->force_unreliable_video = 0;
        GRTLOG("remote needs TCP media proxy", 0, 0);
        stun_established(mc, vcnil, 0, vp);
    }
    else if(mc->nego_media_select == vc("udp via stun"))
    {
        // try udp via stun
        TRACK_ADD(CL_udp_handshake, 1);
        mc->force_unreliable_config = 1;
        mc->use_stun = 1;
        mc->force_unreliable_audio = 1;
        mc->force_unreliable_video = 1;
        GRTLOG("remote can try STUN media", 0, 0);
        stun_connect_ok(mc, vcnil, 0, vp);
    }
    else
    {
        GRTLOG("can't figure out what remote wants", 0, 0);
        TRACK_ADD(CL_huh_handshake, 1);
    }
}

void
timer1_stun_expired(MMChannel *mc, vc, void *, ValidPtr vp)
{
    mc->schedule_destroy();
    // note: destroy callback will do final cleanup
    if(vp.is_valid())
        ((MMCall *)(void *)vp)->timer1_stun_expired();
    TRACK_ADD(CL_proxy_connect_timeout, 1);
}

static void
track_stun_connect(MMChannel *, vc what, void *, ValidPtr)
{
    if(what.is_nil())
    {
        TRACK_ADD(CL_ll_prox_connect_failed, 1);
    }
    else
    {
        TRACK_ADD(CL_ll_prox_connect_succeeded, 1);
    }
}

// this gets called in CALLER when the "server-assist" command
// returns from the server with a positive response that includes
// some rendevous information
void
stun_connect(vc host, vc port, vc prox, vc uid, int media_select, ValidPtr vp, MessageDisplay *mid)
{
    MMCall *mmc = 0;
    if(vp.is_valid())
        mmc = (MMCall *)(void *)vp;
    if(mmc && mmc->cancel)
    {
        mmc->call_locally_canceled();
        delete mmc;
        return;
    }
    // connect up to the host/port, it is a proxy
    // connection where the callee will rendevous with us
    // for control channel
    MMChannel *mc = new MMChannel;

    mc->msg_output = mid;
    mc->msg_out("Contacting proxy...");
    mc->start_service();
    mc->attempt_uid = uid;
    mc->port = (int)port;
    mc->proxy_info = prox;
    //mc->force_non_firewall_friendly = 1;
    if(media_select == MEDIA_UDP_VIA_STUN)
    {
        mc->destroy_callback = stun_connect_failed;
        mc->dcb_arg3 = vp;
        mc->established_callback = stun_connect_ok;
        mc->ecb_arg3 = vp;
        mc->force_unreliable_config = 1;
        mc->use_stun = 1;
        mc->media_setup = "udp via stun";
    }
    else if(media_select == MEDIA_TCP_VIA_PROXY)
    {
        mc->destroy_callback = stun_connect_failed;
        mc->dcb_arg3 = vp;
        mc->established_callback = stun_established;
        mc->ecb_arg3 = vp;
        mc->force_unreliable_config = 0;
        mc->use_stun = 0;
        mc->media_setup = "tcp via proxy";
    }
    else if(media_select == MEDIA_VIA_HANDSHAKE)
    {
        // wait for the initial capabilities negotiation
        // to finish to try and figure out what to do.
        mc->destroy_callback = stun_connect_failed;
        mc->dcb_arg3 = vp;
        mc->established_callback = tcp_proxy_connect_and_decide;
        mc->ecb_arg3 = vp;
        mc->force_unreliable_config = 0;
        mc->use_stun = 0;
        mc->media_setup = "via handshake";
    }
    mc->timer1.load(CALLLIVE_SERVER_TIMEOUT);
    mc->timer1.set_oneshot(1);
    mc->timer1.start();
    mc->timer1_callback = timer1_stun_expired;
    mc->t1cb_arg3 = vp;
    mc->low_level_connect_callback = track_stun_connect;

    if(mmc)
    {
        mmc->chan_id = mc->myid;
        mc->msg_chan = mmc->msg_chan;
        mc->user_control_chan = mmc->user_control_chan;
        mc->call_type = mmc->call_type;
    }

    in_addr_t addr;
    if((addr = inet_addr((const char *)host)) == INADDR_NONE)
    {
        // start connect process at resolve stage
        if(!mc->start_resolve(MMChannel::BYNAME, 0, (const char *)host))
        {
            mc->schedule_destroy(MMChannel::HARD);
        }
    }
    else
    {
        // start connect process with ip
        mc->addr_out.s_addr = addr;
        if(!mc->start_connect())
        {
            mc->schedule_destroy(MMChannel::HARD);
        }

        Netlog_signal.emit(mc->tube->mklog("peer_uid", to_hex(mc->attempt_uid)));

    }

}

void
stun_servass_failure(const char *msg, vc to_uid, ValidPtr vp)
{
    if(vp.is_valid())
    {
        MMCall *mmc = (MMCall *)(void *)vp;
        mmc->stun_servass_failure();
        delete mmc;
    }
}

void
direct_call_failed(MMChannel *mc, vc, void *, ValidPtr vp)
{
    MMCall *mmc = 0;
    if(vp.is_valid())
        mmc = (MMCall *)(void *)vp;
    // call may have been rejected
    if(!mc->fail_reason.is_nil() && mc->rejected)
    {
        if(mmc)
        {
            mmc->call_rejected(mc->fail_reason);
            delete mmc;
        }
        TRACK_ADD(CL_call_screening_reject, 1);
        return;
    }
    else if(mc->pstate == MMChannel::WAIT_FOR_MATCH || mc->pstate == MMChannel::WAIT_FOR_MATCH_CRYPTO)
    {
//		// if we connected directly, but the timer expires
//		// waiting for the accept/reject from the other side,
//		// that stops the calling sequence
//		mc->msg_out("Call failed. No Answer.");
//		if(mmc)
//		{
//			mmc->call_rejected("no answer");
//			delete mmc;
//		}
//		TRACK_ADD(CL_call_no_answer, 1);
//		return;
        // ca. 2015, we don't assume there is someone on
        // the other end based on low-level connect status.
        // if the call fails at this point, it is just because
        // the protocol isn't advancing quick enough, so just
        // call it a fail and continue on.
        mc->msg_out("Call failed, taking too long.");
        if(mmc)
        {
            mmc->call_rejected("protocol taking too long");
        }
        TRACK_ADD(CL_call_setup_taking_too_long, 1);
    }
    if(mc->cancel) // punting altogether
    {
        if(mmc)
        {
            mmc->call_locally_canceled();
            delete mmc;
        }
        return;
    }
    if(mmc && mmc->cancel)
    {
        mmc->call_locally_canceled();
        delete mmc;
        return;
    }
    if(Disable_outgoing_SAC || (mmc && (mmc->media_select == MEDIA_TCP_DIRECT)))
    {
        // signal call failure and stop the progression
        if(mmc)
        {
            mmc->direct_call_failed(1);
            delete mmc;
        }
        mc->msg_out("Call failed (SAC not attempted.)");
        TRACK_ADD(CL_SAC_disabled, 1);
        return;
    }
    // attempt to set up a control channel via server rendevous

    switch(mmc ? mmc->media_select : Media_select)
    {
    case MEDIA_TCP_VIA_PROXY:
        mc->msg_out("TCP SAC...");
        break;
    case MEDIA_UDP_VIA_STUN:
        mc->msg_out("UDP SAC...");
        break;
    default:
    case MEDIA_VIA_HANDSHAKE:
        mc->msg_out("Server assisted call...");
        break;
    }
    // note: at this point, the call has no "chan_id", since there is
    // no channel associated with the call (we are just contacting
    // the server via an already setup connection in order to
    // start the next phase of the call setup.
    if(mmc)
        mmc->chan_id = -2; // -2 is just something other than valid or -1, no significance
    start_server_assisted_call(mc->attempt_uid,
                               (mmc ? mmc->media_select : Media_select), vp, mc->msg_output);

    TRACK_ADD(CL_call_setup_direct_failed, 1);

    // note: even tho this says "failed", it continues to
    // the next step of call setup
    if(mmc)
        mmc->direct_call_failed(0);
}

void
direct_call_ok(MMChannel *mc, vc, void *, ValidPtr vp)
{
    mc->destroy_callback = call_terminated;
    mc->dcb_arg3 = vp;
    mc->timer1.stop();
    mc->msg_out("Direct call established");
    TRACK_ADD(CL_direct_ok, 1);
    TRACK_ADD(CL_direct_total_time, CALLLIVE_DIRECT_TIMEOUT - mc->timer1.get_time_left());
    if(vp.is_valid())
    {
        MMCall *mmc = (MMCall *)(void *)vp;
        mmc->direct_call_ok();
        mc->msg_chan = mmc->msg_chan;
        mc->user_control_chan = mmc->user_control_chan;
    }
}

void
timer1_expired(MMChannel *mc, vc, void *, ValidPtr vp)
{
    mc->schedule_destroy();
    TRACK_ADD(CL_direct_timeout, 1);
    // note: destory callback will handle cleanup and switch to
    // next calling state.
    if(vp.is_valid())
    {
        MMCall *mmc = (MMCall *)(void *)vp;
        mmc->timer1_expired();
    }
}
}

