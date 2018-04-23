
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCO_NO_UVSOCK
#ifndef VCLHUVSOCK_H
#define VCLHUVSOCK_H
// socket interface based on libuv

vc lh_uv_socket(vc protocol, vc local_addr, vc is_listen, vc reuse_addr);
vc lh_uv_sockclose(vc sock, vc how);
vc lh_uv_sockshutdown(vc sock, vc how);
vc lh_uv_connect(vc sock, vc remote_addr);
vc lh_uv_poll();
vc lh_uv_socksend(VCArglist *a);
vc lh_uv_sockrecv(VCArglist *a);
vc lh_uv_get_all();
#endif
#endif
