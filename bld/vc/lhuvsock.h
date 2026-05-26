
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

vc lh_uv_socket(const vc& protocol, const vc& local_addr, const vc& is_listen, const vc& reuse_addr);
vc lh_uv_sockclose(VCArglist *a);
vc lh_uv_sockshutdown(VCArglist *a);
vc lh_uv_connect(VCArglist *a);
vc lh_uv_poll();
vc lh_uv_socksend(VCArglist *a);
vc lh_uv_sockrecv(VCArglist *a);
vc lh_uv_get_all();
#endif
#endif
