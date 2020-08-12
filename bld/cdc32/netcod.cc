
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCO_CDC_LIBUV
//
// primitive networking support using xstrms and vc sockets.
// note: this stuff is screaming for exceptions, but we don't got it
// yet in enough compilers to use it... sigh.

static char RcsId[] = "$Header: g:/dwight/repo/cdc32/rcs/netcod.cc 1.27 1999/01/10 16:09:37 dwight Checkpoint $";
#include <windows.h>
#include <string.h>
#include "vc.h"
#include "netcod.h"
#include "vcwsock.h"
#include "matcom.h"
#include "vcxstrm.h"
#include "sleep.h"
#include "dwlog.h"
#include "dwstr.h"
#include "mmchan.h"
#include "dwrtlog.h"
#include "qauth.h"
#include "ta.h"
#include "dwqbm.h"
using namespace dwyco;

static DwQueryByMember<SimpleSocket> SSQbm;

vc LocalIP;

static vc Wouldblock("wouldblock");
static vc Resumable("resumable");

int FrameSocket::dummy;
int FrameSocket::always_zero;
static int
net_socket_error(vc *v)
{
    static int lock;
    if(lock)
        return VC_SOCKET_BACKOUT;
    if(!lock)
    {
        lock = 1;
        vc_socket *vs = (vc_socket *)v;
        GRTLOG("backout socket error", 0, 0);
        GRTLOGVC(vs->socket_error_desc());
// socket-*addr actually calls more winsock functions, so may
// cause errors to be cleared or recurse in here.
//GRTLOGVC(vs->socket_local_addr());
//GRTLOGVC(vs->socket_peer_addr());
        lock = 0;
        return VC_SOCKET_BACKOUT;
    }
    else
        oopanic("recursive error");
    // NOTREACHED
    return VC_SOCKET_BACKOUT;
}

static int
wouldblock_handler(vc *v)
{
    vc_socket *vs = (vc_socket *)v;
    GRTLOG("wouldblock handler %p %d", vs, 0);
    if(vs->socket_error() == Wouldblock)
    {
        ++vs->retries;
        if(vs->retries > vs->max_retries)
        {
            GRTLOG("max retry", 0, 0);
            return VC_SOCKET_BACKOUT;
        }
        dwsleep(1, 0, 1);
        GRTLOG("resume", 0, 0);
        return VC_SOCKET_RESUME;
    }
    GRTLOG("error", 0, 0);
    GRTLOGVC(vs->socket_error());
    return VC_SOCKET_BACKOUT;
}


static int
resume_error(vc *v)
{
    if(v->socket_error() == Resumable)
        return VC_SOCKET_RESUME;
    return VC_SOCKET_BACKOUT;
}

int
SimpleSocket::any_waiting_for_write()
{
    return SSQbm.exists_by_fun(&SimpleSocket::waiting_for_write, 1);
}

SimpleSocket::SimpleSocket()
{
    noblock = 0;
    istream = 0;
    ostream = 0;
    polling_for_connect = 0;
    SSQbm.add(this);
}

// note: socket must already have been inited and
// connected before calling this.
SimpleSocket::SimpleSocket(vc sock)
{
    noblock = 0;
    istream = 0;
    ostream = 0;
    polling_for_connect = 0;
    this->sock = sock;

#ifdef _Windows
    HWND h;
    if(MMChannel::get_main_window_callback &&
            (h = (*MMChannel::get_main_window_callback)(0)))
    {
        sock.socket_set_option(VC_WSAASYNC_SELECT,
                               (unsigned long)h,
                               WM_USER + 400, FD_READ|FD_WRITE|FD_CLOSE/*|FD_CONNECT*/);
    }
#endif

    peer = sock.socket_peer_addr();
    laddr = sock.socket_local_addr();
    SSQbm.add(this);
}

SimpleSocket::~SimpleSocket()
{
    SSQbm.del(this);
    delete ostream;
    ostream = 0;
    delete istream;
    istream = 0;
    exit();
}

void
SimpleSocket::initsock()
{
    sock = vc(VC_SOCKET_STREAM);
    if(!istream)
        istream = new vcxstream(sock);
    if(!ostream)
        ostream = new vcxstream(sock);
}

int
SimpleSocket::wouldblock()
{
    if(sock.is_nil())
        return 0;
    return sock.socket_error() == Wouldblock;
}

int
SimpleSocket::waiting_for_write()
{
    if(sock.is_nil())
        return 0;
    if(ostream->get_status() == vcxstream::RETRYING)
        return 1;
    if(polling_for_connect)
        return 1;
    return 0;
}


DwString SimpleSocket::local_addr()
{
    return (const char *)laddr;
}


DwString SimpleSocket::peer_addr()
{
    return (const char *)peer;
}

int
SimpleSocket::non_blocking(int nb)
{
    noblock = !!nb;
    if(!sock.is_nil())
        sock.socket_set_option(noblock ? VC_NONBLOCKING: VC_BLOCKING);
    return 1;
}

int
SimpleSocket::reconnect(const char *remote_addr)
{
    oopanic("can't reconnect reliable sockets");
    return 0;
}

int
SimpleSocket::init(const char *remote_addr, const char *local_addr, int retry, HWND hwnd)
{
    if(!retry)
        initsock();
    sock.set_err_callback(net_socket_error);

    if(local_addr == 0)
        local_addr = "any:any";
    if(!retry)
    {
        if(sock.socket_init(local_addr, 0, 0).is_nil())
        {
            bad_op();
            return 0;
        }
        sock.socket_set_option(noblock ? VC_NONBLOCKING: VC_BLOCKING);
        //sock.socket_set_option(VC_SET_TCP_NO_DELAY, 1);
        HWND h;
        if(MMChannel::get_main_window_callback &&
                (h = (*MMChannel::get_main_window_callback)(0)))
        {
#ifdef _Windows
            sock.socket_set_option(VC_WSAASYNC_SELECT,
                                   (unsigned long)h,
                                   WM_USER + 400, FD_READ|FD_WRITE|FD_CLOSE/*|FD_CONNECT*/);
#endif
        }
        // ug, hack
        // don't connect if the remote addr isn't speced (maybe a listening
        // udp socket, for example)
        if(remote_addr && strlen(remote_addr) >= 3 && strncmp(remote_addr, "any", 3) != 0)
        {
            vc vcra(remote_addr);
            if(sock.socket_connect(vcra).is_nil())
            {
                GRTLOG("netcod first init connect failed (wb %d)", wouldblock(), 0);
                GRTLOGVC(vcra);
                if(!wouldblock())
                    bad_op();
                polling_for_connect = 1;
                return 0;
            }
        }
        else
        {
            // no peer address to be had since we didn't connect
            laddr = sock.socket_local_addr();
            return 1;
        }
    }
    else
    {
#if 0
        // note: if sock is non-blocking, and return from
        // underlying connect is EISCONN, this connect
        // should return non-nil, indicating that the
        // connection is established. hopefully, other
        // error codes are reflected properly if the
        // connection couldn't go through.
        vc vcra(remote_addr);
        VC_ERR_CALLBACK cb = sock.set_err_callback(resume_error);
        if(sock.socket_connect(vcra).is_nil())
        {
            GRTLOG("netcod init connect failed (wb %d)", wouldblock(), 0);
            GRTLOGVC(vcra);
            sock.set_err_callback(cb);
            if(!wouldblock())
                bad_op();
            return 0;
        }
        sock.set_err_callback(cb);
#endif
        // per winsock spec, check writeable
        // to see if connection is done.
        int a = sock.socket_poll(VC_SOCK_WRITE|VC_SOCK_ERROR, 0, 0);
        if(a == -1)
        {
            bad_op();
            return 0;
        }
        if(a & VC_SOCK_ERROR)
        {
            bad_op();
            return 0;
        }
        if(a != VC_SOCK_WRITE)
        {
            // pseudo wouldblock
            sock.socket_set_error("wouldblock");
            return 0;
        }
        // horrible hack, fix this XXX
        // fix is: when socket_connect is called, all the
        // stuff should be set provisionally correct even
        // for non-blocking sockets.
        //vc_winsock *vcs = (vc_winsock *)sock.nonono();
        //vcs->peer_addr = remote_addr;
        //vcs->socket_local_addr();
        //vcs->used_connect = 1;
    }
    peer = sock.socket_peer_addr();
    laddr = sock.socket_local_addr();
    if(LocalIP.is_nil())
    {
        DwString a((const char *)laddr);
        int i;
        if((i = a.find(":")) != DwString::npos)
        {
            a.erase(i);
        }
        LocalIP = a.c_str();
        GRTLOG("LocalIP = %s", (const char *)LocalIP, 0);
    }
    GRTLOG("simple peer (local, peer)", 0, 0);
    GRTLOGVC(laddr);
    GRTLOGVC(peer);
    polling_for_connect = 0;
    return 1;
}

void
SimpleSocket::bad_op()
{
    if(!sock.is_nil())
    {
        last_error = sock.socket_error_desc();
        GRTLOG("bad op", 0, 0);
        GRTLOGVC(sock);
        GRTLOGVC(last_error);
        sock = vcnil;
    }
    polling_for_connect = 0;
}

void
SimpleSocket::exit()
{
    if(!sock.is_nil())
    {
        last_error = sock.socket_error_desc();
        sock.socket_close(0);
        sock = vcnil;
    }
}

int
SimpleSocket::can_write()
{
    if(sock.is_nil())
        return 1; // note: this is bogus, like bad_op below, but not much we can do with now
    int a;
    if((a = sock.socket_poll(VC_SOCK_WRITE, 0, 0)) == VC_SOCK_WRITE)
        return 1;
    if(a == -1)
    {
        bad_op();
        return 1; // attempt to clue in upper layers indirectly...
    }
    return 0;
}

int
SimpleSocket::has_data(int sec, int usec)
{
    if(sock.is_nil())
        return 1; // see comment in can_write
    int a;
    if((a = sock.socket_poll(VC_SOCK_READ, sec, usec)) == VC_SOCK_READ)
        return 1;
    if(a == -1)
    {
        bad_op();
        return 1; // see comment in can_write
    }
    return 0;
}

int
SimpleSocket::active()
{
    // doesn't say anything about whether socket is
    // working or not...
    return !sock.is_nil();
}


#define DECL_WBHANDLE VC_ERR_CALLBACK cb;

#define WBHANDLE2  \
	cb = sock.set_err_callback(net_socket_error);

#define WBHANDLE  \
	cb = sock.set_err_callback(wouldblock_handler); \
   {\
	vc_socket *v = (vc_socket *)sock.nonono(); \
	v->retries = 0; \
	v->max_retries = 0; \
	}

#define RET(a) {if(sock.type() == VC_SOCKET) sock.set_err_callback(cb); return (a);}

int
SimpleSocket::sendlc(DWBYTE *buf, int len, int wbok)
{
    if(sock.is_nil())
        return 0;
    DECL_WBHANDLE
    if(wbok)
    {
        WBHANDLE2
    }
    else
    {
        WBHANDLE
    }
    //vcxstream ostrm(sock, 0, len + sizeof(long));
    vcxstream& ostrm = *ostream;
    if(sock.socket_get_mode() == VC_NONBLOCKING)
    {
        if(ostrm.get_status() != vcxstream::RETRYING)
        {
            if(ostrm.get_status() != vcxstream::WRITEABLE)
            {
                if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
                    RET(0);
            }

            char *obuf;
            if((obuf = ostrm.out_want(sizeof(uint32_t))) == 0)
                RET(0);
            *(uint32_t *)obuf = htonl(len);
            if((obuf = ostrm.out_want(len)) == 0)
                RET(0);
            memmove(obuf, buf, len);
        }
        else
        {
            if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
                RET(0);
        }
        switch(ostrm.flushnb())
        {
        case NB_DONE:
            if(!ostrm.close(vcxstream::DISCARD))
                RET(0);
            RET(1);
        case NB_RETRY:
            if(!ostrm.close(vcxstream::RETRY))
                RET(0);
            // assume socket has wouldblock pending
            RET(0);
        case NB_ERROR:
            ostrm.close(vcxstream::DISCARD);
            RET(0);
        default:
            oopanic("bad flushnb return");
        }
    }
    else // BLOCKING
    {
        if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::MULTIPLE))
            RET(0);
        char *obuf;
        if((obuf = ostrm.out_want(sizeof(uint32_t))) == 0)
            RET(0);
        *(uint32_t *)obuf = htonl(len);
        if((obuf = ostrm.out_want(len)) == 0)
            RET(0);
        memmove(obuf, buf, len);
        if(!ostrm.close(vcxstream::FLUSH))
            RET(0);
    }
    RET(1);
}

int
SimpleSocket::recvlc(DWBYTE*& buf, int& len)
{
    if(sock.is_nil())
        return 0;
    DECL_WBHANDLE
    WBHANDLE
    //vcxstream istrm(sock);
    vcxstream& istrm = *istream;
    if(!istrm.open(vcxstream::READABLE, vcxstream::ATOMIC))
        RET(0);

    char *b;
    if((b = istrm.in_want(sizeof(uint32_t))) == 0)
    {
        //VcError << "recvlc error " << (const char *)sock.socket_error() << "\n";
        if(wouldblock())
            istrm.close(vcxstream::RETRY);
        else
            bad_op();
        RET(0);
    }
    int in_len = (int)ntohl(*(uint32_t *)b);
    if(in_len < 0)
    {
        bad_op();
        RET(0);
    }
    if((b = istrm.in_want(in_len)) == 0)
    {
        //VcError << "recvlc error " << (const char *)sock.socket_error() << "\n";
        if(wouldblock())
            istrm.close(vcxstream::RETRY);
        else
            bad_op();
        RET(0);
    }

    DWBYTE *b2 = new DWBYTE[in_len];
    memmove(b2, b, in_len);
    buf = b2;
    if(!istrm.close(vcxstream::FLUSH))
    {
        bad_op();
        RET(0);
    }
    len = in_len;
    RET(1);
}


int
SimpleSocket::sendvc(vc v)
{
    if(sock.is_nil())
        return 0;
    int len;
    DECL_WBHANDLE
    WBHANDLE2
    vcxstream& ostrm = *ostream;
    GRTLOG("sendvc ", 0, 0);
    GRTLOGVC(v);
    if(sock.socket_get_mode() == VC_NONBLOCKING)
    {
        if(ostrm.get_status() != vcxstream::RETRYING)
        {
            GRTLOG("first", 0, 0);
            if(ostrm.get_status() != vcxstream::WRITEABLE)
            {
                if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
                    RET(0);
                GRTLOG("atomic", 0, 0);
            }
            vc_composite::new_dfs();
            if((len = v.xfer_out(ostrm)) < 0)
                RET(0);
        }
        else
        {
            if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
                RET(0);
            GRTLOG("retry", 0, 0);
        }
        switch(ostrm.flushnb())
        {
        case NB_DONE:
            GRTLOG("flush nb done", 0, 0);
            if(!ostrm.close(vcxstream::DISCARD))
                RET(0);
            RET(1);
        case NB_RETRY:
            GRTLOG("flush nb retry", 0, 0);
            if(!ostrm.close(vcxstream::RETRY))
                RET(0);
            // assume socket has wouldblock pending
            RET(0);
        case NB_ERROR:
            GRTLOG("flush nb error", 0, 0);
            ostrm.close(vcxstream::DISCARD);
            RET(0);
        default:
            oopanic("bad flushnb return2");
        }
    }
    else // BLOCKING
    {
        //vcxstream ostrm(sock);
        if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
            RET(0);
        vc_composite::new_dfs();
        if((len = v.xfer_out(ostrm)) < 0)
            RET(0);
        if(!ostrm.close(vcxstream::FLUSH))
            RET(0);
    }

#if 0
    vc f(VC_FILE);
    f.open("ser.out", VCFILE_WRITE);
    vcxstream tstrm(f);
    if(!tstrm.open(vcxstream::WRITEABLE))
        return 0;
    vc_composite::new_dfs();
    if((len = v.xfer_out(tstrm)) < 0)
        return 0;
    if(!tstrm.close(vcxstream::FLUSH))
        return 0;
    f.close();
#endif
    return len;
}

int
SimpleSocket::recvvc(vc& v)
{
    if(sock.is_nil())
        return 0;
    int len;
    DECL_WBHANDLE
    WBHANDLE
    //vcxstream istrm(sock);
    vcxstream& istrm = *istream;
    if(!istrm.open(vcxstream::READABLE, vcxstream::ATOMIC))
        RET(0);
    if((len = v.xfer_in(istrm)) < 0)
    {
        switch(len)
        {
        case EXIN_DEV:
            if(wouldblock())
                istrm.close(vcxstream::RETRY);
            break;
        default:
        case EXIN_PARSE:
            bad_op();
            // may have provoked a bug... 3/6/2003
            //istrm.close(vcxstream::DISCARD);
            return 0;
        }

        RET(0);
    }
    if(!istrm.close(vcxstream::FLUSH))
        RET(0);
    RET(len);
}


int
Listener::init(const char *, const char *local_addr, int, HWND)
{
    initsock();
    sock.set_err_callback(net_socket_error);
    if(local_addr == 0)
        local_addr = "any:any";
    if(sock.socket_init(local_addr, 1, 1).is_nil())
    {
        bad_op();
        return 0;
    }
    sock.socket_set_option(noblock ? VC_NONBLOCKING: VC_BLOCKING);
    laddr = sock.socket_local_addr();
    return 1;
}

int
Listener::init2(const char *local_addr)
{
    initsock();
    sock.set_err_callback(net_socket_error);
    if(local_addr == 0)
        local_addr = "any:any";
    if(sock.socket_init(local_addr, 1, 1).is_nil())
    {
        bad_op();
        return 0;
    }
    sock.socket_set_option(noblock ? VC_NONBLOCKING: VC_BLOCKING);
    laddr = sock.socket_local_addr();
    return 1;
}

int
Listener::accept_ready()
{
    return has_data();
}

SimpleSocket *
Listener::accept()
{
    // sock is socket that has been determined to be
    // ready for an accept call...
    if(sock.is_nil())
        return 0;
    vc tmp;
    vc newsock = sock.socket_accept(tmp);
    if(newsock.is_nil())
    {
        bad_op();
        return 0;
    }
    HWND h;
    if(MMChannel::get_main_window_callback &&
            (h = (*MMChannel::get_main_window_callback)(0)))
    {
#ifdef _Windows
        newsock.socket_set_option(VC_WSAASYNC_SELECT,
                                  (unsigned long)h,
                                  WM_USER + 400, FD_READ|FD_WRITE|FD_CLOSE/*|FD_CONNECT*/);
#endif
    }
    SimpleSocket *s = new SimpleSocket;
    s->sock = newsock;
    s->peer = tmp;
    s->laddr = newsock.socket_local_addr();
    s->istream = new vcxstream(s->sock);
    s->ostream = new vcxstream(s->sock);
    GRTLOG("accept simple (local, peer)", 0, 0);
    GRTLOGVC(s->laddr);
    GRTLOGVC(s->peer);
    return s;
}


//FSVec FrameSocket::Sockets;


FrameSocket::FrameSocket(unsigned int len)
{
#if 0
#ifdef _Windows
    if(vc_winsock::wsa_data.iMaxUdpDg != 0
            && len > vc_winsock::wsa_data.iMaxUdpDg)
        len = vc_winsock::wsa_data.iMaxUdpDg;
#endif
#endif
    packet_buf = new DWBYTE[len];
    plen = len;

    seq_send = 0;
    seq_recv = 0;
    reason_seq_bad = 0;
    packets_lost = 0;
    sent = 0;
    recvd = 0;
}

FrameSocket::FrameSocket(vc sock, unsigned int len) :
    SimpleSocket(sock)
{
#if 0
#ifdef _Windows
    if(vc_winsock::wsa_data.iMaxUdpDg != 0
            && len > vc_winsock::wsa_data.iMaxUdpDg)
        len = vc_winsock::wsa_data.iMaxUdpDg;
#endif
#endif
    packet_buf = new DWBYTE[len];
    plen = len;

    seq_send = 0;
    seq_recv = 0;
    reason_seq_bad = 0;
    packets_lost = 0;
    sent = 0;
    recvd = 0;
}

FrameSocket::~FrameSocket()
{
    delete [] packet_buf;
}


void
FrameSocket::initsock()
{
    sock = vc(VC_SOCKET_DGRAM);
}

int
FrameSocket::can_write()
{
    if(sock.is_nil())
        return 0;
    // note: can't count on write indicator from
    // socket stack to actually decide if the packet
    // is going to be dropped (many stacks must wire
    // this to true for UDP sockets and immediately
    // drop packets if something goes wrong.)
    //

    int a;
    if((a = sock.socket_poll(VC_SOCK_WRITE, 0, 0)) == VC_SOCK_WRITE)
        return 1;
    if(a == -1)
    {
        bad_op();
        return 1; // attempt to clue in upper layers indirectly...
    }
    return 0;
}

// note: channel handling only implemented
// for non-fragmenting cases...
int
FrameSocket::send(DWBYTE *buf, int len, int chan, int user_byte, int user_len, int user_seq)
{
    if(sock.is_nil())
        return 0;
    GRTLOGA("framesocket::send len %d chan %d ub %d ul %d useq %d", len, chan, user_byte, user_len, user_seq);
    GRTLOGVC(laddr);
    GRTLOGVC(peer);
    vcxstream ostrm(sock, (char *)packet_buf, plen, vcxstream::FIXED);
    if(!ostrm.open(vcxstream::WRITEABLE))
        return 0;

    char *obuf;
    if(!send_seq(ostrm))
        return 0;
    vc vlen(len);
    if(vlen.xfer_out(ostrm) < 0)
        return 0;
    obuf = ostrm.out_want(len + 1 + user_len + (user_seq != 0 ? 4 : 0));
    if(!obuf)
        return 0;
    *obuf = chan;
    switch(user_len)
    {
    case 1:
        *(obuf + 1) = user_byte;
        break;
    case 2:
        *(obuf + 1) = (user_byte >> 8);
        *(obuf + 2) = user_byte;
        break;
    case 4:
        *(obuf + 1) = user_byte >> 24;
        *(obuf + 2) = user_byte >> 16;
        *(obuf + 3) = user_byte >> 8;
        *(obuf + 4) = user_byte;
        break;
    default:
        oopanic("frame send bad userlen");
    }

    memmove(obuf + 1 + user_len, buf, len);
    if(user_seq != 0)
    {
        int a;
        a = htonl(user_seq);
        memmove(obuf + 1 + user_len + len, (char *)&a, 4);
    }

    if(!ostrm.close(vcxstream::FLUSH))
        return 0;
    ++sent;
    return 1;
}


int
FrameSocket::recv(DWBYTE*& buf, int& len, int& chan, int& user_out, int len_out, int& user_seq_out)
{
    if(sock.is_nil())
        return 0;
    vc in_len;

    vcxstream istrm(sock, (char *)packet_buf, plen, vcxstream::FIXED);
    if(!istrm.open(vcxstream::READABLE))
        return 0;

    // note: since packets are complete
    // units of one frame, we don't need to
    // check for in-sequence packets (i suppose
    // you could just drop out of seq frames, but
    // instead we just use the frame as is.)
    int dum;
    if(!recv_seq(istrm, dum))
        return 0;

    if(in_len.xfer_in(istrm) < 0)
        return 0;

    char *b;
    long il = in_len;
    // +2 for chan and user_byte
    if((b = istrm.in_want(il + 1 + len_out)) == 0)
        return 0;

    DWBYTE *b2 = new DWBYTE[il];
    memmove(b2, b + 1 + len_out, il);
    buf = b2;
    len = il;
    chan = *(unsigned char *)b;
    switch(len_out)
    {
    case 1:
        user_out = *(unsigned char *)(b + 1);
        break;
    case 2:
        user_out = (*(unsigned char *)(b + 1) << 8) | (*(unsigned char *)(b + 2));
        break;
    case 4:
        user_out = (*(unsigned char *)(b + 3) << 8) | (*(unsigned char *)(b + 4)) |
                   (*(unsigned char *)(b + 1) << 24) | (*(unsigned char *)(b + 2) << 16);
        break;
    default:
        oopanic("frame recv bad user len");
    }
    if(user_seq_out != 0)
    {
        if((b = istrm.in_want(4)) == 0)
            return 0;
        int a;
        memmove((char *)&a, b, sizeof(a));
        user_seq_out = ntohl(a);
    }

    if(!istrm.close(vcxstream::FLUSH))
        return 0;
    ++recvd;
    return 1;
}


int
FrameSocket::send_seq(vcxstream& ostrm)
{
    char *obuf;
    if((obuf = ostrm.out_want(1)) == 0)
        return 0;
    *obuf = seq_send++;
    return 1;
}

// this function can return 3 things:
// 0 - can't determine sequence number
// 1 - packet is in sequence
// -1 - packet is out of sequence
int
FrameSocket::recv_seq(vcxstream& istrm, int& seq_out)
{
    char *b;
    if((b = istrm.in_want(1)) == 0)
        return 0;
    unsigned char seq = *(unsigned char *)b;
    seq_out = seq;
    if(seq != seq_recv)
    {
        // estimate number of packets lost
        int lost = seq - seq_recv;
        if(lost < 0)
            lost += 256;
        packets_lost += lost;
        seq_recv = ++seq;
        ++reason_seq_bad;
        return -1;
    }
    else
        seq_recv++;
    return 1;
}

void
FrameSocket::set_packet_counts(int send, int recv)
{
    sent = send;
    recvd = recv;
}

void
FrameSocket::get_packet_counts(int& s, int& r)
{
    s = sent;
    r = recvd;
}

int
FrameSocket::sendvc(vc v)
{
    int len;
    if(sock.is_nil())
        return 0;
    vcxstream ostrm(sock, (char *)packet_buf, plen, vcxstream::FIXED);
    ostream = &ostrm;
    len = SimpleSocket::sendvc(v);
    ostream = 0;
    return len;
}

int
FrameSocket::recvvc(vc& v)
{
    if(sock.is_nil())
        return 0;
    int len;
    vcxstream istrm(sock, (char *)packet_buf, plen, vcxstream::FIXED);
    istream = &istrm;
    len = SimpleSocket::recvvc(v);
    istream = 0;
    peer = sock.socket_peer_addr();
    return len;
}

int
FrameSocket::reconnect(const char *remote_addr)
{
    // this is a bit tricky. *some* implementations
    // claim that you have to "dissolve" any previous
    // connect state by setting to "all 0", or AF_UNSPEC
    // before reconnecting. winsock2 claims you can just
    // call connect again without further problems, and
    // old packets that might be q'd to the previous address
    // will be chucked (which is what we want.) linux man page
    // claims that the "dissolve" functionality is not implemented,
    // but doesn't explicitly say what happens if you call connect
    // without doing the "dissolve". who knows...
    GRTLOG("reconnect (local, peer)", 0, 0);
    GRTLOGVC(laddr);
    GRTLOGVC(peer);
    if(sock.is_nil())
        return 0;
    if(sock.socket_connect(remote_addr).is_nil())
        return 0;
    peer = sock.socket_peer_addr();
    GRTLOG("reconnect ok to %s", remote_addr, 0);
    return 1;
}

#endif
