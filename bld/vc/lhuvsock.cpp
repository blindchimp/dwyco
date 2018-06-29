
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifndef DWYCO_NO_UVSOCK

#include "lhuvsock.h"
#include "dwvec.h"
#include "vcmap.h"
#include "vcuvsock.h"
#ifdef LINUX
#include <netdb.h>
#endif

// note: LH callback functions for I/O should be declared like so:
// compile(foo_callback uvsock obj `...')
// the callback is called once for each object that is q'd
// the obj has the form
// vector(command t/nil data more-data)
// where command can be:
// "data" - the data arg contains the object received
// "connect" - "t" the socket is ready to accept put-objs, nil = failure
// "accept" - the socket is ready to generate data
// (note: you get a "connect" and an "accept" command on both
// sides of a socket, since it is a full duplex link by default.
// it will not contain peer or extra socket info.
// the socket just "appears". if you need to know when a socket
// appears, watch for the "listen-accept" message which comes from
// the listening socket.
// "listen-accept" - the "accept" from the listening socket contains the new socket.
// you can ignore this message unless there is an error, in which case
// it is unclear whether you may need to shutdown the listener and 
// restart it. probably a good idea.
// "eof" - graceful close on reading
// "shutdown" - graceful/failure close on writing
// "error" - failure while reading
// "init" - failure during initialization
// "write" - failure during write
// 
// ---
// the second argument indicates success or failure.
// usually one failure is enough to stop all further messages
// from that socket
// ---
// the 3rd and 4th args depend on the command (see above)
//
// note: a simpler interface below in the getobjs api just returns
// a list of all the currently available (uvsock, cmd-vec) pairs.
// maybe sometime if the callback this needed, i'll fill that in.

vc
lh_uv_socket(vc protocol, vc local_addr, vc is_listen, vc reuse_addr)
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
		sock = vc(VC_UVSOCKET_STREAM);
	else if(protocol == udp)
		sock = vc(VC_UVSOCKET_DGRAM);
#if 0
	else if(protocol == unixx) // not quite right, but ok for now
		sock = vc(VC_SOCKET_STREAM_UNIX);
#endif
	sock.socket_init(local_addr, is_listen.is_nil() ? 0 : 1,
		 reuse_addr.is_nil() ? 0 : 1);
	CHECK_ANY_BO(vcnil);
	return sock;
}

vc
lh_uv_sockclose(vc sock, vc how)
{
	vc v = sock.socket_close(how.is_nil() ? 0 : 1);
	CHECK_ANY_BO(vcnil);
	return v;
}

vc
lh_uv_sockshutdown(vc sock, vc how)
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
lh_uv_connect(vc sock, vc remote_addr)
{
	vc v = sock.socket_connect(remote_addr);
	CHECK_ANY_BO(vcnil);
	return v;
}

vc
lh_uv_get_all()
{
	vc ret(VC_VECTOR);

	UV_READY_Q_ITER i(vc_uvsocket::Ready_q_p);
	// note: don't perform any ops on the sockets that might
	// cause the ready_q to change while we are iterating over it.
	for(;!i.eol(); i.forward())
	{
		DwAssocImp<vc_uvsocket *, long> val = i.get();
		vc sockret;
		vc_uvsocket *vcu = val.get_value();
        sockret.attach(vcu);
		vc listret(VC_VECTOR);
		vc elem;
		dwlista_foreach(elem, vcu->getq)
		{
			listret.append(elem);
		}
		vc sret(VC_VECTOR);
		sret[0] = sockret;
		sret[1] = listret;
		ret.append(sret);
		// inhibit ready_q updating in this case
		((DwListA<vc> *)&vcu->getq)->clear();
	}
	// ok, we've gotten all the data out of all the sockets,
	// clear everything out.
	vc_uvsocket::Ready_q_p->clear();
	return ret;
}

vc
lh_uv_poll()
{
	return vc_uvsocket::run_loop_once();
}

vc
lh_uv_socksend(VCArglist *a)
{
	if(a->num_elems() < 2)
	{
		USER_BOMB("socket send must have at least two args", vcnil);
	}
    vc sock = (*a)[0];
    vc item = (*a)[1];
	vc v;
	if(a->num_elems() == 3)
		v = sock.socket_put_obj(item, (*a)[2], 0);
	else
		v = sock.socket_put_obj(item, vcnil, 0);

	CHECK_ANY_BO(vcnil);
	return v;
}

vc
lh_uv_sockrecv(VCArglist *a)
{
	vc addr_info;

	if(a->num_elems() < 1)
	{
		USER_BOMB("socket recv must have at least one arg", vcnil);
	}
    vc sock = (*a)[0];

	int avail;
	vc recvd_item = sock.socket_get_obj(avail, addr_info);
	// i suppose if you want to emulate the old stuff, we would check
	// for errors and raise exceptions in those cases. no overly
	// interested in that level of emulation tho
	CHECK_ANY_BO(vcnil);

	if(!avail)
		return vcnil;
	

	if(a->num_elems() > 2)
		(*a)[2].local_bind(addr_info);
	if(a->num_elems() > 1)
		(*a)[1].local_bind(recvd_item);

	return vctrue;
}

#endif
