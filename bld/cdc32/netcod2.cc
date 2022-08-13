
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifdef DWYCO_CDC_LIBUV
//
// primitive networking support using xstrms and vc sockets.
// note: this stuff is screaming for exceptions, but we don't got it
// yet in enough compilers to use it... sigh.

#include <string.h>
#include "vc.h"
#include "vccomp.h"
#include "netcod2.h"
#include "matcom.h"
#include "vcxstrm.h"
#include "dwstr.h"

#include "dwrtlog.h"
#include "ta.h"
#include "dwstr.h"
#include "dwqbm.h"
#ifndef _Windows
#include <arpa/inet.h>
#endif

using namespace dwyco;

static DwQueryByMember<SimpleSocket> SSQbm;

vc LocalIP;

int FrameSocket::dummy;
int FrameSocket::always_zero;

int
SimpleSocket::any_waiting_for_write()
{
    return SSQbm.exists_by_fun(&SimpleSocket::waiting_for_write, 1);
}

SimpleSocket::SimpleSocket()
{
    saw_hard_error = 0;
    q_empty = 1;
    SSQbm.add(this);
}

// note: socket must already have been inited and
// connected before calling this.
SimpleSocket::SimpleSocket(vc sock)
{
    this->sock = sock;
    peer = sock.socket_peer_addr();
    laddr = sock.socket_local_addr();
    saw_hard_error = 0;
    q_empty = 1;
    SSQbm.add(this);
}

SimpleSocket::~SimpleSocket()
{
    SSQbm.del(this);
    exit();
}

void
SimpleSocket::initsock()
{
    sock = vc(VC_UVSOCKET_STREAM);
}

int
SimpleSocket::wouldblock()
{
    // upper levels usually do this:
    // has_data? if yes, try to get it.
    //  if getting data fails, was it wouldblock error?
    //  if yes, ignore and try again later, otherwise close connection.
    //
    // so with the new stuff, we just say if anything comes from the lower
    // level that wasn't a hard error, we call that "wouldblock" to keep the
    // old software happy.
    if(q_empty && !saw_hard_error)
        return 1;
    return 0;
}

int
SimpleSocket::waiting_for_write()
{
    if(sock.is_nil())
        return 0;
    if(sock.socket_get_write_q_size() > 0)
        return 1;
    return 0;
}

DwString SimpleSocket::local_addr()
{
    return DwString((const char *)laddr);
}

DwString SimpleSocket::peer_addr()
{
    return DwString((const char *)peer);
}

int
SimpleSocket::non_blocking(int nb)
{
    return 1;
}

int
SimpleSocket::reconnect(const char *remote_addr)
{
    oopanic("can't reconnect reliable sockets");
    return 0;
}

int
SimpleSocket::init(const char *remote_addr, const char *local_addr, int retry)
{
    if(!retry)
        initsock();

    if(local_addr == 0)
        local_addr = "any:any";
    if(!retry)
    {
        if(sock.socket_init(local_addr, 0, 0).is_nil())
        {
            bad_op();
            return 0;
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
                bad_op();
                return 0;
            }
            if(sock.socket_peer_addr().is_nil())
            {
                // set our peer to be the provisional peer
                peer = remote_addr;
            }
            laddr = sock.socket_local_addr();
            return 0;
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
        // retrying is just the way the upper levels poll for connect.
        // should probably change this eventually.
        while(1)
        {
            vc addr;
            int avail = 0;
            vc obj = sock.socket_get_obj(avail, addr);
            if(!avail)
            {
                q_empty = 1;
                return 0;
            }
            if(obj.is_nil())
            {
                bad_op();
                return 0;
            }

            GRTLOG("connect poll ", 0, 0);
            GRTLOGVC(sock);
            GRTLOGVC(obj);

            if((obj[0] == vc("connect") || obj[0] == vc("accept")) && obj[1] == vctrue)
            {
                // connect done, we know there is are two
                // things in the q, so if there isn't, it is an api change
                // that has broken things
                vc obj = sock.socket_get_obj(avail, addr);
                if(!avail || !((obj[0] == vc("connect") || obj[0] == vc("accept")) && obj[1] == vctrue))
                {
                    bad_op();
                    return 0;
                }
                if(obj.is_nil())
                {
                    bad_op();
                    return 0;
                }
                q_empty = 0;
                break;
            }
            else
            {
                // anything else at this point is a hard error
                last_error = "connect error";
                saw_hard_error = 1;
                bad_op();
                return 0;
            }
        }
    }
    if(peer.is_nil())
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
    return 1;
}

void
SimpleSocket::bad_op()
{
    if(!sock.is_nil())
    {
        last_error = "hard error"; //sock.socket_error_desc();
        GRTLOG("bad op", 0, 0);
        GRTLOGVC(sock);
        GRTLOGVC(last_error);
        sock = vcnil;
    }
    saw_hard_error = 1;
}

void
SimpleSocket::exit()
{
    if(!sock.is_nil())
    {
        last_error = "exit error"; //sock.socket_error_desc();
        sock.socket_close(0);
        sock = vcnil;
    }
}

int
SimpleSocket::can_write()
{
    if(sock.is_nil())
        return 1; // note: this is bogus, like bad_op below, but not much we can do with now
    // need some kind of flag that a write error occurred
    // so we don't keep writing
    // kluge, we allow up to  200k in the write q before we stop them
    if(sock.socket_get_write_q_size() < 200 * 1024)
        return 1;
    GRTLOGVC(sock);
    GRTLOG("write q full", 0, 0);
    return 0;
}

int
SimpleSocket::has_data(int sec, int usec)
{
    if(sock.is_nil())
        return 1; // see comment in can_write
    return sock.socket_get_read_q_len() > 0;
}

int
SimpleSocket::active()
{
    // doesn't say anything about whether socket is
    // working or not...
    return !sock.is_nil();
}


int
SimpleSocket::sendlc(DWBYTE *buf, int len, int wbok)
{
    oopanic("oof");
    if(sock.is_nil())
        return 0;
    return 0;

}

int
SimpleSocket::recvlc(DWBYTE*& buf, int& len)
{
    oopanic("oof");
    return 0;
#if 0
    // note: buf is allocated here and returned
    if(sock.is_nil())
        return 0;


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
#endif
}


int
SimpleSocket::sendvc(vc v)
{
    if(sock.is_nil())
        return 0;
    vc len = sock.socket_put_obj(v, vcnil, 0);
    if(len.is_nil())
        return 0;
    GRTLOG("sendvc ", 0, 0);
    GRTLOGVC(sock);
    GRTLOGVC(v);

    return len;
}

int
SimpleSocket::recvvc(vc& v)
{
    if(sock.is_nil())
        return 0;
    while(1)
    {
        vc addr;
        int avail = 0;
        vc obj = sock.socket_get_obj(avail, addr);
        if(!avail)
        {
            q_empty = 1;
            return 0;
        }
        q_empty = 0;
        if(obj.is_nil())
        {
            bad_op();
            return 0;
        }

        GRTLOG("recvvc ", 0, 0);
        GRTLOGVC(sock);
        GRTLOGVC(obj);

        if(obj[0] == vc("data"))
        {
            v = obj[2];
            return obj[3];
        }
        else if(obj[0] == vc("eof"))
        {
            last_error = "eof";
            saw_hard_error = 1;
            return 0;
        }
        else if(obj[1] == vctrue)
        {
            // some synthetic indicator, but not an error
            // just eat and try the next thing in the q
        }
        else
        {
            // some error
            last_error = obj[3];
            saw_hard_error = 1;
            return 0;
        }
    }
    return 0;
}


int
Listener::init(const char *, const char *local_addr, int)
{
    initsock();

    if(local_addr == 0)
        local_addr = "any:any";
    if(sock.socket_init(local_addr, 1, 1).is_nil())
    {
        bad_op();
        return 0;
    }
    //sock.socket_set_option(noblock ? VC_NONBLOCKING: VC_BLOCKING);
    laddr = sock.socket_local_addr();
    return 1;
}

int
Listener::init2(const char *local_addr)
{
    initsock();
    //sock.set_err_callback(net_socket_error);
    if(local_addr == 0)
        local_addr = "any:any";
    if(sock.socket_init(local_addr, 1, 1).is_nil())
    {
        bad_op();
        return 0;
    }
    //sock.socket_set_option(noblock ? VC_NONBLOCKING: VC_BLOCKING);
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
    int avail = 0;
    vc newsock = sock.socket_get_obj(avail, tmp);
    if(!avail)
        return 0;
    if(newsock.is_nil())
    {
        bad_op();
        return 0;
    }
    if(newsock[1].is_nil())
    {
        // some error in low level accept
        return 0;
    }


    SimpleSocket *s = new SimpleSocket;
    s->sock = newsock[2];
    s->peer = s->sock.socket_peer_addr();
    s->laddr = s->sock.socket_local_addr();

    GRTLOG("accept simple (local, peer)", 0, 0);
    GRTLOGVC(s->laddr);
    GRTLOGVC(s->peer);
    return s;
}
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
    vc_composite::new_dfs();
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

    if(!ostrm.open(vcxstream::WRITEABLE, vcxstream::ATOMIC))
        return 0;
    vc_composite::new_dfs();
    if((len = v.xfer_out(ostrm)) < 0)
        return 0;
    if(!ostrm.close(vcxstream::FLUSH))
        return 0;
    return len;
}

int
FrameSocket::recvvc(vc& v)
{
    if(sock.is_nil())
        return 0;
    int len;
    vcxstream istrm(sock, (char *)packet_buf, plen, vcxstream::FIXED);

    if(!istrm.open(vcxstream::READABLE, vcxstream::ATOMIC))
        return 0;
    if((len = v.xfer_in(istrm)) < 0)
    {
        return 0;
    }
    if(!istrm.close(vcxstream::FLUSH))
        return 0;

    peer = sock.socket_peer_addr();
    return len;
}

int
FrameSocket::has_data(int sec, int usec)
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
