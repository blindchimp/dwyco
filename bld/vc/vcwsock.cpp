
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "dwstr.h"
#include <string.h>

#ifdef USE_WINSOCK
#include <WinSock2.h>
#include <WS2tcpip.h>
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
//#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
//#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE
#endif

struct scoped_sockaddr
{
    struct sockaddr *value;
    explicit scoped_sockaddr(struct sockaddr *&v) {value = v; v = nullptr;}
    ~scoped_sockaddr() {if(value) free(value);}
    operator struct sockaddr *() {return value;}
};


#include "dwvecp.h"
#include "vcwsock.h"
#include "vcxstrm.h"
#include "vcmap.h"
//static char Rcsid[] = "$Header: /e/linux/home/dwight/repo/vc/rcs/vcwsock.cpp 1.69 1999/03/17 14:57:04 dwight Exp $";

int vc_winsock::have_net;

//VcWinsockDispatcher *vc_winsock::dispatcher;
//Socketvec *vc_winsock::Poll_results;
SocketSet *vc_winsock::Read_set;
SocketSet *vc_winsock::Write_set;


unsigned long hash(vc_winsock *a)
{
	return (unsigned long)a * 0x15a4e35L;
}

#ifdef LINUX
#define USE_POLL
#include <sys/poll.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <signal.h>
#ifdef MACOSX
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/param.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <limits.h>
#endif
#endif
#include <cstddef>

#ifndef USE_WINSOCK
#include "vcberk.h"

static int
WSAGetLastError()
{
	return errno;
}

static int
WSASetLastError(int e)
{
	errno = e;
	return 0;
}

#endif

static SocketSet *All_socks;

#ifndef USE_POLL
int
vc_winsock::poll_impl(int whatfor, int sec, int usec, Socketvec& res)
{
	fd_set rset;
	fd_set wset;
	fd_set eset;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);

	int dont_block = 0;
    SocketSetIter i(All_socks);
	for(; !i.eol(); i.forward())
	{
        vc_winsock *vcsockp = (vc_winsock *)i.get();
        vcsockp->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
        SOCKET s = vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			if(whatfor & VC_SOCK_READ)
			{
#if 0
// see comment below in poll2
                if(vcxr.has_input())
				{
					dont_block = 1;
				}
				else
#endif
					FD_SET(s, &rset);
			}

			if(whatfor & VC_SOCK_WRITE)
				FD_SET(s, &wset);

			if(whatfor & VC_SOCK_ERROR)
				FD_SET(s, &eset);
		}
	}
	if(dont_block)
	{
		sec = 0;
		usec = 0;
	}
	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = usec;

	long num = select(FD_SETSIZE, &rset, &wset, &eset, &tv);

	if(num == SOCKET_ERROR)
	{
		return -1;
	}

	// some winsocks (core-internet-connect) return
	// inconsistent results if num == 0 (the
	// sock sets are not cleared...)
	if(num == 0)
	{
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_ZERO(&eset);
	}
		
	for(i.rewind(); !i.eol(); i.forward())
	{
        vc_winsock *vcsockp = (vc_winsock *)i.get();
        SOCKET s = vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			int did_append = 0;
			if((whatfor & VC_SOCK_READ) && (FD_ISSET(s, &rset) || 
                    vcsockp->vcxr.has_input()))
			{
                vcsockp->status |= VC_SOCK_READ;
				if(!did_append)
				{
                    res.append(vcsockp);
					did_append = 1;
				}
			}

			if((whatfor & VC_SOCK_WRITE) && FD_ISSET(s, &wset))
			{
                vcsockp->status |= VC_SOCK_WRITE;
				if(!did_append)
				{
                    res.append(vcsockp);
					did_append = 1;
				}
			}

			if((whatfor & VC_SOCK_ERROR) && FD_ISSET(s, &eset))
			{
                vcsockp->status |= VC_SOCK_ERROR;
				if(!did_append)
				{
                    res.append(vcsockp);
					did_append = 1;
				}
			}
		}
	}
    return res.num_elems();
}

//
// whatfor now means:
// whatfor & VC_SOCK_READ = poll all sockets for read
// whatfor & VC_SOCK_WRITE = poll all for write
// whatfor & VC_SOCK_ERR = poll all for error
// otherwise, use the read/write sets to figure out what to
// poll for.
int
vc_winsock::poll2_impl(int whatfor, int sec, int usec, Socketvec& res)
{
	fd_set rset;
	fd_set wset;
	fd_set eset;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);

	int dont_block = 0;
    SocketSetIter dhi(All_socks);
	if(whatfor != 0)
	{
		for(; !dhi.eol(); dhi.forward())
		{
            vc_winsock *vcsockp = (vc_winsock *)dhi.get();
            vcsockp->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
            SOCKET s = vcsockp->sock;
			if(s != INVALID_SOCKET)
			{
				if(whatfor & VC_SOCK_READ)
				{
#if 0
// see comment below in poll2
                    if(vcxr.has_input())
					{
						dont_block = 1;
					}
					else
#endif
						FD_SET(s, &rset);
				}

				if(whatfor & VC_SOCK_WRITE)
					FD_SET(s, &wset);

				if(whatfor & VC_SOCK_ERROR)
					FD_SET(s, &eset);
			}
		}
	}
	if(!(whatfor & VC_SOCK_READ))
	{
		// load up the read set
		SocketSet *ss = vc_winsock::get_read_set();
		SocketSetIter ssi(ss);
		for(; !ssi.eol(); ssi.forward())
		{
			vc_winsock *vcw = ssi.get();
			SOCKET s = vcw->sock;
			if(s != INVALID_SOCKET)
			{
				vcw->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
                if(vcw->vcxr.has_input())
				{
					dont_block = 1;
				}
				else
					FD_SET(s, &rset);
			}
		}
	}
	if(!(whatfor & VC_SOCK_WRITE))
	{
		// load up the write set
		SocketSet *ss = vc_winsock::get_write_set();
		SocketSetIter ssi(ss);
		for(; !ssi.eol(); ssi.forward())
		{
			vc_winsock *vcw = ssi.get();
			SOCKET s = vcw->sock;
			if(s != INVALID_SOCKET)
			{
				vcw->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
				FD_SET(s, &wset);
			}
		}
	}

	if(dont_block)
	{
		sec = 0;
		usec = 0;
	}

	struct timeval tv;
	tv.tv_sec = sec;
	tv.tv_usec = usec;

	long num = select(FD_SETSIZE, &rset, &wset, &eset, &tv);

	if(num == SOCKET_ERROR)
	{
		return -1;
	}

	// some winsocks (core-internet-connect) return
	// inconsistent results if num == 0 (the
	// sock sets are not cleared...)
	if(num == 0)
	{
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_ZERO(&eset);
	}
		
	for(dhi.rewind(); !dhi.eol(); dhi.forward())
	{
        vc_winsock *vcsockp = (vc_winsock *)dhi.get();
        SOCKET s = vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			int did_append = 0;
            if(FD_ISSET(s, &rset) || vcsockp->vcxr.has_input())
			{
                vcsockp->status |= VC_SOCK_READ;
				if(!did_append)
				{
                    res.append(vcsockp);
					did_append = 1;
				}
			}

			if(FD_ISSET(s, &wset))
			{
                vcsockp->status |= VC_SOCK_WRITE;
				if(!did_append)
				{
                    res.append(vcsockp);
					did_append = 1;
				}
			}

			if(FD_ISSET(s, &eset))
			{
                vcsockp->status |= VC_SOCK_ERROR;
				if(!did_append)
				{
                    res.append(vcsockp);
					did_append = 1;
				}
			}
		}
	}

    return res.num_elems();
}
#else
#include <sys/poll.h>

int
vc_winsock::poll_impl(int whatfor, int sec, int usec, Socketvec& res)
{
	int dont_block = 0;
	int nfds = All_socks->num_elems();
	//struct pollfd *pfds = new struct pollfd[nfds];
    // hack, if nfds is 0, we are here just to sleep most likely
    // but we still want index checking and poll wants a real pointer
    DwVec<struct pollfd> pfds(nfds == 0 ? 1 : nfds, DWVEC_FIXED);
	int pfd_count = 0;
	
	SocketSetIter i(All_socks);
	for(; !i.eol(); i.forward())
	{
		vc_winsock *vcsockp = (vc_winsock *)i.get();
		vcsockp->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
		SOCKET s = vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			pfds[pfd_count].fd = s;
			pfds[pfd_count].events = 0;
			if(whatfor & VC_SOCK_READ)
			{
#if 0
//see comment below in poll2
				if(vcxr.has_input())
				{
					dont_block = 1;
				}
				else
#endif
				{
					pfds[pfd_count].events |= POLLIN;
				}
			}

			if(whatfor & VC_SOCK_WRITE)
				pfds[pfd_count].events |= POLLOUT;

			if(whatfor & VC_SOCK_ERROR)
				pfds[pfd_count].events |= POLLPRI;

			++pfd_count;
		}
	}
	if(dont_block)
	{
		sec = 0;
		usec = 0;
	}

	int num = ::poll(&pfds[0], pfd_count, sec * 1000 + usec / 1000);

	if(num == SOCKET_ERROR)
	{
		return -1;
	}

	pfd_count = 0;
	for(i.rewind(); !i.eol(); i.forward())
	{
		vc_winsock *vcsockp = (vc_winsock *)i.get();
		SOCKET s = vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			int did_append = 0;
			if((whatfor & VC_SOCK_READ) && ((pfds[pfd_count].revents & (POLLIN|POLLERR|POLLHUP)) || 
                    vcsockp->vcxr.has_input()))
			{
				vcsockp->status |= VC_SOCK_READ;
				if(!did_append)
				{
					res.append(vcsockp);
					did_append = 1;
				}
			}

			if((whatfor & VC_SOCK_WRITE) && (pfds[pfd_count].revents & (POLLOUT|POLLERR|POLLHUP)))
			{
				vcsockp->status |= VC_SOCK_WRITE;
				if(!did_append)
				{
					res.append(vcsockp);
					did_append = 1;
				}
			}

			if((whatfor & VC_SOCK_ERROR) && (pfds[pfd_count].revents & (POLLPRI|POLLERR|POLLHUP)))
			{
				vcsockp->status |= VC_SOCK_ERROR;
				if(!did_append)
				{
					res.append(vcsockp);
					did_append = 1;
				}
			}
			++pfd_count;
		}
	}
	return res.num_elems();
}

//
// whatfor now means:
// whatfor & VC_SOCK_READ = poll all sockets for read
// whatfor & VC_SOCK_WRITE = poll all for write
// whatfor & VC_SOCK_ERR = poll all for error
// otherwise, use the read/write sets to figure out what to
// poll for.
// new: 
// whatfor & VC_SOCK_READ_NOT_WRITE = poll all sockets for read
// except for the ones in the write set.
// whatfor & VC_SOCK_WRITE_NOT_READ = poll all sockets for
// write except for the ones in the read set.
//
int
vc_winsock::poll2_impl(int whatfor, int sec, int usec, Socketvec& res)
{
    int nfds = All_socks->num_elems();
	// note: can't really tell ahead, so just allocate big
    DwVec<struct pollfd> pfds(nfds == 0 ? 1 : nfds * 3);
	//struct pollfd *pfds = new struct pollfd[nfds * 3];
	int pfd_count = 0;

	int dont_block = 0;
    SocketSetIter dhi(All_socks);
	if(whatfor != 0)
	{
		for(; !dhi.eol(); dhi.forward())
		{
			vc_winsock *vcsockp = (vc_winsock *)dhi.get();
			vcsockp->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
			SOCKET s = vcsockp->sock;
			if(s != INVALID_SOCKET)
			{
				pfds[pfd_count].fd = s;
				pfds[pfd_count].events = 0;
				if(whatfor & VC_SOCK_READ)
				{
#if 0
// note: this if 0 is a compromise. if we don't block
// and there is input pending in the stream
// buffer for a socket, then we end up spinning
// in cases where only a partially formed hi-level
// structure exists in the buffer. if we block,
// then there is a potential situation where
// there might be a full structure that was
// in the buffer because of a read-ahead, but
// the caller just won't get it right away.
// the previous cases happens more commonly,
// and results in a performance problem, so
// we take the other route. this whole thing
// sucks and needs to be redone 
					if(vcxr.has_input())
					{
						dont_block = 1;
					}
					else
#endif
					{
						pfds[pfd_count].events |= POLLIN;
					}
				}

				if(whatfor & VC_SOCK_WRITE)
					pfds[pfd_count].events |= POLLOUT;

				if(whatfor & VC_SOCK_ERROR)
					pfds[pfd_count].events |= POLLPRI;

				if(whatfor & VC_SOCK_READ_NOT_WRITE)
				{
					SocketSet *ss = vc_winsock::get_write_set();
					// note: if write set contains this socket,
					// we don't increment pfd_count, so the socket
					// is effectively deleted from the poll set.
					if(!ss->contains(vcsockp))
						++pfd_count;
				}
				else if(whatfor & VC_SOCK_WRITE_NOT_READ)
				{
					SocketSet *ss = vc_winsock::get_read_set();
					// note: if write set contains this socket,
					// we don't increment pfd_count, so the socket
					// is effectively deleted from the poll set.
					if(!ss->contains(vcsockp))
						++pfd_count;
				}
				else
					++pfd_count;
			}
		}
	}
	if(!(whatfor & VC_SOCK_READ))
	{
		// load up the read set
		SocketSet *ss = vc_winsock::get_read_set();
		SocketSetIter ssi(ss);
		for(; !ssi.eol(); ssi.forward())
		{
			vc_winsock *vcw = ssi.get();
			SOCKET s = vcw->sock;
			if(s != INVALID_SOCKET)
			{
				pfds[pfd_count].fd = s;
				pfds[pfd_count].events = 0;
				vcw->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
				if(vcw->vcxr.has_input())
				{
					dont_block = 1;
				}
				else
				{
					pfds[pfd_count].events |= POLLIN;
				}
				++pfd_count;
			}
		}
	}
	if(!(whatfor & VC_SOCK_WRITE))
	{
		// load up the write set
		SocketSet *ss = vc_winsock::get_write_set();
		SocketSetIter ssi(ss);
		for(; !ssi.eol(); ssi.forward())
		{
			vc_winsock *vcw = ssi.get();
			SOCKET s = vcw->sock;
			if(s != INVALID_SOCKET)
			{
				pfds[pfd_count].fd = s;
				pfds[pfd_count].events = 0;
				vcw->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
				pfds[pfd_count].events |= POLLOUT;
				++pfd_count;
			}
		}
	}

	if(dont_block)
	{
		sec = 0;
		usec = 0;
	}

	int num = ::poll(&pfds[0], pfd_count, sec * 1000 + usec / 1000);

	if(num == SOCKET_ERROR)
	{
		return -1;
	}

	pfd_count = 0;
	if(whatfor != 0)
	{
		for(dhi.rewind(); !dhi.eol(); dhi.forward())
		{
			vc_winsock *vcsockp = (vc_winsock *)dhi.get();
			SOCKET s = vcsockp->sock;
			if(s != INVALID_SOCKET)
			{
				int did_append = 0;
				if((whatfor & VC_SOCK_READ) && ((pfds[pfd_count].revents & (POLLIN|POLLERR|POLLHUP)) || 
						vcsockp->vcxr.has_input()))
				{
					if(1 || !(whatfor & VC_SOCK_READ_NOT_WRITE) ||
						!vc_winsock::get_write_set()->contains(vcsockp))
					{
						vcsockp->status |= VC_SOCK_READ;
						if(!did_append)
						{
							res.append(vcsockp);
							did_append = 1;
						}
					}
				}

				if((whatfor & VC_SOCK_WRITE) && (pfds[pfd_count].revents & (POLLOUT|POLLERR|POLLHUP)))
				{
					if(1|| !(whatfor & VC_SOCK_WRITE_NOT_READ) ||
						!vc_winsock::get_read_set()->contains(vcsockp))
					{
						vcsockp->status |= VC_SOCK_WRITE;
						if(!did_append)
						{
							res.append(vcsockp);
							did_append = 1;
						}
					}
				}

				if((whatfor & VC_SOCK_ERROR) && (pfds[pfd_count].revents & (POLLPRI|POLLERR|POLLHUP)))
				{
					vcsockp->status |= VC_SOCK_ERROR;
					if(!did_append)
					{
						res.append(vcsockp);
						did_append = 1;
					}
				}
				// keep the pfd_count in lock step the same way it was
				// entered into the pfd list
				if(whatfor & VC_SOCK_READ_NOT_WRITE)
				{
					SocketSet *ss = vc_winsock::get_write_set();
					// note: if write set contains this socket,
					// we don't increment pfd_count, so the socket
					// is effectively deleted from the poll set.
					if(!ss->contains(vcsockp))
						++pfd_count;
				}
				else if(whatfor & VC_SOCK_WRITE_NOT_READ)
				{
					SocketSet *ss = vc_winsock::get_read_set();
					// note: if write set contains this socket,
					// we don't increment pfd_count, so the socket
					// is effectively deleted from the poll set.
					if(!ss->contains(vcsockp))
						++pfd_count;
				}
				else
					++pfd_count;
			}
		}
	}

	if(!(whatfor & VC_SOCK_READ))
	{
		SocketSet *ss = vc_winsock::get_read_set();
		SocketSetIter ssi(ss);
		for(; !ssi.eol(); ssi.forward())
		{
			vc_winsock *vcw = ssi.get();
			SOCKET s = vcw->sock;
			if(s != INVALID_SOCKET)
			{
				if(((pfds[pfd_count].revents & (POLLIN|POLLERR|POLLHUP)) || 
						vcw->vcxr.has_input()) && !(vcw->status & VC_SOCK_READ))
				{
					vcw->status |= VC_SOCK_READ;
					if(!(vcw->status & (VC_SOCK_WRITE|VC_SOCK_ERROR)))
                        res.append(vcw);
				}
				++pfd_count;
			}
		}
	}
	if(!(whatfor & VC_SOCK_WRITE))
	{
		// load up the write set
		SocketSet *ss = vc_winsock::get_write_set();
		SocketSetIter ssi(ss);
		for(; !ssi.eol(); ssi.forward())
		{
			vc_winsock *vcw = ssi.get();
			SOCKET s = vcw->sock;
			if(s != INVALID_SOCKET)
			{
				if((pfds[pfd_count].revents & (POLLOUT|POLLERR|POLLHUP)) &&
					!(vcw->status & VC_SOCK_WRITE))
				{
					vcw->status |= VC_SOCK_WRITE;
					if(!(vcw->status & (VC_SOCK_READ|VC_SOCK_ERROR)))
                        res.append(vcw);
				}
				++pfd_count;
			}
		}
	}
	return res.num_elems();
}
#endif


#ifdef __WIN32__
static HANDLE Mutex;
static int
get_mutex(HANDLE hMutex)  
{ 
    DWORD dwWaitResult; 

    // Request ownership of mutex.
 
    dwWaitResult = WaitForSingleObject( 
        hMutex,   // handle of mutex
        20000L);   // five-second time-out interval
 
    switch (dwWaitResult) 
    {
        // The thread got mutex ownership.
        case WAIT_OBJECT_0: 
            return 1;

        // Cannot get mutex ownership due to time-out.
        case WAIT_TIMEOUT: 
            return 0;

        // Got ownership of the abandoned mutex object.
        case WAIT_ABANDONED: 
            return 0;
    }

	return 1;
}
#endif

SocketSet *
vc_winsock::get_read_set()
{
	return Read_set;
}

SocketSet *
vc_winsock::get_write_set()
{
	return Write_set;
}

void
vc_winsock::socket_add_write_set()
{
	Write_set->add(this);
}

void
vc_winsock::socket_add_read_set()
{
	Read_set->add(this);
}

void
vc_winsock::socket_del_write_set()
{
	Write_set->del(this);
}

void
vc_winsock::socket_del_read_set()
{
	Read_set->del(this);
}

void
vc_winsock::clear_read_set()
{
	delete Read_set;
    Read_set = new SocketSet(0, 31);
}

void
vc_winsock::clear_write_set()
{
	delete Write_set;
    Write_set = new SocketSet(0, 31);
}

// initialize thread local variables
int
vc_winsock::thread_startup()
{
	//dispatcher = new VcWinsockDispatcher;
    Read_set = new SocketSet(0, 31);
    Write_set = new SocketSet(0, 31);
	All_socks = new SocketSet(0, 31);
	return 1;
}

// global startup, calls thread_startup for
// the global thread
int
vc_winsock::startup()
{
	if(have_net)
		return 1;
#ifdef __WIN32__
	Mutex = CreateMutex(NULL, FALSE, NULL);
	if(!get_mutex(Mutex))
		::abort();
#endif

	
#ifdef USE_WINSOCK
    WSADATA wsa_data;

    int err = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if(err != 0)
	{
		ReleaseMutex(Mutex);
		return 0;
	}
    if ( LOBYTE( wsa_data.wVersion ) != 2 ||
            HIBYTE( wsa_data.wVersion ) != 2 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup();
        ReleaseMutex(Mutex);
        return 0;
    }

#endif
	thread_startup();
	have_net = 1;
#ifdef __WIN32__
	ReleaseMutex(Mutex);
#endif

	return 1;
}

int
vc_winsock::thread_shutoff()
{
	//delete dispatcher;
	//dispatcher = 0;
	delete Read_set;
	Read_set = 0;
	delete Write_set;
	Write_set = 0;
	delete All_socks;
	All_socks = 0;
	return 1;
}

int
vc_winsock::shutoff()
{
#ifdef __WIN32__
	if(!get_mutex(Mutex))
		::abort();
#endif
	int hadnet = have_net;
#ifdef USE_WINSOCK
	if(have_net)
		WSACleanup();
#endif
	thread_shutoff();
	have_net = 0;
#ifdef __WIN32__
	ReleaseMutex(Mutex);
#endif
	return hadnet;
}

void
vc_winsock::close_all_but(SOCKET tosave)
{
#ifdef __WIN32__
	if(!get_mutex(Mutex))
		::abort();
#endif
	SocketSetIter i(All_socks);
	for(; !i.eol(); i.forward())
	{
		vc_winsock *vcsockp = (vc_winsock *)i.get();
		SOCKET s = vcsockp->sock;
		if(s != INVALID_SOCKET && s != tosave)
                {
#ifdef USE_WINSOCK
                        closesocket(s);
#else
                        ::close(s);
#endif
                }
	}
#ifdef __WIN32__
	ReleaseMutex(Mutex);
#endif
}

int
vc_winsock::poll_all(int whatfor, Socketvec& out, int sec, int usec)
{
#ifdef __WIN32__
	if(!get_mutex(Mutex))
		::abort();
#endif
	out.set_size(0);
	int num = poll_impl(whatfor, sec, usec, out);
	if(num < 0)
		num = -1;
#ifdef __WIN32__
	ReleaseMutex(Mutex);
#endif
	return num;
}

int
vc_winsock::poll_sets(int whatfor, Socketvec& out, int sec, int usec)
{
#ifdef __WIN32__
	if(!get_mutex(Mutex))
		::abort();
#endif
	out.set_size(0);
	int num = poll2_impl(whatfor, sec, usec, out);
	if(num < 0)
		num = -1;
#ifdef __WIN32__
	ReleaseMutex(Mutex);
#endif
	return num;
}

vc_winsock::vc_winsock() : 
	vcxr(this, 0, 2048, vcxstream::CONTINUOUS_READAHEAD), vcxs(this)
{
	if(!have_net)
	{
		if(startup())
        	have_net = 1;
    }

	sock = INVALID_SOCKET;
	used_connect = 0;
	is_listening = 0;
	async_error = 0;
#ifdef BUFFER_SOCKETS
	buffered = 1;
#else
	buffered = 0;
#endif
#ifdef __WIN32__
	if(!get_mutex(Mutex))
		::abort();
#endif
	//dispatcher->register_handler(&ioh);
	if(All_socks)
        All_socks->add(this);
#ifdef __WIN32__
	ReleaseMutex(Mutex);
#endif
}

// note: this is private because it is only used
// when creating a new vc_winsock object after
// a successful accept call.
vc_winsock::vc_winsock(const vc_winsock& w) : 
	vcxr(this, 0, 2048, vcxstream::CONTINUOUS_READAHEAD), vcxs(this)
{
	sock = w.sock;
	peer_addr = w.peer_addr;
	local_addr = w.local_addr;
	used_connect = w.used_connect;
	error = w.error;
	is_listening = 0;
	err_callback = w.err_callback; // XXX yuck, fix this
	async_error = w.async_error;
	buffered = w.buffered;
#ifdef __WIN32__
	if(!get_mutex(Mutex))
		::abort();
#endif
	//dispatcher->register_handler(&ioh);
	if(All_socks)
        All_socks->add(this);
#ifdef __WIN32__
	ReleaseMutex(Mutex);
#endif
}

vc_winsock::~vc_winsock()
{
#ifdef __WIN32__
	if(!get_mutex(Mutex))
		::abort();
#endif
	// we added these 'if's because sometimes
	// global dtors might get fired that reference
	// a socket *after* the application has called "thread_shutoff"
	// and turned off the dispatcher and stuff.
	//if(dispatcher)
		//dispatcher->clear_handler(&ioh);
	if(Read_set)
		Read_set->del(this);
	if(Write_set)
		Write_set->del(this);
	if(All_socks)
        All_socks->del(this);
#ifdef __WIN32__
	ReleaseMutex(Mutex);
#endif
	if(sock != INVALID_SOCKET)
	{
		// note: this dtor may have been invoked by final
		// tear-down of system, so raising an exception
		// is a problem (since it may try referring to
		// an exception context that has already been destroyed)
		// so we punt...
		//RAISE(warn_not_closed,);
		// which brings up a bigger issue... should we just tell the
		// final destructor mechanism to punt on all cleanup
		// since we are likely to be exitting anyway...?
		//
		// set don't-linger... since this is a hard close
#if 0
#ifdef USE_WINSOCK
		int no = 0;
        setsockopt(sock, SOL_SOCKET, SO_DONTLINGER, (const char *)&no, sizeof(no));
#endif
		struct linger l;
		l.l_onoff = 1;
		l.l_linger = 0;
		setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&l, sizeof(l));
#endif
#ifdef USE_WINSOCK
		//shutdown(sock, 2);
		closesocket(sock);
#else
		::close(sock);
#endif
        sock = INVALID_SOCKET;
	}
}

// this one is used to set up a socket that
// has already been allocated by the os.
// like in situations where a child has been
// forked and a socket inherited.

vc
vc_winsock::socket_init(SOCKET os_handle, vc listening, int)
{
	if(sock != INVALID_SOCKET)
		RAISEABORT(generic_problem, vcnil);
		
	sock = os_handle;
	if(socket_local_addr().is_nil())
		return vcnil;
	if(listening.is_nil() && socket_peer_addr().is_nil())
		return vcnil;
	is_listening = !listening.is_nil();
	// note: we have to go with default values
	// for must of the other stuff, since we
	// can't even query it (like listening state
	// and such.) just have to assume the
	// caller knows what is going on.
	// this probably shouldn't be used except in
	// cases where an accepted socket is being
	// handed off to a child.
	return vctrue;
	
}

struct sockaddr *
vc_winsock::make_sockaddr(const vc& local_addr, int& len_out)
{
		
	struct sockaddr *sap;
	if(!local_addr.is_nil())
	{
		if(!vc_to_sockaddr(local_addr, sap, len_out))
		{
			RAISEABORT(bad_addr_format, 0);
		}
		return sap;
	}
	else
	{
        struct sockaddr_in& sa = *(struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
		sa.sin_port = 0;
		sa.sin_family = PF_INET;
		sa.sin_addr.s_addr = INADDR_ANY;
		sap = (struct sockaddr *)&sa;
		len_out = sizeof(sa);
		return sap;
	}
}

vc
vc_winsock::socket_init(const vc& local_addr, int is_listen, int reuseaddr, int)
{
	sock = get_socket();
	if(sock == INVALID_SOCKET)
	{
		RAISEABORT(bad_socket_create, vcnil);
	}

	if(!local_addr.is_nil())
	{
		int len;

        struct sockaddr *_sap = make_sockaddr(local_addr, len);
        scoped_sockaddr sap(_sap);
		if(!sap)
		{
			return vcnil;
		}

		if(reuseaddr)
		{
			const int yes = 1;
			if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
				(const char *)&yes, sizeof(yes)) == SOCKET_ERROR)
            {
				RAISEABORT(generic_problem, vcnil);
			}
		}

        if(::bind(sock, sap, len) == SOCKET_ERROR)
		{
			RAISEABORT(bad_bind, vcnil);
			/*NOTREACHED*/
		}

		if(is_listen)
		{
			if(listen(sock, 5) == SOCKET_ERROR)
			{
				RAISEABORT(bad_listen, vcnil);
				/*NOTREACHED*/
			}
			is_listening = 1;
		}
	}

	return vctrue;
}

// note: need to use "close_info" to decide on a hard_close
// or not. has impact on wouldblock processing too, since
// closesocket may fail with wouldblock if linger is set.
vc
vc_winsock::socket_close(int close_info)
{
	if(sock != INVALID_SOCKET)
	{
#if 0
		struct linger l;
		l.l_onoff = close_info == 1 ? 0 : 1;
		l.l_linger = 3;
		setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char *)&l, sizeof(l));
#ifdef USE_WINSOCK
		int no = 0;
		setsockopt(sock, SOL_SOCKET, SO_DONTLINGER, (const char *)&no, sizeof(no));
#endif
		//fix this
		socket_set_option(VC_BLOCKING);
#endif
#ifdef USE_WINSOCK
		shutdown(sock, 2);
		closesocket(sock);
#else
		::close(sock);
#endif
		sock = INVALID_SOCKET;
		used_connect = 0;
		async_error = 0;
#if defined(STREAM_RECV) || defined(STREAM_XMIT)
	// don't do this, because if an exception is
	// raised that closes the socket, we still
	// want the buffered contents that preceeded
	// the exception. it is up to the client to
	// close down the streams when they are done
	// with them.
#if 0
		
		vcxr.close(vcxstream::DISCARD);
		vcxs.close(vcxstream::DISCARD);
#endif
#endif
		return vctrue;
	}
	return vcnil;
}

vc
vc_winsock::socket_shutdown(int how)
{
	if(!init())
    	return vcnil;
#ifdef USE_WINSOCK
// don't want the misery of including winsock2.h
	switch(how)
	{
	case VC_SOCK_SHUTDOWN_RD:
		how = 0; //SD_RECEIVE
		break;
	case VC_SOCK_SHUTDOWN_WR:
		how = 1; // SD_SEND
		break;
	case VC_SOCK_SHUTDOWN_BOTH:
		how = 2; // SD_BOTH
		break;
	default:
		RAISEABORT(generic_problem, vcnil);
	}
#else
	switch(how)
	{
	case VC_SOCK_SHUTDOWN_RD:
		how = SHUT_RD;
		break;
	case VC_SOCK_SHUTDOWN_WR:
		how = SHUT_WR;
		break;
	case VC_SOCK_SHUTDOWN_BOTH:
		how = SHUT_RDWR;
		break;
	default:
		RAISEABORT(generic_problem, vcnil);
	}
#endif
	if(::shutdown(sock, how) == -1)
	{
		RAISEABORT(generic_problem, vcnil);
	}
	return vctrue;
}
	

vc
vc_winsock::socket_accept(vc& addr_info)
{
	if(!init())
    	return vcnil;

	struct sockaddr *_sa = get_sockaddr();
	scoped_sockaddr sa(_sa);
#ifdef UNIX
	socklen_t len = get_sockaddr_len();
#else
	int len = get_sockaddr_len();
#endif

	SOCKET newsock = accept(sock, sa, &len);
	if(newsock == INVALID_SOCKET)
	{
		RAISEABORT(bad_accept, vcnil);
	}

	addr_info = sockaddr_to_vc(sa, len);
	// for listening sockets, peer_addr is last peer connected
    peer_addr = addr_info;

	len = get_sockaddr_len();
	if(getsockname(newsock, sa, &len) == SOCKET_ERROR)
	{
		RAISEABORT(generic_problem, vcnil);
	}
	
	vc_winsock_stream *nv = wrap_sock(newsock, *this);
	nv->peer_addr = addr_info;
	nv->local_addr = sockaddr_to_vc(sa, len);
	
	vc v;
	v.redefine(nv);
    return v;
}

vc
vc_winsock::socket_connect(const vc& addr_info)
{
	if(!init())
    	return vcnil;
	
	struct sockaddr *sap = 0;
	int len;

	if(!vc_to_sockaddr(addr_info, sap, len))
	{
		RAISEABORT(bad_addr_format, vcnil);
	}

	// set the peer here provisionally. we assume that
	// if the connect finally does complete in the
	// case of non-blocking sockets, the peer won't
	// have changed...
	peer_addr = addr_info;
	// assume here that UDP sockets will never block on
	// a connect. this gets the used_connect member set
	// properly even with non-blocking TCP sockets.
	used_connect = 1;
	if(connect(sock, sap, len) == SOCKET_ERROR)
	{
		// set the local address even if connection can be
		// made immediately.
		last_wsa_error = WSAGetLastError();
        if(smode == VC_NONBLOCKING && last_wsa_error == EINPROGRESS)
          last_wsa_error = EWOULDBLOCK;
		if(smode == VC_NONBLOCKING && last_wsa_error == EWOULDBLOCK)
		{
			// NOTE: in win8, the local address is not available until
			// the connect completes. i'm not sure if/why we would need
			// the local address before then, so we'll just let it go.
#ifndef __WIN32__
			if(socket_local_addr().is_nil())
			{
				if(sap)
					free(sap);
				return vcnil;
			}
#endif
			// socket_local_addr resets error with call to
			// getsockname, need to reset it so error handling
			// can proceed
			WSASetLastError(EWOULDBLOCK);
			last_wsa_error = EWOULDBLOCK;
			// EL FUCKTO! builder4 compiler generates code that
			// clears last error if it is thread local
            //volatile int a;
            //a = WSAGetLastError();
            //a = WSAGetLastError();

		}
		if(sap)
		{
			free(sap);
			sap = 0;
		}
		RAISE(bad_connect, vcnil);
	}
	if(sap)
		free(sap);
	socket_local_addr();

	return vctrue;
}

int
vc_winsock::socket_poll(int whatfor, long to_sec, long to_usec)
{
	if(!init())
		return -1;

#ifndef USE_POLL
	int cnt = 0;
	
	fd_set rset;
	fd_set wset;
	fd_set eset;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);

	int dont_block = 0;
	if(whatfor & VC_SOCK_READ)
	{
		if(vcxr.has_input())
		{
			dont_block = 1;
		}
		else
		{
			FD_SET(sock, &rset);
			++cnt;
		}
	}
	if(whatfor & VC_SOCK_WRITE)
	{
		FD_SET(sock, &wset);
		++cnt;
	}
	if(whatfor & VC_SOCK_ERROR)
	{
		FD_SET(sock, &eset);
		++cnt;
	}
	int block = 0;
	struct timeval tv;
	if(dont_block || (to_sec != -1 && to_usec != -1))
	{
		if(dont_block)
		{
			to_sec = 0;
			to_usec = 0;
		}
		tv.tv_sec = to_sec;
        tv.tv_usec = to_usec;
	}
	else
	{
		block = 1;
    }

	long num = 0;
	
	if(cnt > 0)
	{
		num = select(FD_SETSIZE, &rset, &wset, &eset, (block ? 0 : &tv));

		if(num == SOCKET_ERROR)
		{
			RAISEABORT(bad_select, -1);
		}
	}

	int ret = 0;

	//if(num == 0)
	//	return 0;
	if((whatfor & VC_SOCK_READ) && ((num != 0 && FD_ISSET(sock, &rset)) || vcxr.has_input()))
		ret |= VC_SOCK_READ;
	if(num != 0 && FD_ISSET(sock, &wset))
		ret |= VC_SOCK_WRITE;
	if(num != 0 && FD_ISSET(sock, &eset))
		ret |= VC_SOCK_ERROR;

	return ret;
#else

	struct pollfd pfd;
	pfd.fd = sock;
	pfd.events = 0;

	int dont_block = 0;
	if(whatfor & VC_SOCK_READ)
	{
		if(vcxr.has_input())
		{
			dont_block = 1;
		}
		else
			pfd.events |= POLLIN;
	}
	if(whatfor & VC_SOCK_WRITE)
	{
		pfd.events |= POLLOUT;
	}
	if(whatfor & VC_SOCK_ERROR)
	{
		pfd.events |= POLLPRI;
	}
	int block = 0;
	if(dont_block || (to_sec != -1 && to_usec != -1))
	{
		if(dont_block)
		{
			to_sec = 0;
			to_usec = 0;
		}
	}
	else
	{
		block = 1;
    }
        long num;
        while(1)
        {
            num = ::poll(&pfd, 1, (block ? -1 : to_sec * 1000 + to_usec / 1000));
            
            if(num == SOCKET_ERROR)
            {
                // this is weird, on linux even if the polling timeout is 0, eintr
                // can be returned
                if(errno == EINTR)
                    continue;
                RAISEABORT(bad_select, -1);
                // NOTREACHED
                break;
            }
            else
                break;
        }

	int ret = 0;
	if((whatfor & VC_SOCK_READ) && ((num != 0 && (pfd.revents & (POLLIN|POLLERR|POLLHUP))) 
		|| vcxr.has_input()))
		ret |= VC_SOCK_READ;
	if(num != 0 && (whatfor & VC_SOCK_WRITE) && (pfd.revents & (POLLOUT|POLLERR|POLLHUP)))
		ret |= VC_SOCK_WRITE;
	if(num != 0 && (whatfor & VC_SOCK_ERROR) && (pfd.revents & (POLLPRI|POLLERR|POLLHUP)))
		ret |= VC_SOCK_ERROR;

	return ret;
#endif
}


vc
vc_winsock::socket_send(vc item, const vc& send_info, const vc&)
{
	if(!init())
    	return vcnil;
	
	vcxstream *os;
	vcxstream osl(this);
	if(buffered)
	{
		os = &vcxs;
	}
	else
	{
		os = &osl;
	}
	vcxstream& ostrm = *os;

	if(smode == VC_NONBLOCKING)
	{
		static vc retry("retry");
		if(ostrm.get_status() != vcxstream::RETRYING)
		{
			if(ostrm.get_status() != vcxstream::WRITEABLE)
			{
				if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
				{
					RAISEABORT(generic_problem, vcnil);
				}
			}
			// should never happen unless we run out of memory or something nasty like that
			vc_composite::new_dfs();
			if(item.xfer_out(ostrm) < 0) 
			{
				ostrm.close(vcxstream::DISCARD);
				RAISEABORT(generic_problem, vcnil);
			}
		}
		else
		{
			if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
			{
				RAISEABORT(generic_problem, vcnil);
			}
		}
		// try pushing it out once now
		switch(ostrm.flushnb())
		{
		case NB_DONE:
			// this gets the chit table discarded so open/close
			// pairs-up properly for graph cycle computations
			if(!ostrm.close(vcxstream::DISCARD))
			{
				RAISEABORT(generic_problem, vcnil);
			}
			return vctrue;
		case NB_RETRY:
			// when we return retry, it is up to the caller
			// to call the crank function to continue to
			// try to send the remaining data
			if(!ostrm.close(vcxstream::RETRY))
			{
				RAISEABORT(generic_problem, vcnil);
			}
			return retry;
		case NB_ERROR:
			ostrm.close(vcxstream::DISCARD);
			RAISEABORT(generic_problem, vcnil);
		}

	}
	else
	{
	if(!ostrm.open(vcxstream::WRITEABLE/*, vcxstream::ATOMIC*/))
	{
		RAISEABORT(generic_problem, vcnil);
    }
    vc_composite::new_dfs();
	if(item.xfer_out(ostrm) < 0)
	{
		ostrm.close(vcxstream::DISCARD);
		RAISEABORT(bad_send_raw, vcnil); // fix this
	}
	if(!ostrm.flush())
	{
		ostrm.close(vcxstream::DISCARD);
		RAISEABORT(bad_send_raw, vcnil); // fix this
	}
	ostrm.close();
	return vctrue;
	}
	return vcnil;
}

long
vc_winsock::overflow(vcxstream&, char *buf, long len)
{
	vc sent = socket_send_raw(buf, len, 0);
	if(sent.is_nil())
		return -1;
	long lsent = (long)sent;
	if(smode != VC_NONBLOCKING && lsent < len)
		return -1;
	// allow less than full return in non-blocking
	// case.
	return lsent;
}

long
vc_winsock::underflow(vcxstream&, char *buf, long min, long max)
{
	vc dum;
	long hmmm = -max;
	vc t = socket_recv_raw(buf, hmmm, 0, dum);
	if(t.is_nil())
	{
	//VcError << "underflow1 ret -1\n";
		return -1;
	}
	long got = (long)t;
	if(got < min)
	{
		buf += got;
		hmmm = min - got;
		t = socket_recv_raw(buf, hmmm, 0, dum);
		if(!t.is_nil())
		{
	//VcError << "underflow2 got " << (long)t << "\n";
			got += (long)t;
			//return -1;
		}
	}
	//VcError << "underflow got " << got << "\n";
	// NOTE: this is a HACK, see the comments in socket_recv_raw
	if(got < min && error != vc("eof"))
	{
//VcError << "underflow hack invoked\n";
		WSASetLastError(EWOULDBLOCK);
		last_wsa_error = EWOULDBLOCK;
		RAISE(wouldblock, got);
	}
    return got;
}


vc
vc_winsock::socket_recv(vc& item, const vc& recv_info, vc& addr_info)
{
	if(!init())
		return vcnil;
    addr_info = peer_addr;
	vcxstream *is;
	vcxstream isl(this);
	if(buffered)
		is = &vcxr;
	else
		is = &isl;
	vcxstream& istrm = *is;

	if(buffered)
	{
	if(istrm.get_status() != vcxstream::READABLE)
	{
		if(!istrm.open(vcxstream::READABLE, 
			smode == VC_NONBLOCKING ? vcxstream::ATOMIC : vcxstream::MULTIPLE))
		{
			RAISEABORT(generic_problem, vcnil);
		}
	}
	int err;
	if((err = item.xfer_in(istrm)) < 0)
	{
		static vc wouldblock("wouldblock");
		static vc retry("retry");
		if(smode == VC_NONBLOCKING &&
			error == wouldblock && err == EXIN_DEV)
		{
			istrm.close(vcxstream::RETRY);
			return retry;
		}
		else
			istrm.close(vcxstream::DISCARD);
		return vcnil;
	}
	// note: this implies that per-send is how cycles are
	// computed 
	istrm.chit_new_table();
	istrm.commit();
	}
	else
	{
	if(!istrm.open(vcxstream::READABLE))
	{
		RAISEABORT(generic_problem, vcnil);
    }
	if(item.xfer_in(istrm) < 0)
	{
		istrm.close(vcxstream::DISCARD);
		return vcnil;
	}
	istrm.close();
	}

	return vctrue;
}

vc
vc_winsock::socket_send_raw(void *obuf, long& len, long timeout, const vc&)
{
	if(!init())
		return vcnil;

	int nsent = 0;
    char *buf = (char *)obuf;
	
	while(nsent < len)
	{
		int npsent;
		
		npsent = send(sock, buf, len - nsent, 0);
		if(npsent == SOCKET_ERROR)
		{
			switch((last_wsa_error = WSAGetLastError()))
			{
#ifdef LINUX
			case EINTR:
                                continue;
                            // note: on linux, eintr appear to show up even with non-blocking
                            // sockets from time to time... odd
				RAISEABORT(bad_send_raw, vc(nsent));
				/*NOTREACHED*/
#endif

			case ENOBUFS:
				RAISE(send_no_bufs, vc(nsent));
				continue;

			case EWOULDBLOCK:
#if defined(LINUX) && EWOULDBLOCK != EAGAIN
                        case EAGAIN:
#endif
				RAISE(wouldblock, vc(nsent));
				continue;
#if 0
			case EPIPE: /* not documented in send man page... see write */
				raise_with_fd("A:PROC.REJECT.CONNECTION_CLOSED", fd);
				/*NOTREACHED*/
#endif

				/* that these are returned by send is *not* documented
				 * (not in write either.) i haven't seen them show up
				 * yet, but they're here for safety.
				 */
			case ECONNRESET:
			case ECONNABORTED:
			case EHOSTDOWN:
			case ENETDOWN:
			case ENETRESET:
			case ETIMEDOUT: /* this one showed up once when talking to a PC (?!). */
				// for tcp sockets, not much chance of anything working
				// again...
				RAISEABORT(bad_send_raw, vc(nsent));
				/*NOTREACHED*/

			default:
				RAISEABORT(bad_send_raw, vc(nsent));
				/*NOTREACHED*/
			}
		}
		nsent += npsent;
		buf += npsent;
	}
	return vc(nsent);
}

vc
vc_winsock::socket_recv_raw(void *ibuf, long& len, long timeout, vc& addr_info)
{
	if(!init())
		return vcnil;

	int nread = 0;
	char *buf = (char *)ibuf;
	addr_info = peer_addr; // always good for accepted sockets
	//VcError << "ask len " << len << "\n";
	if(len < 0) // special case, just read what's there
	{
#ifdef USE_WINSOCK
		long klen;
		if(ioctlsocket(sock, FIONREAD, (unsigned long *)&klen) == SOCKET_ERROR)
#else
		int klen;
		if(ioctl(sock, FIONREAD, &klen) == SOCKET_ERROR)
#endif
		{
			RAISEABORT(generic_problem, vcnil);
			/*NOTREACHED*/
		}
#if 0
		if(klen == 0)
			return vczero;
#endif
		len = dwmin(-len, klen);
	//VcError << "getting len " << len << "\n";
	}
	
	while(nread < len)
	{
		int npread;
		
		npread = recv(sock, buf, len - nread, 0);
		//VcError << "npread " << npread << "\n";
		if(npread == SOCKET_ERROR)
		{
			switch((last_wsa_error = WSAGetLastError()))
			{
#ifdef LINUX
			case EINTR:
                            continue;
                RAISEABORT(bad_recv_raw, vc(nread));
				/*NOTREACHED*/
#endif
				
				/* that these are returned by recv is *not* documented
				 * (not in read either.) they seem to show up
				 * once in a while...
				 */
			case ECONNRESET:
			case ECONNABORTED:
			case EHOSTDOWN:
			case ENETDOWN:
			case ENETRESET:
			case ETIMEDOUT: /* this one showed up once when talking to a PC (?!). */
				// for tcp sockets, there is little chance that
				// anything much will work on it anymore, so
				// don't allow a resume...
				RAISEABORT(bad_recv_raw, vc(nread));
				/*NOTREACHED*/

			case EWOULDBLOCK:
#if defined(LINUX) && EWOULDBLOCK != EAGAIN
                        case EAGAIN:
#endif
			//VcError << "wouldblock\n";
				RAISE(wouldblock, vc(nread));
				continue;
				
			default:
				RAISEABORT(bad_recv_raw, vc(nread));
				/*NOTREACHED*/
			}
		}
		else if(npread == 0)
		{
			if(nread > 0)
            {
              vc tmp(nread);
              //VcError << "read 0 nread " << nread << "\n";
              return tmp;
            }
		  //VcError << "read 0 eof " << "\n";
			RAISE(recv_eof, vc(nread));
			break;
		}

		nread += npread;
		buf += npread;

		/*
		// note3: this is getting really irritating.
		// 
		the #if 1 below is NECESSARY, because otherwise
		you will do a bunch of reads from the OS, and
		then raise an exception, which will leave the data
		lying around and not put anywhere.  this is only
		interesting in NONBLOCKING mode because in blocking mode
		an error is fatal and you aren't going to get anywhere
		after that anyway.

		unfortunately, there is a problem with not getting
		all the data that is on the socket (with a resulting
		wouldblock exception when the socket is empty).
		what happens is that higher levels ask for a large
		number of bytes, but only get a handful, which is
		ok. but when the higher levels go to reconstruct
		and find there isn't enough data, there was no wouldblock
		exception raised to let the upper levels know that.
		in effect, it is a bad idea to rely on device errors
		to be reflected at the higher level to indicate when
		a large data structure isn't ready.

		this really needs to be fixed in a big way, but it
		is hard at the moment because it involves decoupling
		a lot of the error handling from the upper level
		error reporting.
		
		as a hack around, we can leave the check in below, but
		raise "pseudo wouldblock" exceptions at the higher
		level that will satify the LH interpreter.

		it is interesting to note that, at this level anyway,
		trying to use the "wouldblock" model is a bitch.
		essentially we are returning a "wouldblock" *and*
		some data (unlike the OS, which doesn't return the
		data at all on a wouldblock). this whole thing needs to
		be rethought a bit, because it has been a headache in
		a big way. one thing might to be get rid of the idea
		of "wouldblock" (or make in an error) and replace it
		with "get whatever is there". at least with the latter, you
		don't run into these problems with being unable to
		pass large things thru the underlying system (wouldblock
		with a size of N is really a request for an atomic
		operation, and if N is too big, you are screwed.)
		
		
		*/
		// note2: taking this back out again, because it
		// appears to have broken something... 
		
		// note: reinstating this "broken fix" because i
		// think it works ok now. if we are in blocking mode,
		// we really want to keep going until we get everything
		// we were asked for (implemented MSG_WAITALL, basically)
		// or an error occurs. note that the exception is raised
		// and processed *before* we return, so all kinds of
		// things could have happened (this is a problem... but too
		// lazy to fix now.) 
#if 1
		// this used to be a fix for failing nonblocking reads, but
		// it breaks other things. the upshot is that if you
		// ask for N bytes, and don't get it, even in nonblocking
		// mode, it is up to the caller to set up the appropriate
		// wouldblock handler. this really needs to be addressed with
		// a better organization of the overall message handling 
		// and message reconstruction.
		
        if(smode == VC_NONBLOCKING)
        {
	//VcError << "nb break" << "\n";
          break; // only do one read, since we can't stash partial read results in this case
	  }
#endif
	}
	vc tmp(nread);
	//VcError << "nread ret " << nread << "\n";
    return tmp;
}

int
vc_winsock::init()
{
#ifdef USE_WINSOCK
	if(async_error != 0)
	{
		WSASetLastError(async_error);
		//async_error = 0;
		RAISEABORT(generic_problem, 0);
	}
	WSASetLastError(WSAEBADF);
#else
	WSASetLastError(EBADF);
#endif
	if(sock == INVALID_SOCKET)
		RAISEABORT(generic_problem, 0);
	return 1;
}


//
// error handling functions
//

void
vc_winsock::set_async_error(SOCKET s, int err)
{
    vc_winsock *vcs;

#ifdef __WIN32__
    if(!get_mutex(Mutex))
        ::abort();
#endif
    if(All_socks)
    {

        SocketSetIter i(All_socks);
        for(; !i.eol(); i.forward())
        {
            vcs = i.get();
            if(vcs->sock == INVALID_SOCKET)
                continue;
            if(vcs->sock == s)
                break;
        }
        if(!i.eol())
        {
            vcs->errvc = vc_wsget_errstr(err);
            vcs->async_error = err;
        }
    }

#ifdef __WIN32__
    ReleaseMutex(Mutex);
#endif
}

int
vc_winsock::vc_to_sockaddr(const vc& v, struct sockaddr *& sapr, int& len)
{
	unsigned short in_port = 0;

    DwString inp((const char *)v);

    int colpos = inp.find(":");
    DwString ip;
    if(colpos != DwString::npos)
    {
        ip = inp;
        ip.remove(colpos);
        DwString sport = inp;
        sport.erase(0, colpos + 1);
        if(!sport.eq("any"))
            in_port = htons(atoi(sport.c_str()));
    }
    else
        ip = inp;

    struct in_addr in_addr;
    in_addr.s_addr = INADDR_ANY;

    if(!ip.eq("any"))
	{
        int res = inet_pton(AF_INET, ip.c_str(), &in_addr);
		// bogus, doesn't work right for broadcast address
		// need to use updated version of inet_addr
        if(res == 0)
		{
			return 0;
		}
	}

	struct sockaddr_in *sap = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

	memset(sap, 0, sizeof(*sap));

	sap->sin_family = AF_INET;
    sap->sin_addr = in_addr;
	sap->sin_port = in_port;
	sapr = (struct sockaddr *)sap;
	len = sizeof(*sap);
	return 1;
}

vc
vc_winsock::sockaddr_to_vc(struct sockaddr *sapi, int len)
{
    struct sockaddr_in *sap = (struct sockaddr_in *)sapi;

    char *str = inet_ntoa(sap->sin_addr);
    if(str == 0)
        return vcnil;
    DwString in_addrstr(str);

    if(sap->sin_port != 0)
    {
        in_addrstr += ':';
        in_addrstr += DwString::fromUint(ntohs(sap->sin_port));
    }

    vc ret(in_addrstr.c_str());
    return ret;
}


int
vc_winsock::do_error_processing()
{
	// get the last error code and map to standard string

	int err = last_wsa_error;
	if(error != vc("fatal"))
	{
		if(err == EWOULDBLOCK)
		{
			error = "wouldblock";
			if(smode == VC_NONBLOCKING)
			{
				errvc = vc_wsget_errstr(err);
				return vc_socket::do_error_processing();
			}
		}
	}
			
	errvc = vc_wsget_errstr(err);

	return vc_socket::do_error_processing();
}

int
vc_winsock::do_fatal_error_processing()
{
	exc_level = 'A';
	error = "fatal";
	int ret = do_error_processing();
	// note: really need close that is "hard"
	
	if(sock != INVALID_SOCKET)
	{
		shutdown(sock, 2);
#ifdef USE_WINSOCK
		closesocket(sock);
#else
		::close(sock);
#endif
		sock = INVALID_SOCKET;
	}
	return ret;
}

int
vc_winsock::do_warn_error_processing()
{
	exc_level = 'W';
	error = "warning";
	WSASetLastError(0);
	last_wsa_error = 0;
	return do_error_processing();
}

int
vc_winsock::do_resumable_error_processing()
{
	exc_level = 'E';
	error = "resumable";
	return do_error_processing();
}

int vc_winsock::warn_not_closed() { return do_warn_error_processing(); }
int vc_winsock::bad_socket_create() { return do_fatal_error_processing(); }
int vc_winsock::bad_addr_format()
{
	// this is hokey, but there are no "error codes"
	// defined for situations where malformed address
	// strings are given to the inet_addr, et. al.
	// routines.
	WSASetLastError(EADDRNOTAVAIL);
	last_wsa_error = EADDRNOTAVAIL;
	return do_fatal_error_processing();
}
int vc_winsock::generic_problem() { return do_fatal_error_processing(); }
int vc_winsock::bad_bind() { return do_fatal_error_processing(); }
int vc_winsock::bad_listen() { return do_fatal_error_processing(); }
// note: even though we use resumable "wouldblock" processing here,
// we don't allow the call to resume. if you want to be able
// to do that: at the site of the call, check for EWOULDBLOCK
// and raise the "wouldblock" exception, which allows for a 
// resume.
int
vc_winsock::special_fatal_handling()
{
	if(smode == VC_NONBLOCKING && last_wsa_error == EWOULDBLOCK)
	{
		do_resumable_error_processing();
		return VC_SOCKET_BACKOUT;
	}
	return do_fatal_error_processing();
}
int vc_winsock::bad_accept()
{
	if(last_wsa_error == ECONNRESET) // linux hack-around
	{
		do_resumable_error_processing();
		return VC_SOCKET_BACKOUT;
	}
	return special_fatal_handling();
}
int vc_winsock::bad_connect()
{
	if(last_wsa_error == EISCONN)
		return do_resumable_error_processing();
	return special_fatal_handling();
}

int vc_winsock::bad_select() { return special_fatal_handling(); }
int vc_winsock::send_no_bufs() { return do_resumable_error_processing(); }
int vc_winsock::wouldblock()
{
    // this just maps EAGAIN to EWOULDBLOCK on systems that have both of
    // these things (duh)
    WSASetLastError(EWOULDBLOCK);
	last_wsa_error = EWOULDBLOCK;
	int t = do_resumable_error_processing();
	error = "wouldblock";
	return t;
}
int vc_winsock::bad_send_raw_soft() { return do_resumable_error_processing(); }
int vc_winsock::bad_send_raw() { return do_fatal_error_processing(); }
int vc_winsock::bad_recv_raw_soft() { return do_resumable_error_processing(); }
int vc_winsock::bad_recv_raw() { return do_fatal_error_processing(); }
int vc_winsock::recv_eof()
{
	exc_level = 'E';
	errvc = "SOCKET_EOF";
	error = "eof";
	int t = vc_socket::do_error_processing();
	//VcError << "socket eof\n";
	return t;
	
}
int vc_winsock::bad_dgram_recv() { return do_fatal_error_processing(); }
vc
vc_winsock::socket_error()
{
	return error;
}
vc
vc_winsock::socket_error_desc()
{
	return errvc;
}

void
vc_winsock::socket_set_error(vc v)
{
	error = v;
}

vc
vc_winsock::socket_set_option(vcsocketmode m, 
	unsigned long a1, unsigned long a2, unsigned long a3)
{
	if(!init())
		return vcnil;
	unsigned long b;
	int val;
#if !defined(_Windows)
	socklen_t sl = sizeof(val);
#endif
	switch(m)
	{
	case VC_BUFFER_SOCK:
		buffered = a1;
		return vctrue;
	case VC_BLOCKING:
		b = 0;
		break;
	case VC_NONBLOCKING:
		b = 1;
		break;
#if defined(USE_WINSOCK)
	case VC_WSAASYNC_SELECT:
		if(WSAAsyncSelect(sock, (HWND)a1, a2, a3) == SOCKET_ERROR)
		{
			RAISEABORT(generic_problem, vcnil);
		}
		smode = VC_NONBLOCKING;
		return vctrue;
#endif
	case VC_CLOSE_ON_EXEC:
	// this is the default behavior on windows, so don't
	// raise an error here.
#if !defined(_Windows)
		if((val = fcntl(sock, F_GETFD, 0)) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
		if(fcntl(sock, F_SETFD, val|FD_CLOEXEC) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
#endif
		return vctrue;
	case VC_NO_CLOSE_ON_EXEC:
#if defined(_Windows)
		RAISEABORT(generic_problem, vcnil);
#else
		if((val = fcntl(sock, F_GETFD, 0)) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
		if(fcntl(sock, F_SETFD, val & ~FD_CLOEXEC) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
#endif
		return vctrue;
	case VC_SET_RECV_BUF_SIZE:
#if defined(_Windows)
		RAISEABORT(generic_problem, vcnil);
#else
		val = a1;
		if(setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
		return vctrue;
#endif

	case VC_GET_RECV_BUF_SIZE:
#if defined(_Windows)
		RAISEABORT(generic_problem, vcnil);
#else
		if(getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &val, &sl) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
		return val;
#endif

	case VC_SET_SEND_BUF_SIZE:
#if defined(_Windows)
		RAISEABORT(generic_problem, vcnil);
#else
		val = a1;
		if(setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val)) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
		return vctrue;
#endif

	case VC_GET_SEND_BUF_SIZE:
#if defined(_Windows)
		RAISEABORT(generic_problem, vcnil);
#else
		if(getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &val, &sl) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
		return val;
#endif

	case VC_SET_TCP_NO_DELAY:
#if defined(_Windows)
		RAISEABORT(generic_problem, vcnil);
#else
		val = a1;
        if(setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
		{
			RAISEABORT(generic_problem, vcnil);
		}
		return vctrue;
#endif

    case VC_SET_BROADCAST:
#if !defined(_Windows)
        val = a1;
        if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &val, sizeof(val)) == -1)
        {
            RAISEABORT(generic_problem, vcnil);
        }
#endif
        return vctrue;

	default:
		RAISEABORT(generic_problem, vcnil);
		/*NOTREACHED*/
	}
#if defined(USE_WINSOCK)
	if(ioctlsocket(sock, FIONBIO, &b) == SOCKET_ERROR)
	{
		RAISEABORT(generic_problem, vcnil);
	}
#else
	int f;
	if((f = fcntl(sock, F_GETFL, 0)) == -1)
	{
		RAISEABORT(generic_problem, vcnil);
	}
	if(b)
		f |= O_NDELAY;
	else
		f &= ~O_NDELAY;
	if(fcntl(sock, F_SETFL, f) == -1) /* non-blocking i/o */
	{
		RAISEABORT(generic_problem, vcnil);
	}
#endif
	smode = m;
	return vctrue;
}

vc
vc_winsock::socket_local_addr()
{
	if(!init())
		return vcnil;
	struct sockaddr *_sa = get_sockaddr();
	scoped_sockaddr sa(_sa);
#ifdef UNIX
	socklen_t len = get_sockaddr_len();
#else
	int len = get_sockaddr_len();
#endif
	if(getsockname(sock, sa, &len) == SOCKET_ERROR)
	{
		RAISEABORT(generic_problem, vcnil);
	}
    local_addr = sockaddr_to_vc(sa, len);
	return local_addr;
}

vc
vc_winsock::socket_peer_addr()
{
	if(!init())
		return vcnil;
	if(!peer_addr.is_nil())
		return peer_addr;
	struct sockaddr *_sa = get_sockaddr();
	scoped_sockaddr sa(_sa);
#ifdef UNIX
	socklen_t len = get_sockaddr_len();
#else
	int len = get_sockaddr_len();
#endif
	if(getpeername(sock, sa, &len) == SOCKET_ERROR)
	{
		RAISEABORT(generic_problem, vcnil);
	}
    // note: the len is the actual len returned at this point, which
    // may be different than what we sent in, especially with unix sockets.
    peer_addr = sockaddr_to_vc(sa, len);
	return peer_addr;
	
}

void
vc_winsock::stringrep(VcIO os) const
{
	// there really isn't a good def of a
	// stringrep for sockets.
	// might want to generate a little code
	// fragment here to regenerate the state of
	// the socket at some point, but for now
	// we just barf out some stuff...
	os << "socket(";
	
	out_flavor(os);
	
	os << " ";
	local_addr.stringrep(os);
	os << " ";
	os << (is_listening ? "t" : "nil");
	os << " ";
	// reuse_addr, problematic... we could just set it
	// to t, since on unix boxes reuse_addr works right,
	// but on windows, it does the wrong thing (reuses it
	// even if the socket is connected to a peer)
	os << " nil";
	os << ")";
}


//
// stream socket objects
//

void
vc_winsock_stream::out_flavor(VcIO os) const
{
	os << "tcp";
}

vc_winsock_stream::vc_winsock_stream()
{
}

// used to construct a winsock object from a newly accepted socket.
vc_winsock_stream::vc_winsock_stream(SOCKET s, const vc_winsock& oldsock)
	: vc_winsock(oldsock)
{
	sock = s;
	used_connect = 0;
}

SOCKET
vc_winsock_stream::get_socket()
{
	SOCKET s = socket(PF_INET, SOCK_STREAM, 0);
	return s;
}

void
vc_winsock_stream::printOn(VcIO os)
{
	os << "socket(stream peer= ";
	peer_addr.printOn(os);
	if(used_connect)
		os << " uconn";
	else
		os << " no_uconn";
		
	os << ")";
}

//
// datagram socket objects
//
// note: datagram sockets transmit and receive
// objects as datagrams. there is no attempt to
// fragment and reassemble datagrams. this means
// that there is an arbitrary limit to the size
// of items that can be transmitted via this
// mechanism. the limit is determined by the
// winsock implementation...
//
char *vc_winsock_datagram::iobuf;
unsigned int vc_winsock_datagram::iobuflen;

void
vc_winsock_datagram::out_flavor(VcIO os) const
{
	os << "udp";
}

vc_winsock_datagram::vc_winsock_datagram()
{
	if(have_net && iobuf == 0)
	{
#if 0
#ifdef USE_WINSOCK
		if(wsa_data.iMaxUdpDg != 0)
		{	
			// bogus for now, something breaks when > 32000
			if(wsa_data.iMaxUdpDg > 32000)
				wsa_data.iMaxUdpDg = 32000; 
			iobuf = new char [wsa_data.iMaxUdpDg];
			iobuflen = wsa_data.iMaxUdpDg;
		}
		else
#endif
#endif
		{

            // impose arbitrary limit
            iobuf = new char [32768];
            iobuflen = 32768;
		}
	}
}

SOCKET
vc_winsock_datagram::get_socket()
{
	SOCKET s = socket(PF_INET, SOCK_DGRAM, 0);
	return s;
}

vc
vc_winsock_datagram::socket_init(SOCKET s, vc listening, int)
{
	return vc_winsock::socket_init(s, listening);
}

vc
vc_winsock_datagram::socket_init(const vc& laddr, int is_listen, int reuseaddr, int)
{
	// ignore "listen" request, since you can't do that on one of these.
    return vc_winsock::socket_init(laddr, 0, reuseaddr);
}	

vc
vc_winsock_datagram::socket_accept(vc& addr_info)
{
	WSASetLastError(ESOCKTNOSUPPORT);
	last_wsa_error = ESOCKTNOSUPPORT;
	do_fatal_error_processing();
	return vcnil;
}

vc
vc_winsock_datagram::socket_send(vc item, const vc& send_info, const vc& to)
{
	if(!init())
		return vcnil;
	
	vcxstream ostrm(this, iobuf, iobuflen, vcxstream::FIXED);
	if(!to.is_nil())
		peer_addr = to;
	if(!ostrm.open(vcxstream::WRITEABLE))
	{
    	RAISEABORT(generic_problem, vcnil);
		/*NOTREACHED*/
	}

	vc_composite::new_dfs();
	if(item.xfer_out(ostrm) < 0 || !ostrm.flush())
	{
		ostrm.close(vcxstream::DISCARD);
		return vcnil;
    }
	ostrm.close();
	return vctrue;
}

long
vc_winsock_datagram::overflow(vcxstream&, char *buf, long len)
{
	long olen = len;
	vc t = socket_send_raw(buf, olen, 0, peer_addr);
	if(t.is_nil())
		return -1;
	long retlen = (long)t;
	if(retlen < len)
		return -1;
	return retlen;
}

long
vc_winsock_datagram::underflow(vcxstream&, char *buf, long min, long max)
{
	vc t = socket_recv_raw(buf, max, 0, peer_addr);
	if(t.is_nil())
		return -1;
	long retlen = (long)t;
	//if(retlen < min)
	//	return -1;
	return retlen;
}


vc
vc_winsock_datagram::socket_recv(vc& item, const vc& recv_info, vc& addr_info)
{
	if(!init())
		return vcnil;

	vcxstream istrm(this, iobuf, iobuflen, vcxstream::FIXED);
    // note: this is where the actual io op occurs.
	if(!istrm.open(vcxstream::READABLE))
	{
		return vcnil;
	}
	addr_info = peer_addr;
	if(item.xfer_in(istrm) < 0)
	{
		istrm.close();
		return vcnil;
	}
	istrm.close();
	return vctrue;
}

vc
vc_winsock_datagram::socket_send_raw(void *obuf, long& len, long timeout, const vc& to)
{
	if(!init())
		return vcnil;

	int nsent = 0;
	char *buf = (char *)obuf;
    struct sockaddr *_addrp = 0;
    int addrlen = 0;

	if(!to.is_nil())
	{
		// note: need this, since after a dgram socket
		// is connected, you can't send packets to
		// another place before you "dissolve" the
		// association (this isn't at all clear from
		// any docs...)
		if(!used_connect || to != peer_addr)
		{
            if(!vc_to_sockaddr(to, _addrp, addrlen))
			{
				RAISEABORT(bad_addr_format, vcnil);
			}
		}
	}
    scoped_sockaddr addrp(_addrp);
	
	
	while(1)
	{
		nsent = sendto(sock, buf, len, 0, addrp, addrlen);
		if(nsent == SOCKET_ERROR)
		{
			switch((last_wsa_error = WSAGetLastError()))
			{
#ifdef LINUX
			case EINTR:
				RAISEABORT(bad_send_raw, vcnil);
				/*NOTREACHED*/
#endif

			case ENOBUFS:
				RAISE(send_no_bufs, vcnil);
				continue;

			case EWOULDBLOCK:
#if defined(LINUX) && EWOULDBLOCK != EAGAIN
                        case EAGAIN:
#endif
				RAISE(wouldblock, vcnil);
				continue;
#if 0
			case EPIPE: /* not documented in send man page... see write */
				raise_with_fd("A:PROC.REJECT.CONNECTION_CLOSED", fd);
				/*NOTREACHED*/
#endif

				/* that these are returned by send is *not* documented
				 * (not in write either.) i haven't seen them show up
				 * yet, but they're here for safety.
				 */
			case ECONNRESET:
			case ECONNABORTED:
			case EHOSTDOWN:
			case ENETDOWN:
			case ENETRESET:
			case ETIMEDOUT: /* this one showed up once when talking to a PC (?!). */
				// something probably screwed in the protocol-stack...
				// allow retry anyway.
				RAISE(bad_send_raw_soft, vcnil);
				continue;

			case EMSGSIZE:
				RAISEABORT(bad_send_raw, vcnil);
				/*NOTREACHED*/

			// you can get these when you send to
			// a non-listening port on the other side, even
			// if you are not in connected-mode...
			// hopefully the udp stack doesn't whack the
			// socket.

			case ECONNREFUSED:
				RAISE(bad_send_raw_soft, vcnil);
				continue;

			default:
				RAISEABORT(bad_send_raw, vcnil);
				/*NOTREACHED*/
			}
		}
		else
        	break;
	}

	vc tmp(nsent);
	return tmp;
}


vc
vc_winsock_datagram::socket_recv_raw(void *ibuf, long& len, long timeout, vc& addr_info)
{
	if(!init())
		return vcnil;

	int nread = 0;
	char *buf = (char *)ibuf;
	struct sockaddr *_sa = get_sockaddr();
	scoped_sockaddr sin(_sa);
#ifdef UNIX
	socklen_t sinlen;
#else
	int sinlen;
#endif
	// hmmm, what about zero length udp packets.
	// nothing in the spec on this.
	if(len < 0) // special case, just read what's there
	{
		long klen;
#ifdef USE_WINSOCK
		if(ioctlsocket(sock, FIONREAD, (unsigned long *)&klen) == SOCKET_ERROR)
#else
		if(ioctl(sock, FIONREAD, &klen) == SOCKET_ERROR)
#endif
		{
			RAISEABORT(generic_problem, vcnil);
			/*NOTREACHED*/
		}
#if 0
		if(klen == 0)
			return vczero;
#endif
		len = dwmin(-len, klen);
	}

	while(1)
	{
	sinlen = get_sockaddr_len();
	// this hack is for winsock2.0, which tells me that
	// the socket is already connected if i used connect
	// on the dgram socket, and then ask for the peer... sigh.
	// this is different from 1.1, where it just coughed up
	// the peer, like you would expect.
	nread = recvfrom(sock, buf, len, 0, 
        used_connect ? (struct sockaddr *)0 : sin,
		used_connect ? 0 : &sinlen);
	if(nread == SOCKET_ERROR)
	{
		switch((last_wsa_error = WSAGetLastError()))
		{
#ifdef LINUX
		case EINTR:
			RAISEABORT(bad_dgram_recv, vcnil);
			/*NOTREACHED*/
#endif
			
			/* that these are returned by recv is *not* documented
			 * (not in read either.) they seem to show up
			 * once in a while...
			 */
			// win2k seems to return these errors now...
			// there is a difference in the way linux and
			// windows does this (and it appears the windows
			// behavior is recent ca 2004) if you get this in windows, the socket will
			// be closed (it is a hard error, not a soft one.)
			// this is true even if the socket is not connected
			// and is a bug in the ms implementation

		case ECONNRESET:
#ifdef _Windows
			// we can't just ignore these, but thankfully it isn't a
			// hard error (ie, the socket can keep receiving after
			// one of these comes thru on xp/2k).
			// i think linux uses ECONNREFUSED for this, but since
			// we set so_bsd_compat under linux, that isn't an issue now.
			RAISE(bad_recv_raw_soft, vcnil);
			continue;
#endif
		case ECONNABORTED:
		case EHOSTDOWN:
		case ENETDOWN:
		case ENETRESET:
		case ETIMEDOUT: /* this one showed up once when talking to a PC (?!). */
			// for datagrams, this is probably a sign that the
			// protocol stack is screwed up... sigh... not much we
			// can do about it...
#ifdef _Windows
			RAISEABORT(bad_recv_raw, vcnil);
#else
			RAISE(bad_recv_raw_soft, vcnil);
#endif
			continue;

			// you can get these when you send to
			// a non-listening port on the other side, even
			// if you are not in connected-mode... and yes,
			// even when you issue the recv, the icmp message
			// can rear it's head at any time.
			// hopefully the udp stack doesn't whack the
			// socket.
			// windows does close the socket (this is a bug, see
			// above.)

		case ECONNREFUSED:
#ifdef _Windows
				RAISEABORT(bad_recv_raw, vcnil);
#else
				RAISE(bad_recv_raw_soft, vcnil);
#endif				
				continue;

		case EWOULDBLOCK:
#if defined(LINUX) && EWOULDBLOCK != EAGAIN
                case EAGAIN:
#endif
			RAISE(wouldblock, vcnil);
			continue;

		case EMSGSIZE:
			// not clear if the address is right in this
			// case or not...
            addr_info = sockaddr_to_vc(sin, get_sockaddr_len());
			RAISEABORT(bad_dgram_recv, vc(len));
			/*NOTREACHED*/
			
		default:
			RAISEABORT(bad_recv_raw, vcnil);
			/*NOTREACHED*/
		}
	}
	else
		break;
	}

	// fix for winsock2.0 bug: return the previously recorded peer
	// address, since we can't get it from recvfrom call...
	if(used_connect)
		addr_info = peer_addr;
	else
        addr_info = sockaddr_to_vc(sin, sinlen);
	return vc(nread);
}

void
vc_winsock_datagram::printOn(VcIO os)
{
	os << "socket(dgram peer= ";
	peer_addr.printOn(os);
	if(used_connect)
		os << " uconn";
	else
		os << " no_uconn";
		
	os << ")";
}

#ifdef UNIX

vc_winsock_unix::vc_winsock_unix()
{
}

// used to construct a winsock object from a newly accepted socket.
vc_winsock_unix::vc_winsock_unix(SOCKET s, const vc_winsock& oldsock)
	: vc_winsock_stream(s, oldsock)
{
}

SOCKET
vc_winsock_unix::get_socket()
{
	int s = socket(PF_UNIX, SOCK_STREAM, 0);
	return s;
}

int
vc_winsock_unix::vc_to_sockaddr(const vc& v, struct sockaddr *& sapr, int& len)
{
	if(v.len() >= PATH_MAX)
		return 0;

	struct sockaddr_un *sap = (struct sockaddr_un *)malloc(sizeof(struct sockaddr_un));

    if(v.len() > sizeof(sap->sun_path) - 1)
    {
        free(sap);
        return 0;
    }

	memset(sap, 0, sizeof(*sap));

	sap->sun_family = AF_UNIX;
	memcpy(sap->sun_path, (const char *)v, v.len());

	sapr = (struct sockaddr *)sap;
    len = offsetof(sockaddr_un, sun_path) + v.len(); //sizeof(*sap);
    // note: we know this is going to a "bind" call, so don't try
    // to finess the length, just give it the size of the whole thing.
    //len = sizeof(*sap);
	return 1;
}

// NOTE: the len here is assumed to be a len returned from one of
// the system calls like accept, getsockname, etc.
// NOT a len cobbled together that might be sent into
// connect or whatever (the reason for this is because there
// are slightly different rules for determining the sun_path
// depending on platform.)
vc
vc_winsock_unix::sockaddr_to_vc(struct sockaddr *sapi, int len)
{
    struct sockaddr_un *sap = (struct sockaddr_un *)sapi;

    // naming for unix domain sockets is really goofy, having
    // unnamed and abstract named sockets, all depends on platform too.
    // using rules from https://www.man7.org/linux/man-pages/man7/unix.7.html
    // ca 2023 as a guide. note, this man page has an error regarding
    // length of abstract names, not taking into account "extra" items that
    // might be before "family" field. also, macos doesn't have abstract
    // names, fwiw.
    // note: probably need an "offset" to take into account
    // padding here too
    int prefix_len = offsetof(sockaddr_un, sun_path);
    // note: some man pages claim to return 0 length for
    // unamed sockets, some say "sizeof stuff at the beginning"
    // so hedging here
    if(len <= prefix_len)
    {
        // unnamed socket
        return vc("");
    }
    // this is linux only i'm guessing
    if(sap->sun_path[0] == '\0')
    {
        // abstract namespace
        return vc(VC_BSTRING, sap->sun_path, len - prefix_len);
    }

    // a filesystem path, null termination is a mystery, so
    // make sure we do it explicitly.
    int plen = strnlen(sap->sun_path, len - prefix_len);

    vc ret(VC_BSTRING, sap->sun_path, plen);
    return ret;
}

struct sockaddr *
vc_winsock_unix::make_sockaddr(const vc& local_addr, int& len_out)
{
	struct sockaddr *sap;
	if(!local_addr.is_nil())
	{
		if(!vc_to_sockaddr(local_addr, sap, len_out))
		{
			RAISEABORT(bad_addr_format, 0);
		}
		return sap;
	}
	else
	{
		RAISEABORT(bad_addr_format, 0);
	}
}

struct sockaddr *
vc_winsock_unix::get_sockaddr()
{
	struct sockaddr_un *sa;
    sa = (struct sockaddr_un *)malloc(sizeof(*sa));
	return (struct sockaddr *)sa;
}

int
vc_winsock_unix::get_sockaddr_len()
{
	return sizeof(struct sockaddr_un);
}

vc_winsock_stream *
vc_winsock_unix::wrap_sock(SOCKET s, const vc_winsock& ws)
{
	return new vc_winsock_unix(s, ws);
}

#endif

struct sockaddr *
vc_winsock::get_sockaddr()
{
	struct sockaddr_in *sa;
    sa = (struct sockaddr_in *)malloc(sizeof(*sa));
	return (struct sockaddr *)sa;
}

int
vc_winsock::get_sockaddr_len()
{
	return sizeof(struct sockaddr_in);
}

vc_winsock_stream *
vc_winsock_stream::wrap_sock(SOCKET s, const vc_winsock& ws)
{
	return new vc_winsock_stream(s, ws);
}

vc
vc_winsock::socket_send_buf(vc item, const vc& send_info, const vc&)
{
	if(!init())
    	return vcnil;

	vcxstream *os;
	vcxstream osl(this);
	if(buffered)
	{
		os = &vcxs;
	}
	else
	{
		os = &osl;
	}
	vcxstream& ostrm = *os;

	if(smode == VC_NONBLOCKING)
	{
		static vc retry("retry");
		if(ostrm.get_status() != vcxstream::RETRYING)
		{
			if(ostrm.get_status() != vcxstream::WRITEABLE)
			{
				if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
				{
					RAISEABORT(generic_problem, vcnil);
				}
			}
			// should never happen unless we run out of memory or something nasty like that
			vc_composite::new_dfs();
			// item is a string that we are gonna pump out
			// raw, without any structuring
			char *buf = ostrm.out_want(item.len());
			if(!buf)
			{
				ostrm.close(vcxstream::DISCARD);
				RAISEABORT(generic_problem, vcnil);
			}
			memcpy(buf, (const char *)item, item.len());
		}
		else
		{
			if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
			{
				RAISEABORT(generic_problem, vcnil);
			}
		}
		// try pushing it out once now
		switch(ostrm.flushnb())
		{
		case NB_DONE:
			// this gets the chit table discarded so open/close
			// pairs-up properly for graph cycle computations
			if(!ostrm.close(vcxstream::DISCARD))
			{
				RAISEABORT(generic_problem, vcnil);
			}
			return vctrue;
		case NB_RETRY:
			// when we return retry, it is up to the caller
			// to call the crank function to continue to
			// try to send the remaining data
			if(!ostrm.close(vcxstream::RETRY))
			{
				RAISEABORT(generic_problem, vcnil);
			}
			return retry;
		case NB_ERROR:
			ostrm.close(vcxstream::DISCARD);
			RAISEABORT(generic_problem, vcnil);
		}

	}
	else
	{
	if(!ostrm.open(vcxstream::WRITEABLE/*, vcxstream::ATOMIC*/))
	{
		RAISEABORT(generic_problem, vcnil);
    }
    vc_composite::new_dfs();
	char *buf = ostrm.out_want(item.len());
	if(!buf)
	{
		ostrm.close(vcxstream::DISCARD);
		RAISEABORT(bad_send_raw, vcnil); // fix this
	}
	memcpy(buf, (const char *)item, item.len());
	if(!ostrm.flush())
	{
		ostrm.close(vcxstream::DISCARD);
		RAISEABORT(bad_send_raw, vcnil); // fix this
	}
	ostrm.close();
	return vctrue;
	}
	return vcnil;
}

vc
vc_winsock::socket_recv_buf(vc& item, int to_get, const vc& recv_info, vc& addr_info)
{
	if(!init())
		return vcnil;
    addr_info = peer_addr;

	vcxstream *is;
	vcxstream isl(this);
	if(buffered)
		is = &vcxr;
	else
		is = &isl;
	vcxstream& istrm = *is;

	if(buffered)
	{
	if(istrm.get_status() != vcxstream::READABLE)
	{
		if(!istrm.open(vcxstream::READABLE, 
			smode == VC_NONBLOCKING ? vcxstream::ATOMIC : vcxstream::MULTIPLE))
		{
			RAISEABORT(generic_problem, vcnil);
		}
	}
	char *buf = istrm.in_want(to_get);
	if(!buf)
	{
		static vc wouldblock("wouldblock");
		static vc retry("retry");
		if(smode == VC_NONBLOCKING &&
			error == wouldblock)
		{
			istrm.close(vcxstream::RETRY);
			return retry;
		}
		else
			istrm.close(vcxstream::DISCARD);
		return vcnil;
	}
	vc ret(VC_BSTRING, buf, to_get);
	item = ret;
	// note: this implies that per-send is how cycles are
	// computed 
	istrm.chit_new_table();
	istrm.commit();
	}
	else
	{
	if(!istrm.open(vcxstream::READABLE))
	{
		RAISEABORT(generic_problem, vcnil);
    }
	char *buf = istrm.in_want(to_get);
	if(!buf)
	{
		istrm.close(vcxstream::DISCARD);
		return vcnil;
	}
	vc ret(VC_BSTRING, buf, to_get);
	item = ret;
	istrm.close();
	}

	return vctrue;
}
