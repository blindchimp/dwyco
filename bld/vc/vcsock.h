
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCSOCK_H
#define VCSOCK_H
// $Header: g:/dwight/repo/vc/rcs/vcsock.h 1.48 1997/09/19 20:59:01 dwight Stable $

#include "vccomp.h"
#include "vcio.h"


#define RAISE(fun, retval) do {excretval = (retval); NONLH_CHECK_ANY_BO(retval) if((fun)() == VC_SOCKET_BACKOUT) return retval;} while(0)
#define RAISEABORT(fun, retval) do {last_wsa_error = WSAGetLastError(); excretval = (retval); NONLH_CHECK_ANY_BO(retval) if((fun)() == VC_SOCKET_RESUME) oopanic("resume after abort?"); return retval;} while (0)

#define VC_SOCKET_RESUME 0
#define VC_SOCKET_BACKOUT 1

class vc_socket : public vc_composite
{
private:

protected:

	virtual int warn_not_closed() = 0;
	virtual int bad_socket_create() = 0;
	virtual int bad_addr_format() = 0;
	virtual int generic_problem() = 0;
	virtual int bad_bind() = 0;
	virtual int bad_listen() = 0;
	virtual int bad_accept() = 0;
	virtual int bad_connect() = 0;
	virtual int bad_select() = 0;
	virtual int send_no_bufs() = 0;
	virtual int wouldblock() = 0;
	virtual int bad_send_raw_soft() = 0;
	virtual int bad_send_raw() = 0;
	virtual int bad_recv_raw_soft() = 0;
	virtual int bad_recv_raw() = 0;
	virtual int recv_eof() = 0;
	virtual int bad_dgram_recv() = 0;

	virtual int do_error_processing();

	// too many things reset the return of WSAGetLastError
	// this just caches it so we have access to something
	// stable while the error stuff is running
	int last_wsa_error;

public:
	vc_socket(); 

	int status;	// for read/write/error ready checking (valid only after poll)

	// error reporting items
	char exc_level; // A, W, E
	vc errvc;
	vcerrormode emode;
	vcsocketmode smode;
	int retries;
	int max_retries;
	// this is set when a raise is done with the retval
	// it is used in cases where a valid return is needed for
	// situations like sending partial buffers (we need the number of
	// bytes sent) and then getting a would-block exception.
	// it is a kluge. it needs to be fixed globally, the
	// exception stuff at this level is a real pain in the butt.
	// in fact, it might be better if we avoided "wouldblock" altogether,
	// and just set up some returns for "partial operation completed".
	vc excretval;

	virtual vc_default *do_copy() const {oopanic("copy socket?"); return 0;}
	enum vc_type type() const {return VC_SOCKET;}
    int is_quoted() const {return 1;}
	void printOn(VcIO os) {os << "socket(errvc="; errvc.printOn(os); os << ")";}
	void stringrep(VcIO os) const {os << "socket()";}
	virtual vcsocketmode socket_get_mode() { return smode; }


};
#endif
