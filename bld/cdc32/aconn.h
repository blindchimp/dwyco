
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ACONN_H
#define ACONN_H

#include "dwstr.h"
#include "ssns.h"
#include "vc.h"

namespace dwyco {

void init_aconn();
void turn_accept_off();
void turn_accept_on();
void poll_listener();
void set_listen_state(int on);
int is_listening();
extern vc Broadcast_discoveries;
extern ssns::signal1<vc> Local_uid_discovered;

}

#define CSMS_DIRECT_ONLY 0
#define CSMS_TCP_ONLY 1
#define CSMS_UDP_ONLY 2
#define CSMS_VIA_HANDSHAKE 3

#endif
