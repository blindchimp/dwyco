#if 0
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#ifdef VCSOCK

#ifdef USE_WINSOCK
#include <winsock.h>
#endif

#ifdef USE_BERKSOCK
#include "vcberk.h"
#endif

#include "dwvecp.h"
#include "vcwsioh.h"
#include "vcwsock.h"

#ifdef LINUX
#define USE_POLL
#endif

//static char Rcsid[] = "$Header: /e/linux/home/dwight/repo/vc/rcs/vcwsioh.cpp 1.50 1999/03/17 14:57:04 dwight Exp $";

VcWinsockIOH::VcWinsockIOH(vc_winsock *v)
	: vcsockp(v), vcxr(v, 0, 2048, vcxstream::CONTINUOUS_READAHEAD), vcxs(v)
{
}

VcWinsockIOH::~VcWinsockIOH()
{
}


int
VcWinsockIOH::can_write()
{
	vcsockp->status |= VC_SOCK_WRITE;
	return 0;
}

int
VcWinsockIOH::can_read()
{
	vcsockp->status |= VC_SOCK_READ;
	return 0;
}

int
VcWinsockIOH::has_exception()
{
	vcsockp->status |= VC_SOCK_ERROR;
	return 0;
}


VcWinsockDispatcher::VcWinsockDispatcher()
{

}

VcWinsockDispatcher::~VcWinsockDispatcher()
{
}

// this is only used just before doing an exec
// so it doesn't bother trying to keep the state
// of the system intact
void
VcWinsockDispatcher::close_all_but(SOCKET tosave)
{
	DwIOHandlerListIter i(&handlers);
	for(; !i.eol(); i.forward())
	{
		VcWinsockIOH *ioh = (VcWinsockIOH *)i.get();
		SOCKET s = ioh->vcsockp->sock;
		if(s != INVALID_SOCKET && s != tosave)
		{
#ifdef USE_WINSOCK
			closesocket(s);
#else
			::close(s);
#endif
		}
	}
}

vc_winsock *
VcWinsockDispatcher::os_socket_to_vc(SOCKET os_sock)
{
	DwIOHandlerListIter i(&handlers);
	for(; !i.eol(); i.forward())
	{
		VcWinsockIOH *ioh = (VcWinsockIOH *)i.get();
		SOCKET s = ioh->vcsockp->sock;
		if(s != INVALID_SOCKET && s == os_sock)
		{
			return ioh->vcsockp;
		}
	}
	return 0;
}

#ifndef USE_POLL
int
VcWinsockDispatcher::poll(int whatfor, int sec, int usec)
{
	fd_set rset;
	fd_set wset;
	fd_set eset;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);

	int dont_block = 0;
	DwIOHandlerListIter i(&handlers);
	for(; !i.eol(); i.forward())
	{
		VcWinsockIOH *ioh = (VcWinsockIOH *)i.get();
		ioh->vcsockp->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
		SOCKET s = ioh->vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			if(whatfor & VC_SOCK_READ)
			{
#if 0
// see comment below in poll2
				if(ioh->vcxr.has_input())
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
		VcWinsockIOH *ioh = (VcWinsockIOH *)i.get();
		SOCKET s = ioh->vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			int did_append = 0;
			if((whatfor & VC_SOCK_READ) && (FD_ISSET(s, &rset) || 
					ioh->vcxr.has_input()))
			{
				ioh->can_read();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}

			if((whatfor & VC_SOCK_WRITE) && FD_ISSET(s, &wset))
			{
				ioh->can_write();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}

			if((whatfor & VC_SOCK_ERROR) && FD_ISSET(s, &eset))
			{
				ioh->has_exception();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}
		}
	}
	return vc_winsock::get_poll_results()->num_elems();
}

//
// whatfor now means:
// whatfor & VC_SOCK_READ = poll all sockets for read
// whatfor & VC_SOCK_WRITE = poll all for write
// whatfor & VC_SOCK_ERR = poll all for error
// otherwise, use the read/write sets to figure out what to
// poll for.
int
VcWinsockDispatcher::poll2(int whatfor, int sec, int usec)
{
	fd_set rset;
	fd_set wset;
	fd_set eset;

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);

	int dont_block = 0;
	DwIOHandlerListIter dhi(&handlers);
	if(whatfor != 0)
	{
		for(; !dhi.eol(); dhi.forward())
		{
			VcWinsockIOH *ioh = (VcWinsockIOH *)dhi.get();
			ioh->vcsockp->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
			SOCKET s = ioh->vcsockp->sock;
			if(s != INVALID_SOCKET)
			{
				if(whatfor & VC_SOCK_READ)
				{
#if 0
// see comment below in poll2
					if(ioh->vcxr.has_input())
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
				VcWinsockIOH *ioh = &vcw->ioh;
				vcw->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
				if(ioh->vcxr.has_input())
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
		VcWinsockIOH *ioh = (VcWinsockIOH *)dhi.get();
		SOCKET s = ioh->vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			int did_append = 0;
			if(FD_ISSET(s, &rset) || ioh->vcxr.has_input())
			{
				ioh->can_read();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}

			if(FD_ISSET(s, &wset))
			{
				ioh->can_write();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}

			if(FD_ISSET(s, &eset))
			{
				ioh->has_exception();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}
		}
	}

	return vc_winsock::get_poll_results()->num_elems();
}
#else
#include <sys/poll.h>

int
VcWinsockDispatcher::poll(int whatfor, int sec, int usec)
{
	int dont_block = 0;
	int nfds = handlers.num_elems();
	struct pollfd *pfds = new struct pollfd[nfds];
	int pfd_count = 0;
	
	DwIOHandlerListIter i(&handlers);
	for(; !i.eol(); i.forward())
	{
		VcWinsockIOH *ioh = (VcWinsockIOH *)i.get();
		ioh->vcsockp->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
		SOCKET s = ioh->vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			pfds[pfd_count].fd = s;
			pfds[pfd_count].events = 0;
			if(whatfor & VC_SOCK_READ)
			{
#if 0
//see comment below in poll2
				if(ioh->vcxr.has_input())
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

	int num = ::poll(pfds, pfd_count, sec * 1000 + usec / 1000);

	if(num == SOCKET_ERROR)
	{
		delete [] pfds;
		return -1;
	}

	pfd_count = 0;
	for(i.rewind(); !i.eol(); i.forward())
	{
		VcWinsockIOH *ioh = (VcWinsockIOH *)i.get();
		SOCKET s = ioh->vcsockp->sock;
		if(s != INVALID_SOCKET)
		{
			int did_append = 0;
			if((whatfor & VC_SOCK_READ) && ((pfds[pfd_count].revents & (POLLIN|POLLERR|POLLHUP)) || 
					ioh->vcxr.has_input()))
			{
				ioh->can_read();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}

			if((whatfor & VC_SOCK_WRITE) && (pfds[pfd_count].revents & (POLLOUT|POLLERR|POLLHUP)))
			{
				ioh->can_write();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}

			if((whatfor & VC_SOCK_ERROR) && (pfds[pfd_count].revents & (POLLPRI|POLLERR|POLLHUP)))
			{
				ioh->has_exception();
				if(!did_append)
				{
					vc_winsock::get_poll_results()->append(ioh->vcsockp);
					did_append = 1;
				}
			}
			++pfd_count;
		}
	}
	delete [] pfds;
	return vc_winsock::get_poll_results()->num_elems();
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
VcWinsockDispatcher::poll2(int whatfor, int sec, int usec)
{
	int nfds = handlers.num_elems();
	// note: can't really tell ahead, so just allocate big
	struct pollfd *pfds = new struct pollfd[nfds * 3];
	int pfd_count = 0;

	int dont_block = 0;
	DwIOHandlerListIter dhi(&handlers);
	if(whatfor != 0)
	{
		for(; !dhi.eol(); dhi.forward())
		{
			VcWinsockIOH *ioh = (VcWinsockIOH *)dhi.get();
			ioh->vcsockp->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
			SOCKET s = ioh->vcsockp->sock;
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
					if(ioh->vcxr.has_input())
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
					if(!ss->contains(ioh->vcsockp))
						++pfd_count;
				}
				else if(whatfor & VC_SOCK_WRITE_NOT_READ)
				{
					SocketSet *ss = vc_winsock::get_read_set();
					// note: if write set contains this socket,
					// we don't increment pfd_count, so the socket
					// is effectively deleted from the poll set.
					if(!ss->contains(ioh->vcsockp))
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
				VcWinsockIOH *ioh = &vcw->ioh;
				vcw->status &= ~(VC_SOCK_READ|VC_SOCK_WRITE|VC_SOCK_ERROR);
				if(ioh->vcxr.has_input())
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

	int num = ::poll(pfds, pfd_count, sec * 1000 + usec / 1000);

	if(num == SOCKET_ERROR)
	{
		delete [] pfds;
		return -1;
	}

	pfd_count = 0;
	if(whatfor != 0)
	{
		for(dhi.rewind(); !dhi.eol(); dhi.forward())
		{
			VcWinsockIOH *ioh = (VcWinsockIOH *)dhi.get();
			SOCKET s = ioh->vcsockp->sock;
			if(s != INVALID_SOCKET)
			{
				int did_append = 0;
				if((whatfor & VC_SOCK_READ) && ((pfds[pfd_count].revents & (POLLIN|POLLERR|POLLHUP)) || 
						ioh->vcxr.has_input()))
				{
					if(1 || !(whatfor & VC_SOCK_READ_NOT_WRITE) ||
						!vc_winsock::get_write_set()->contains(ioh->vcsockp))
					{
						ioh->can_read();
						if(!did_append)
						{
							vc_winsock::get_poll_results()->append(ioh->vcsockp);
							did_append = 1;
						}
					}
				}

				if((whatfor & VC_SOCK_WRITE) && (pfds[pfd_count].revents & (POLLOUT|POLLERR|POLLHUP)))
				{
					if(1|| !(whatfor & VC_SOCK_WRITE_NOT_READ) ||
						!vc_winsock::get_read_set()->contains(ioh->vcsockp))
					{
						ioh->can_write();
						if(!did_append)
						{
							vc_winsock::get_poll_results()->append(ioh->vcsockp);
							did_append = 1;
						}
					}
				}

				if((whatfor & VC_SOCK_ERROR) && (pfds[pfd_count].revents & (POLLPRI|POLLERR|POLLHUP)))
				{
					ioh->has_exception();
					if(!did_append)
					{
						vc_winsock::get_poll_results()->append(ioh->vcsockp);
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
					if(!ss->contains(ioh->vcsockp))
						++pfd_count;
				}
				else if(whatfor & VC_SOCK_WRITE_NOT_READ)
				{
					SocketSet *ss = vc_winsock::get_read_set();
					// note: if write set contains this socket,
					// we don't increment pfd_count, so the socket
					// is effectively deleted from the poll set.
					if(!ss->contains(ioh->vcsockp))
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
				VcWinsockIOH *ioh = &vcw->ioh;
				if(((pfds[pfd_count].revents & (POLLIN|POLLERR|POLLHUP)) || 
						ioh->vcxr.has_input()) && !(vcw->status & VC_SOCK_READ))
				{
					ioh->can_read();
					if(!(vcw->status & (VC_SOCK_WRITE|VC_SOCK_ERROR)))
						vc_winsock::get_poll_results()->append(ioh->vcsockp);
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
				VcWinsockIOH *ioh = &vcw->ioh;
				if((pfds[pfd_count].revents & (POLLOUT|POLLERR|POLLHUP)) &&
					!(vcw->status & VC_SOCK_WRITE))
				{
					ioh->can_write();
					if(!(vcw->status & (VC_SOCK_READ|VC_SOCK_ERROR)))
						vc_winsock::get_poll_results()->append(ioh->vcsockp);
				}
				++pfd_count;
			}
		}
	}
	delete [] pfds;
	return vc_winsock::get_poll_results()->num_elems();
}
#endif
#endif
#endif
