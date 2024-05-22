
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef NETDIAG_H
#define NETDIAG_H

#include "vc.h"
#include "dwstr.h"

struct probe_result
{
    DwString adapter_name;
    int res;
    probe_result() {
        res = 0;
    }
    int operator==(const probe_result&) const {
        return 0;
    }
};

//DwVec<probe_result> rfc3489(vc to, int& num_adapters, DwString&, int use_ipconfig = 1);

long simple_bw_in(DwString&);
long simple_bw_out(DwString&);
int upnp_probe(DwString& res, int use_ipconfig = 1);
int init_netdiag();

// one of
// open
// no-udp
// nat-symmetric
// nat-restricted
extern vc My_connection;

#define NETDIAG_NAT_OPEN 1
#define NETDIAG_NAT_UDP_FIREWALL 2
#define NETDIAG_NAT_ADDRESS_RESTRICTED 4
#define NETDIAG_NAT_PORT_RESTRICTED 8
#define NETDIAG_NAT_FULL_CONE 16
#define NETDIAG_NAT_NO_UDP 32
#define NETDIAG_NAT_CANT_BIND 64
#define NETDIAG_NAT_SYMMETRIC 128



#endif
