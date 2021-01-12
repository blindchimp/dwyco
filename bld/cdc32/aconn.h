
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

namespace dwyco {

void turn_accept_off();
void turn_accept_on();
void poll_listener();
void set_listen_state(int on);
int is_listening();

//struct DwNetConfig
//{

//public:

//    DWUIDECL_BEGIN
//    DWUIDECLVAL(int, primary_port)
//    DWUIDECLVAL(int, secondary_port)
//    DWUIDECLVAL(int, pal_port)
//    DWUIDECLVAL(int, nat_primary_port)
//    DWUIDECLVAL(int, nat_secondary_port)
//    DWUIDECLVAL(int, nat_pal_port)
//    DWUIDECLVAL(bool, advertise_nat_ports)
//    DWUIDECLVAL(int, disable_upnp)
//    DWUIDECLVAL(int, call_setup_media_select)
//    DWUIDECLVAL(int, listen)
//    DWUIDECL_END

//    DwNetConfig();
//    void load();
//    void save();

//    DwString get_primary_suffix(const char *ip);
//    DwString get_secondary_suffix(const char *ip);
//    DwString get_pal_suffix(const char *ip);

//    DwString get_nat_primary_suffix(const char *ip);
//    DwString get_nat_secondary_suffix(const char *ip);
//    DwString get_nat_pal_suffix(const char *ip);
//};

//extern DwNetConfig DwNetConfigData;
}

#define CSMS_DIRECT_ONLY 0
#define CSMS_TCP_ONLY 1
#define CSMS_UDP_ONLY 2
#define CSMS_VIA_HANDSHAKE 3

#endif
