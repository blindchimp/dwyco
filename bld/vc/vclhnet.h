
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCLHNET_H
#define VCLHNET_H
// $Header: g:/dwight/repo/vc/rcs/vclhnet.h 1.49 1998/12/18 23:54:48 dwight Exp $


vc lh_socket(VCArglist *a);
vc lh_socket_from_os_handle(VCArglist *a);
vc lh_horrible_hack(VCArglist *a);
vc lh_sockclose(VCArglist *a);
vc lh_sockshutdown(VCArglist *a);
vc lh_accept(VCArglist *a);
vc lh_connect(VCArglist *a);
vc lh_poll(VCArglist *a);
vc lh_poll_all(VCArglist *a);
vc lh_socksend(VCArglist *a);
vc lh_sockrecv(VCArglist *a);
vc lh_socksendstring(VCArglist *a);
vc lh_sockrecvstring(VCArglist *a);
vc lh_socksend_buf(VCArglist *a);
vc lh_sockrecv_buf(VCArglist *a);
vc lh_sockset_option(VCArglist *a);
vc lh_gethostbyname(VCArglist *a);
vc lh_sock_peer_addr(VCArglist *a);
vc lh_sock_local_addr(VCArglist *a);
vc lh_poll_sets(VCArglist *a);
vc lh_add_write_set(VCArglist *a);
vc lh_add_read_set(VCArglist *a);
vc lh_del_write_set(VCArglist *a);
vc lh_del_read_set(VCArglist *a);
vc lh_clear_write_set();
vc lh_clear_read_set();
#endif

