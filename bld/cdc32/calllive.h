
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CALLLIVE_H
#define CALLLIVE_H
#include "vc.h"
#include "pval.h"

class MessageDisplay;
class MMChannel;

namespace dwyco {
void stun_connect(vc host, vc port, vc prox, vc uid, int media_select, ValidPtr, MessageDisplay *md = 0);
//void direct_conference_connect(vc hostlist, vc portlist, vc proxlist, vc uid_list, TEditId *mid = 0);
void tcp_proxy_connect_and_decide(MMChannel *mc, vc, void *, ValidPtr vp);

void timer1_expired(MMChannel *mc, vc, void *, ValidPtr vp);
void direct_call_ok(MMChannel *mc, vc, void *, ValidPtr vp);
void direct_call_failed(MMChannel *mc, vc, void *, ValidPtr vp);
void timer1_stun_expired(MMChannel *mc, vc, void *, ValidPtr vp);
void stun_connect_ok(MMChannel *mc, vc, void *, ValidPtr vp);
void stun_setup_timeout(MMChannel *mc, vc, void *, ValidPtr vp);
void stun_connect_failed(MMChannel *mc, vc, void *, ValidPtr vp);
void stun_servass_failure(const char *msg, vc to_uid, ValidPtr vp);
extern int Disable_outgoing_SAC;
}

// time to set up a direct call (including initial negotiation).
#define CALLLIVE_DIRECT_TIMEOUT (4000)

// these modes tell how the state machine should proceed as a direct
// call is attempted. essentially, "direct call" means creating a real-time
// channel to another client (even if it is through a proxy). in all cases,
// a direct call is tried first. if you have no valid ip address, that will fail
// if you select one of the other modes, it can proceed to try and find another
// way to set up the call.

// stop after trying a direct connection (if it fails, do not attempt other methods)
#define MEDIA_TCP_DIRECT 0
// try using the dwyco tcp proxy servers to do the connection. fails if the server determines
// the recipient is not online (could possibly mean the recipient is also invisible.)
#define MEDIA_TCP_VIA_PROXY 1
// try using the STUN servers/techniques to get a UDP stream started. this is defunct, it was never reliable
// enough to justify the problems it caused tech support.
#define MEDIA_UDP_VIA_STUN 2
// this one waits until the control channel is set up, and performs a selection based on what we think our
// connection is capable of (as far as NAT) and what the remote side has as well (match_config).
// i think the idea was that if it was obvious udp could never work because one side or the other was
// behind a restrictive NAT, we could just to TCP and ignore other config options.
// i don't think this was ever used, and may not work in any interesting way.
#define MEDIA_VIA_HANDSHAKE 3

#endif
