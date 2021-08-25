
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
// time to allow user to accept/reject a call
#define CALLLIVE_DIRECT_USER_TIMEOUT (30000)

#define MEDIA_TCP_DIRECT 0
#define MEDIA_TCP_VIA_PROXY 1
#define MEDIA_UDP_VIA_STUN 2
#define MEDIA_VIA_HANDSHAKE 3

#endif
