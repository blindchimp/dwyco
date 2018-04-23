
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCWSOCK_H
#define VCWSOCK_H
// $Header: /e/linux/home/dwight/repo/vc/rcs/vcwsock.h 1.56 1999/03/17 14:57:04 dwight Exp $

// Winsock implementation of vc sockets
#ifdef USE_WINSOCK
#include <winsock.h>
#endif

#ifdef USE_BERKSOCK
#include "vcberk.h"
#endif

#include "vcsock.h"
class vc_winsock;

unsigned long hash(vc_winsock *a);
#include "dwset.h"
#include "vcxstrm.h"


template<class T> class DwVecP;

typedef DwVecP<vc_winsock> Socketvec;
typedef DwSet<vc_winsock *> SocketSet;
typedef DwSetIter<vc_winsock *> SocketSetIter;

class vc_winsock_stream;
class vc_winsock : public vc_socket
{
private:
		
	//static VcWinsockDispatcher *dispatcher;
	//static Socketvec *Poll_results;
	static SocketSet *Read_set;
	static SocketSet *Write_set;
	// this is a hack because of a bug in bc45,
	// doesn't do the THREADLOCAL lookup right
	// if the static is referenced directly.
	//static Socketvec *get_poll_results();
        static int poll_impl(int whatfor, int sec, int usec, Socketvec& res);
        static int poll2_impl(int whatfor, int sec, int usec, Socketvec& res);

	vcxstream vcxr;
	vcxstream vcxs;

friend vc lh_horrible_hack(vc);

protected:
	vc_winsock(const vc_winsock&);
	SOCKET sock;
	vc peer_addr;
	vc local_addr;
	int used_connect;
	int is_listening;
	// 1 means the socket is using read-ahead and write buffering.
	// 0 means the socket is only writing/reading exactly the number of
	// bytes needed. this is useful in cases where the serialization
	// syntax changes mid-stream.
	int buffered; 
	vc error;
	int async_error;
	virtual int vc_to_sockaddr(const vc& v, struct sockaddr *&, int& len);
	virtual vc sockaddr_to_vc(struct sockaddr *, int len);
	virtual struct sockaddr *get_sockaddr();
	virtual int get_sockaddr_len();
	virtual struct sockaddr *make_sockaddr(const vc& local_addr, int& len_out);

	int init();

	virtual SOCKET get_socket() = 0;
	virtual vc_winsock_stream * wrap_sock(SOCKET s, const vc_winsock& ws) = 0;
	virtual void out_flavor(VcIO os) const = 0;
	virtual int warn_not_closed() ;
	virtual int bad_socket_create() ;
	virtual int bad_addr_format() ;
	virtual int generic_problem() ;
	virtual int bad_bind() ;
	virtual int bad_listen() ;
	virtual int bad_accept() ;
	virtual int bad_connect() ;
	virtual int bad_select() ;
	virtual int send_no_bufs() ;
	virtual int wouldblock() ;
	virtual int bad_send_raw_soft() ;
	virtual int bad_send_raw() ;
	virtual int bad_recv_raw_soft() ;
	virtual int bad_recv_raw() ;
	virtual int recv_eof() ;
	virtual int bad_dgram_recv();

	virtual int do_error_processing();
	virtual int do_fatal_error_processing();
	virtual int do_warn_error_processing();
	virtual int do_resumable_error_processing();
	int special_fatal_handling();

public:
	vc_winsock();
	~vc_winsock();
#ifdef USE_WINSOCK
	static WSADATA wsa_data;
#endif
	static int have_net;

	static int poll_all(int what_for, Socketvec&, int sec = 0, int usec = 0);
	static int poll_sets(int what_for, Socketvec&, int sec = 0, int usec = 0);
	static void close_all_but(SOCKET);
        static void set_async_error(SOCKET s, int err);

	static int startup();
	static int thread_startup();
	static int shutoff();
	static int thread_shutoff();
	static SocketSet *get_read_set();
	static SocketSet *get_write_set();
	static void clear_read_set();
	static void clear_write_set();

	virtual void socket_add_read_set();
	virtual void socket_add_write_set();
	virtual void socket_del_read_set();
	virtual void socket_del_write_set();

	virtual vc socket_init(const vc& local_addr, int listen, int reuse, int syntax = 0) ;
	virtual vc socket_init(SOCKET os_handle, vc listening , int syntax = 0) ;
	virtual vc socket_accept(vc& addr_info) ;
	virtual vc socket_connect(const vc& addr_info) ;
	virtual vc socket_close(int close_info) ;
	virtual vc socket_shutdown(int how);
	virtual int socket_poll(int what_for, long to_sec, long to_usec) ;
	virtual vc socket_send(vc item, const vc& send_info, const vc& to = vcnil) ;
	virtual vc socket_recv(vc& item, const vc& recv_info, vc& addr_info = vcbitbucket) ;
	virtual vc socket_send_raw(void *buf, long& len, long timeout, const vc& to = vcnil) ;
	virtual vc socket_recv_raw(void *buf, long& len, long timeout, vc& addr_info = vcbitbucket) ;
	virtual vc socket_error();
	virtual vc socket_error_desc();
	virtual void socket_set_error(vc v);
	virtual vc socket_set_option(vcsocketmode m, unsigned long = 0,
		unsigned long = 0, unsigned long = 0);
	virtual vc socket_peer_addr();
	virtual vc socket_local_addr();
	virtual vc socket_recv_buf(vc& item, int to_get, const vc& recv_info, vc& addr_info);
	virtual vc socket_send_buf(vc item, const vc& send_info, const vc& to = vcnil);

	virtual long overflow(vcxstream&, char *, long);
	virtual long underflow(vcxstream&, char *, long, long);
	virtual void stringrep(VcIO o) const ;
};

class vc_winsock_stream : public vc_winsock
{
public:
	vc_winsock_stream(SOCKET, const vc_winsock&);
	vc_winsock_stream();

	virtual vc_winsock_stream * wrap_sock(SOCKET s, const vc_winsock& ws);
	virtual SOCKET get_socket();
	virtual void out_flavor(VcIO os) const;
	void printOn(VcIO os);

};

class vc_winsock_datagram : public vc_winsock
{
private:
	static char *iobuf;
	static unsigned int  iobuflen;

	virtual vc_winsock_stream * wrap_sock(SOCKET s, const vc_winsock& ws) {return 0;}

public:
	vc_winsock_datagram();

	virtual SOCKET get_socket();
	virtual void out_flavor(VcIO os) const;
	
	virtual vc socket_init(const vc& local_addr, int listen, int reuse, int syntax = 0) ;
	virtual vc socket_init(SOCKET os_handle, vc listening, int syntax = 0) ;
	virtual vc socket_accept(vc& addr_info) ;
	virtual vc socket_send_raw(void *buf, long& len, long timeout, const vc& to = vcnil) ;
	virtual vc socket_recv_raw(void *buf, long& len, long timeout, vc& addr_info = vcbitbucket) ;

	virtual vc socket_send(vc, const vc&, const vc& to = vcnil);
	virtual vc socket_recv(vc&, const vc&, vc& from = vcbitbucket);
	virtual long overflow(vcxstream&, char *buf, long len);
	virtual long underflow(vcxstream&, char *buf, long min, long max);
	
	void printOn(VcIO os);

};

class vc_winsock_unix : public vc_winsock_stream
{
public:
	vc_winsock_unix(SOCKET, const vc_winsock&);
	vc_winsock_unix();

	virtual SOCKET get_socket();
	virtual int vc_to_sockaddr(const vc& v, struct sockaddr *&, int& len);
	virtual vc sockaddr_to_vc(struct sockaddr *, int len);
	virtual struct sockaddr *get_sockaddr();
	virtual int get_sockaddr_len();
	virtual struct sockaddr *make_sockaddr(const vc& local_addr, int& len_out);
	virtual vc_winsock_stream *wrap_sock(SOCKET s, const vc_winsock& ws);
};

#endif
