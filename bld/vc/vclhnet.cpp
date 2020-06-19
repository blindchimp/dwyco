
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vclhnet.cpp 1.52 1998/12/18 23:54:48 dwight Exp $";

#include "vclhnet.h"
#include "dwvec.h"
#include "vcmap.h"
#include "vcsock.h"
#include "vcwsock.h"
#ifdef LINUX
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#ifdef _WIN32
#include <WinSock2.h>
#endif

static int
lh_socket_error(vc *vs)
{
	vc_socket *v = (vc_socket *)vs;
	if(v->emode == EXCEPTIONS)
	{
		// construct exception string and raise LH exception
		char exc[80]; //XXX fix this
		exc[0] = v->exc_level;
		exc[1] = ':';
		exc[2] = 0;
		strcat(exc, "LHNET.");
		strcat(exc, (const char *)v->errvc);
		VCArglist a;
		vc excstr(exc);
        a.append(excstr);
		vc v2;
		v2.attach(v);
        a.append(v2);
        a.append(v->excretval);
		Vcmap->excraise(excstr, &a);
		CHECK_ANY_BO(VC_SOCKET_BACKOUT);
		return VC_SOCKET_RESUME;
	}
	return VC_SOCKET_BACKOUT;
}

vc
lh_horrible_hack(vc sock)
{
	if(sock.type() != VC_SOCKET)
	{
		USER_BOMB("arg to os_socket must be socket", vcnil);
	}
	vc_winsock *w = (vc_winsock *)sock.nonono();
	return vc((int)w->sock);
}

vc
lh_socket_from_os_handle(vc os_handle, vc protocol, vc listening)
{
	vc sock;
	if(protocol.type() != VC_STRING ||
		(protocol != vc("tcp") && protocol != vc("udp")))
	{
		USER_BOMB("second arg to socket_from_os must be tcp or udp", vcnil);
	}
	if(protocol == vc("tcp"))
		sock = vc(VC_SOCKET_STREAM);
	else if(protocol == vc("udp"))
		sock = vc(VC_SOCKET_DGRAM);
	sock.set_err_callback(lh_socket_error);
	SOCKET s = -1;
	if(os_handle.type() == VC_STRING)
	{
		// try to convert to integer
		s = (SOCKET)atoi((const char *)os_handle);
	}
	else if(os_handle.type() == VC_INT)
	{
		s = (SOCKET)(int)os_handle;
	}
	else
	{
		USER_BOMB("os handle must be string number or integer", vcnil);
	}
	sock.socket_init(s, listening);
	NONLH_CHECK_ANY_BO(vcnil);
	return sock;
}
vc
lh_socket(vc protocol, vc local_addr, vc is_listen, vc reuse_addr)
{
	vc sock;
	static vc tcp("tcp");
	static vc udp("udp");
	static vc unixx("unix");
	if(protocol.type() != VC_STRING ||
		(protocol != tcp && protocol != udp && protocol != unixx))
	{
		USER_BOMB("first arg to socket must be tcp or udp or unix", vcnil);
	}
	if(protocol == tcp)
		sock = vc(VC_SOCKET_STREAM);
	else if(protocol == udp)
		sock = vc(VC_SOCKET_DGRAM);
	else if(protocol == unixx) // not quite right, but ok for now
		sock = vc(VC_SOCKET_STREAM_UNIX);
	sock.set_err_callback(lh_socket_error);
	sock.socket_init(local_addr, is_listen.is_nil() ? 0 : 1,
		 reuse_addr.is_nil() ? 0 : 1);
	CHECK_ANY_BO(vcnil);
	return sock;
}

vc
lh_sockclose(vc sock, vc how)
{
	vc v = sock.socket_close(how.is_nil() ? 0 : 1);
	CHECK_ANY_BO(vcnil);
	return v;
}

vc
lh_sockshutdown(vc sock, vc how)
{
	static vc r("r");
	static vc w("w");
	static vc rw("rw");
	vc v;

	if(how == r)
		v = sock.socket_shutdown(VC_SOCK_SHUTDOWN_RD);
	else if(how == w)
		v = sock.socket_shutdown(VC_SOCK_SHUTDOWN_WR);
	else if(how == rw)
		v = sock.socket_shutdown(VC_SOCK_SHUTDOWN_BOTH);
    else
    {
        USER_BOMB("how must be r, w, or rw", vcnil);
    }

	CHECK_ANY_BO(vcnil);
	return v;
}

vc
lh_accept(vc sock, vc peer_addr)
{
	vc tmp;
	vc newsock = sock.socket_accept(tmp);
	CHECK_ANY_BO(vcnil);
	if(peer_addr.type() == VC_STRING)
		peer_addr.local_bind(tmp);
	else
	{
		USER_BOMB("peer address atom must be string", vcnil);
	}
	return newsock;
}

vc
lh_connect(vc sock, vc remote_addr)
{
	vc v = sock.socket_connect(remote_addr);
	CHECK_ANY_BO(vcnil);
	return v;
}

vc
lh_poll(vc sock, vc whatfor, vc sec, vc usec)
{
	if(whatfor.type() != VC_STRING)
	{
		USER_BOMB("polling arg must be [rwe]", vcnil);
	}
	const char *wf = whatfor;
    int len = strlen(wf);
	int pargs = 0;
    for(int i = 0; i < len; ++i)
    {
		switch(wf[i])
		{
		case 'r':
			pargs |= VC_SOCK_READ;
			break;
		case 'w':
			pargs |= VC_SOCK_WRITE;
			break;
		case 'e':
			pargs |= VC_SOCK_ERROR;
			break;
		default:
			USER_BOMB("bogus polling arg, must be from [rwe]", vcnil);
		}
    }
		
	long tsec, tusec;
	if(!sec.is_nil())
		tsec = (long)sec;
	else
		tsec = -1;
	if(!usec.is_nil())
		tusec = (long)usec;
	else
		tusec = -1;
		
	int ready = sock.socket_poll(pargs, tsec, tusec);
	CHECK_ANY_BO(vcnil);

	vc ret(VC_VECTOR);
	if(ready & VC_SOCK_ERROR)
		ret.append(vc("e"));
	if(ready & VC_SOCK_READ)
		ret.append(vc("r"));
	if(ready & VC_SOCK_WRITE)
		ret.append(vc("w"));
		
	return ret;
}

vc
lh_poll_all(VCArglist *a)
{
	if(a->num_elems() < 1 || a->num_elems() > 3)
	{
		USER_BOMB("usage: poll-all r|w|e [sec] [usec]", vcnil);
	}
	vc whatfor = (*a)[0];
	if(whatfor.type() != VC_STRING)
	{
		USER_BOMB("polling arg must be [rwe]", vcnil);
	}
	int sec = 0;
	int usec = 0;
	if(a->num_elems() >= 2) 
	{
		if((*a)[1].type() != VC_INT)
		{
			USER_BOMB("polling timeout args must be integer", vcnil);
		}
		sec = (long)((*a)[1]);
	}
		
	if(a->num_elems() == 3)
	{
		if((*a)[2].type() != VC_INT)
		{
			USER_BOMB("polling timeout args must be integer", vcnil);
		}
		usec = (long)((*a)[2]);
	}

	const char *wf = whatfor;
    int len = strlen(wf);
	int pargs = 0;
	int i;
    for(i = 0; i < len; ++i)
    {
		switch(wf[i])
		{
		case 'r':
			pargs |= VC_SOCK_READ;
			break;
		case 'w':
			pargs |= VC_SOCK_WRITE;
			break;
		case 'e':
			pargs |= VC_SOCK_ERROR;
			break;
		default:
			USER_BOMB("bogus polling arg, must be from [rwe]", vcnil);
		}
    }
		
	Socketvec results;
	int n = vc_winsock::poll_all(pargs, results, sec, usec);
	CHECK_ANY_BO(vcnil);
	if(n < 0)
		return vcnil;
	vc retvec(VC_VECTOR, "", n);
	for(i = 0; i < n; ++i)
	{
		vc sock;
		vc ret(VC_VECTOR);
		sock.attach(results[i]);
		ret[0] = sock;
		vc ret2(VC_VECTOR);
		int ready = results[i]->status;
		if(ready & VC_SOCK_ERROR)
			ret2.append(vc("e"));
		if(ready & VC_SOCK_READ)
			ret2.append(vc("r"));
		if(ready & VC_SOCK_WRITE)
			ret2.append(vc("w"));
		ret[1] = ret2;
		retvec[i] = ret;
	}
	return retvec;
}

vc
lh_poll_sets(VCArglist *a)
{
	if(a->num_elems() < 1 || a->num_elems() > 3)
	{
		USER_BOMB("usage: poll-all r|w|e|!|- [sec] [usec]", vcnil);
	}
	vc whatfor = (*a)[0];
	if(whatfor.type() != VC_STRING)
	{
		USER_BOMB("polling arg must be [rwe!-]", vcnil);
	}
	int sec = 0;
	int usec = 0;
	if(a->num_elems() >= 2) 
	{
		if((*a)[1].type() != VC_INT)
		{
			USER_BOMB("polling timeout args must be integer", vcnil);
		}
		sec = (long)((*a)[1]);
	}
		
	if(a->num_elems() == 3)
	{
		if((*a)[2].type() != VC_INT)
		{
			USER_BOMB("polling timeout args must be integer", vcnil);
		}
		usec = (long)((*a)[2]);
	}

	const char *wf = whatfor;
    int len = strlen(wf);
	int pargs = 0;
	int i;
    for(i = 0; i < len; ++i)
    {
		switch(wf[i])
		{
		case 'r':
			pargs |= VC_SOCK_READ;
			break;
		case 'w':
			pargs |= VC_SOCK_WRITE;
			break;
		case 'e':
			pargs |= VC_SOCK_ERROR;
			break;
		case '!':
			pargs |= VC_SOCK_READ_NOT_WRITE;
			break;
		case '-':
			pargs |= VC_SOCK_WRITE_NOT_READ;
			break;
		default:
			USER_BOMB("bogus polling arg, must be from [rwe!-]", vcnil);
		}
    }
		
	Socketvec results;
	int n = vc_winsock::poll_sets(pargs, results, sec, usec);
	CHECK_ANY_BO(vcnil);
	if(n < 0)
		return vcnil;
	vc retvec(VC_VECTOR, "", n);
	for(i = 0; i < n; ++i)
	{
		vc sock;
		vc ret(VC_VECTOR);
		sock.attach(results[i]);
		ret[0] = sock;
		vc ret2(VC_VECTOR);
		int ready = results[i]->status;
		if(ready & VC_SOCK_ERROR)
			ret2.append(vc("e"));
		if(ready & VC_SOCK_READ)
			ret2.append(vc("r"));
		if(ready & VC_SOCK_WRITE)
			ret2.append(vc("w"));
		ret[1] = ret2;
		retvec[i] = ret;
	}
	return retvec;
}
	
vc
lh_socksend(VCArglist *a)
{
	if(a->num_elems() < 2)
	{
		USER_BOMB("socket send must have at least two args", vcnil);
	}
    vc sock = (*a)[0];
    vc item = (*a)[1];
	vc v;
    if(a->num_elems() >= 3)
		v = sock.socket_send(item, vcnil, (*a)[2]);
	else
		v = sock.socket_send(item, vcnil);

	CHECK_ANY_BO(vcnil);
	return v;
}

vc
lh_sockrecv(VCArglist *a)
{
	vc recvd_item;
	vc addr_info;

	if(a->num_elems() < 1)
	{
		USER_BOMB("socket recv must have at least one arg", vcnil);
	}
    vc sock = (*a)[0];

	vc v = sock.socket_recv(recvd_item, vcnil, addr_info);
	CHECK_ANY_BO(vcnil);

	if(v.is_nil())
		return vcnil;

	if(a->num_elems() > 2)
		(*a)[2].local_bind(addr_info);
	if(a->num_elems() > 1)
		(*a)[1].local_bind(recvd_item);

	return vctrue;
}

// these are used for sending raw string data: really only
// intended for talking with non-LH applications
//
vc
lh_socksendstring(VCArglist *a)
{
	if(a->num_elems() < 2)
	{
		USER_BOMB("socket send must have at least two args", vcnil);
	}
    vc sock = (*a)[0];
    vc item = (*a)[1];
	if(item.type() != VC_STRING)
	{
		USER_BOMB("socket send string must have string arg", vcnil);
	}
	long len = item.len();
	const char *data = (const char *)item;
	vc v;
    if(a->num_elems() >= 3)
		v = sock.socket_send_raw((void *)data, len, 0, (*a)[2]);
	else
		v = sock.socket_send_raw((void *)data, len, 0);
	CHECK_ANY_BO(vcnil);

	return v;
}

vc
lh_sockrecvstring(VCArglist *a)
{
	vc addr_info;

	if(a->num_elems() < 2)
	{
        USER_BOMB("socket recv must have at least two args", vcnil);
	}
    vc sock = (*a)[0];
	long len = (*a)[1];
#if 0
	if(len < 0)
	{
		USER_BOMB("len must be >= 0 for socket recv string", vcnil);
	}
#endif
	char *buf;
	if(len >= 0)
		buf = new char[len + 1];
	else
		buf = new char[-len + 1];

	vc v = sock.socket_recv_raw(buf, len, 0, addr_info);
	CHECK_ANY_BO_CLEANUP(delete [] buf;, vcnil);

	if(v.is_nil())
	{
		delete [] buf;
		return vcnil;
	}
	//buf[(long)v] = '\0';
	vc recvd_item(VC_BSTRING, buf, (long)v);
	delete [] buf;

	if(a->num_elems() > 3)
		(*a)[3].local_bind(addr_info);
	if(a->num_elems() > 2)
		(*a)[2].local_bind(recvd_item);

	return vctrue;
}

vc
lh_sockset_option(VCArglist *al)
{
	vc sock = (*al)[0];
	vc option = (*al)[1];
    unsigned long arg = 0;
    if(al->num_elems() > 2)
        arg = (unsigned long)(long)(*al)[2];
	int bomb = 0;
	vcsocketmode so = VC_BLOCKING;
	if(option.type() != VC_STRING)
	{
		bomb = 1;
	}
	static vc b("blocking");
	static vc nb("nonblocking");
	static vc cloexec("close-on-exec");
	static vc nocloexec("no-close-on-exec");
	static vc get_recv("get-recv-buf-size");
	static vc get_send("get-send-buf-size");
	static vc set_recv("set-recv-buf-size");
	static vc set_send("set-send-buf-size");
	static vc set_tcp_no_delay("set-tcp-no-delay");
	static vc set_buffering("buffering");
	if(!bomb)
	{
		if(option == nb)
			so = VC_NONBLOCKING;
		else if(option == b)
			so = VC_BLOCKING;
		else if(option == cloexec)
			so = VC_CLOSE_ON_EXEC;
        else if(option == nocloexec)
			so = VC_NO_CLOSE_ON_EXEC;
		else if(option == get_recv)
			so = VC_GET_RECV_BUF_SIZE;
		else if(option == get_send)
			so = VC_GET_SEND_BUF_SIZE;
		else if(option == set_recv)
			so = VC_SET_RECV_BUF_SIZE;
		else if(option == set_send)
			so = VC_SET_SEND_BUF_SIZE;
		else if(option == set_tcp_no_delay)
			so = VC_SET_SEND_BUF_SIZE;
		else if(option == set_buffering)
			so = VC_BUFFER_SOCK;
		else
			bomb = 1;
	}
	if(bomb)
	{
		USER_BOMB("socket option is either \"blocking\", \"nonblocking\", \"close-on-exec\", \"no-close-on-exec\" or \"buffering\"", vcnil);
	}
	vc ret = sock.socket_set_option(so, arg);
	CHECK_ANY_BO(vcnil);
	return ret;
}

vc
lh_gethostbyname(vc hostname)
{
	struct hostent *h;

	if(hostname.type() != VC_STRING)
		USER_BOMB("first arg to gethostbyname must be string", vcnil);
	h = gethostbyname((const char *)hostname);
	if(h == 0)
		return vcnil;
	struct in_addr in;
	memcpy((char *)&in.s_addr, h->h_addr, h->h_length);
	char *addr = inet_ntoa(in);
	if(addr == 0)
		return vcnil;
	vc dot(addr);
	return dot;
}

vc 
lh_sock_peer_addr(vc sock)
{
	return sock.socket_peer_addr();
}

vc
lh_sock_local_addr(vc sock)
{
	return sock.socket_local_addr();
}

vc
lh_add_write_set(vc s)
{
	s.socket_add_write_set();
	return vctrue;
}

vc
lh_add_read_set(vc s)
{
	s.socket_add_read_set();
	return vctrue;
}

vc
lh_del_write_set(vc s)
{
	s.socket_del_write_set();
	return vctrue;
}

vc
lh_del_read_set(vc s)
{
	s.socket_del_read_set();
	return vctrue;
}

vc
lh_clear_write_set()
{
	vc_winsock::clear_write_set();
	return vctrue;
}

vc
lh_clear_read_set()
{
	vc_winsock::clear_read_set();
	return vctrue;
}

vc
lh_socksend_buf(VCArglist *a)
{
	if(a->num_elems() < 2)
	{
		USER_BOMB("socket_send_buf must have at least two args", vcnil);
	}
    vc sock = (*a)[0];
    vc item = (*a)[1];
	vc v;
    if(a->num_elems() >= 3)
		v = sock.socket_send_buf(item, vcnil, (*a)[2]);
	else
		v = sock.socket_send_buf(item, vcnil);

	CHECK_ANY_BO(vcnil);
	return v;
}

vc
lh_sockrecv_buf(VCArglist *a)
{
	vc recvd_item;
	vc addr_info;

	if(a->num_elems() < 1)
	{
		USER_BOMB("usage: socket_recv_buf sock len [item] [addr-info] ", vcnil);
	}
    vc sock = (*a)[0];

	vc v = sock.socket_recv_buf(recvd_item, (*a)[1], vcnil, addr_info);
	CHECK_ANY_BO(vcnil);

	if(v.is_nil())
		return vcnil;

	if(a->num_elems() > 3)
		(*a)[3].local_bind(addr_info);
	if(a->num_elems() > 2)
		(*a)[2].local_bind(recvd_item);

	return vctrue;
}
