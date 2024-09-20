
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef MMCALL_H
#define MMCALL_H

#include "pval.h"
#include "mmchan.h"
#include "vc.h"
#include "dwqbm.h"
#include "ssns.h"

namespace dwyco {
class MMCall;

typedef void (*CallStatusCallback)(MMCall *, int status, void *user_arg, ValidPtr);
class MMCall
{
public:
    MMCall(
        vc uid, vc host, vc port, vc proxinfo, int msg_chan, int user_control_chan,
        CallStatusCallback scb, void *scb_arg1, ValidPtr scb_arg2
    );
    virtual ~MMCall();

    ValidPtr vp;

    int start_call(int media_select);
    int cancel_call();

    virtual MessageDisplay *get_ui_msg_output() {
        return 0;
    }

    vc uid;
    vc host;
    vc port;
    vc proxinfo;
    vc reject_reason;

    // channels to request in this call.
    int send_video;		// we send video to them
    int recv_video;		// they send video to us
    int send_audio;
    int recv_audio;
    int pub_chat;		// always full duplex
    int priv_chat;		// likewise, full duplex
    vc password;		// call screening password

    // this is user-defined, and can be anything. it is injected into
    // the call setup data that is negotiated during setup so it is
    // available to the callee for whatever purpose it wants (usually
    // call screening.)
    vc call_type;

    // this didn't work out too well... we really can't set up
    // msg channels (and all their callback state) properly.
    // this was basically an attempt to set up a control channel
    // but there is too much other baggage to make it work right.
    // possibly it could be used for a proper "transport" based
    // implementation of msging in the future.
    int msg_chan;

    // this means the call is just a straight data channel
    // between the clients. it might be server assisted.
    // but it has to be TCP, and no media channels are set up.
    // it is kinda like a cached msg channel that can be used to
    // send arbitrary msgs directly between clients.
    int user_control_chan;

    CallStatusCallback scb;
    void *scb_arg1;
    ValidPtr scb_arg2;

    // when a call is finally completed, this is the id
    // of the channel that is handling the call
    int chan_id;

    // search for the mmchan id, and return the call object
    // that is using that channel. used for figuring out
    // how to destroy calls from the channel they are using.
    static MMCall *channel_to_call(int chan_id);
    static DwVec<ValidPtr> MMCalls;
    static dwyco::DwQueryByMember<MMCall> MMCalls_qbm;
    static DwVecP<MMCall> calls_by_type(vc tp);

    // set to 1 means the call should be stopped during
    // setup. has no effect after the setup processing has
    // finished.
    int cancel;

    // how to complete the call
    int media_select;

    ssns::signal4<MMCall *, int, void *, ValidPtr> call_sig;
    // if this is 1, then all the call setup is over and the call is
    // active. this is used if you want to distinguish between a call
    // that is currently in the process of being setup, and one that
    // is ready to accept data.
    int established;

    virtual void call_started();
    virtual void stun_established();
    virtual void stun_connect_failed();
    virtual void stun_setup_timeout();
    virtual void timer1_stun_expired();
    virtual void call_locally_canceled();
    virtual void direct_call_failed(int stop);
    virtual void direct_call_ok();
    virtual void timer1_expired();
    virtual void call_terminated();
    virtual void stun_servass_failure();
    virtual void call_rejected(const char *why);
};
}

#define MMCALL_ESTABLISHED 1	// call successful
#define MMCALL_CANCELED 2		// call was locally canceled during setup
#define MMCALL_FAILED 3			// call could not be completed
#define MMCALL_TERMINATED 4		// call is gracefully shutdown (by either side.)
#define MMCALL_REJECTED 5 		// call is declined from remote side.
#define MMCALL_STARTED 6		// call initiated

#endif
