
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ACONN_H
#define ACONN_H

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
extern ssns::signal2<vc, int> Local_uid_discovered;
extern vc App_ID;
}

// these are fields in the broadcast packet
// the limit below tells how many slots will
// be deserialized. if there are more slots
// in the packet we receive, the entire packet
// is dropped.
#define BD_IP 0
#define BD_PRIMARY_PORT 1
#define BD_SECONDARY_PORT 2
#define BD_PAL_PORT 3
#define BD_NICE_NAME 4
// we have lots of different apps, but don't really want them
// interacting in most cases. set this to a unique id to filter
// out unwanted broadcasts
#define BD_APP_ID 5
// foreground/background
#define BD_DISPOSITION 6


// note: this is used to tell the deserializer how to limit input
// leaving a little slack is a good idea for forward compat
#define BD_DESERIALIZE_LIMIT 8

#define CSMS_DIRECT_ONLY 0
#define CSMS_TCP_ONLY 1
#define CSMS_UDP_ONLY 2
#define CSMS_VIA_HANDSHAKE 3

#endif
