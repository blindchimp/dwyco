
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vcdeflt.h"
#include "vcmap.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vc2.cpp 1.48 1998/08/06 04:44:12 dwight Exp $";

vc
vc::socket_init(const vc& local_addr, int listen, int reuse, int syntax) { return rep->socket_init(local_addr, listen, reuse, syntax); }
vc
vc::socket_init(SOCKET s, vc listening, int syntax) { return rep->socket_init(s, listening, syntax); }
vc
vc::socket_accept(vc& addr_info) { return rep->socket_accept(addr_info); }
vc
vc::socket_connect(const vc& addr_info) { return rep->socket_connect(addr_info); }
vc
vc::socket_close(int close_info) { return rep->socket_close(close_info); }
vc
vc::socket_shutdown(int how) { return rep->socket_shutdown(how); }
int
vc::socket_poll(int what_for, long to_sec, long to_usec) { return rep->socket_poll(what_for, to_sec, to_usec); }
vc
vc::socket_send(vc item, const vc& send_info, const vc& to) { return rep->socket_send(item, send_info, to); }
vc
vc::socket_recv(vc& item, const vc& recv_info, vc& addr_info) { return rep->socket_recv(item, recv_info, addr_info); }
vc
vc::socket_send_raw(void *buf, long& len, long timeout, const vc& to) { return rep->socket_send_raw(buf, len, timeout, to); }
vc
vc::socket_recv_raw(void *buf, long& len, long timeout, vc& addr_info) { return rep->socket_recv_raw(buf, len, timeout, addr_info); }
vc
vc::socket_error()
{
	return rep->socket_error();
}
vc
vc::socket_error_desc()
{
	return rep->socket_error_desc();
}
void
vc::socket_set_error(vc v)
{
	rep->socket_set_error(v);
}
vc
vc::socket_set_option(vcsocketmode m, unsigned long a1, unsigned long a2, unsigned long a3)
{
	return rep->socket_set_option(m, a1, a2, a3);
}

vcsocketmode
vc::socket_get_mode()
{
	return rep->socket_get_mode();
}

vc
vc::socket_local_addr()
{
	return rep->socket_local_addr();
}

vc
vc::socket_peer_addr()
{
	return rep->socket_peer_addr();
}

void vc::socket_add_read_set() {rep->socket_add_read_set();}
void vc::socket_add_write_set() {rep->socket_add_write_set();}
void vc::socket_del_read_set() {rep->socket_del_read_set();}
void vc::socket_del_write_set() {rep->socket_del_write_set();}
vc vc::socket_recv_buf(vc& item, int to_get, const vc& recv_info, vc& addr_info)
{return rep->socket_recv_buf(item, to_get, recv_info, addr_info);}
vc vc::socket_send_buf(vc item, const vc& send_info, const vc& dum)
{return rep->socket_send_buf(item, send_info, dum);}

vc
vc::socket_get_obj(int& avail, vc& addr_info)
{
    return rep->socket_get_obj(avail, addr_info);
}

vc
vc::socket_put_obj(vc obj, const vc& to_addr, int syntax)
{
    return rep->socket_put_obj(obj, to_addr, syntax);
}

int
vc::socket_get_write_q_size()
{
    return rep->socket_get_write_q_size();
}

int
vc::socket_get_read_q_len()
{
    return rep->socket_get_read_q_len();
}

