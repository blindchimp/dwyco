
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/netvid.cc 1.27 1999/01/10 16:09:38 dwight Checkpoint $
 */
#ifdef _Windows
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include "netvid.h"
#include "dwlog.h"
#include "doinit.h"
#include "vc.h"
#include "dwvec.h"
#include "dwrtlog.h"
#include "dwstr.h"
#include "netlog.h"
#include "dwyco_rand.h"
using namespace dwyco;

vc MMTube::Ping("vomit");
int MMTube::dummy;
int MMTube::always_zero;


vc
MMTube::mklog(vc v1, vc v2, vc v3, vc v4)
{
    vc v(VC_VECTOR);
    if(ctrl_sock)
    {
    v.append("local_ip");
    v.append(ctrl_sock->local_addr().c_str());
    v.append("peer_ip");
    v.append(ctrl_sock->peer_addr().c_str());
    }
    v.append("tube_id");
    v.append(tubeid);
    if(!v1.is_nil())
    {
        v.append(v1);
        v.append(v2);
    }
    if(!v3.is_nil())
    {
        v.append(v3);
        v.append(v4);
    }
    return v;
}

MMTube::MMTube() :
    baud(0, !DWVEC_FIXED, DWVEC_AUTO_EXPAND),
    in_bits(0, !DWVEC_FIXED, DWVEC_AUTO_EXPAND),
    socks(0, !DWVEC_FIXED, DWVEC_AUTO_EXPAND)

{
    tick_time = 100;
    time = 0;
    keepalive_timer.set_interval(5000);
    keepalive_timer.set_autoreload(1);
    connected = 0;
    ctrl_sock = 0;
    mm_sock = 0;
    last_tick = GetTickCount();
    enc_ctrl = 0;
    dec_ctrl = 0;
    init_netlog();
    tubeid = dwyco_rand();
    log_signal.connect_ptrfun(netlog::netlog_slot);
}

MMTube::~MMTube()
{
    if(ctrl_sock)
        log_signal.emit(mklog("event", "tube destroy"));
    delete ctrl_sock;
    delete mm_sock;
    for(int i = 2; i < socks.num_elems(); ++i)
        delete socks[i];
}


void
MMTube::toss()
{
    keepalive_timer.stop();
    connected = 0;
    if(ctrl_sock)
    {
        last_ctrl_error = ctrl_sock->last_error;
        GRTLOG("toss %s %s", (const char *)last_ctrl_error, ctrl_sock->peer_addr().c_str());
        log_signal.emit(mklog("event", "ctrl toss"));
    }
    else
    {
        GRTLOG("toss %s ", (const char *)last_ctrl_error, 0);
    }

    delete ctrl_sock;
    ctrl_sock = 0;
    if(mm_sock)
        last_mm_error = mm_sock->last_error;
    delete mm_sock;
    mm_sock = 0;
    for(int i = 2; i < socks.num_elems(); ++i)
    {
        delete socks[i];
        socks[i] = 0;
    }

}

int
MMTube::is_connected()
{
    return connected;
}



int
MMTube::find_chan()
{
    int i;
    for(i = FIRST_RELIABLE_CHAN; i < socks.num_elems(); ++i)
    {
        if(socks[i] == 0)
        {
            in_bits[i] = 0;
            enc_chan[i] = 0;
            dec_chan[i] = 0;
            return i;
        }
    }
    i = socks.num_elems();
    if(i < FIRST_RELIABLE_CHAN)
        i = FIRST_RELIABLE_CHAN;
    in_bits[i] = 0;
    socks[i] = 0;
    enc_chan[i] = 0;
    dec_chan[i] = 0;
    return i;
}

void
MMTube::set_channel(SimpleSocket *s, int enc, int dec, int chan)
{
    if(!connected)
        return;
    if(s != 0 && socks[chan] != 0)
        oopanic("bad set chan");
    socks[chan] = s;
    enc_chan[chan] = enc;
    dec_chan[chan] = dec;
    log_signal.emit(mklog("event", "chan import", "chan_id", chan));
    // what about in_bits and baud... hmmm
}

SimpleSocket *
MMTube::get_chan_sock(int chan)
{
    return socks[chan];
}

void
MMTube::drop_channel(int chan)
{
    if(!connected)
        return;
    log_signal.emit(mklog("event", "chan drop", "chan_id", chan));
    delete socks[chan];
    socks[chan] = 0;
    enc_chan[chan] = 0;
    dec_chan[chan] = 0;
}

int
MMTube::gen_channel(unsigned short remote_port, int& chan)
{
    if(!connected)
        return SSERR;
    int retry = 1;
    if(chan == -1)
    {
        chan = find_chan();
    }
    if(socks[chan] == 0)
    {
        socks[chan] = new SimpleSocket;
        retry = 0;
        socks[chan]->non_blocking(1);
        in_bits[chan] = 0;
    }
    // XXX block/noblock on connect
    socks[chan]->non_blocking(1);

    DwString peer;
    if(!retry)
    {
        char csps[20];
        if(!ctrl_sock)
        {
            drop_channel(chan);
            return SSERR;
        }
        sprintf(csps, "%u", remote_port);
        peer = ctrl_sock->peer_addr();
        int i;
        if((i = peer.find(":")) == DwString::npos)
            return SSERR;
        peer.erase(i + 1);
        peer += csps;
    }
    if(!socks[chan]->init(retry ? 0 : peer.c_str(), 0, retry))
    {
        int ret = SSERR;
        if(socks[chan]->wouldblock())
            return SSTRYAGAIN;
        drop_channel(chan);
        return ret;
    }
    log_signal.emit(mklog("event", "chan connected", "chan_id", chan));
    return 1;
}

int
MMTube::connect(const char *remote_addr, const char *local_addr, int block, HWND hwnd, int setup_unreliable)
{
    if(connected)
        return SSERR;
    int retry = 1;
    if(!ctrl_sock)
    {
        ctrl_sock = new SimpleSocket;
        retry = 0;
        ctrl_sock->non_blocking(1);
    }
    // block for this connect

    // XXX block/noblock on connect
    ctrl_sock->non_blocking(!block);
    if(!ctrl_sock->init(remote_addr, local_addr, retry, hwnd))
    {
        int ret = SSERR;
        if(ctrl_sock->wouldblock())
            return SSTRYAGAIN;
        toss();
        return ret;
    }
    ctrl_sock->non_blocking(1);

    if(setup_unreliable)
    {
        oopanic("old unreliable");

    }
    connected = 1;
    log_signal.emit(mklog("event", "ctrl connected"));
    return 1;
}

int
MMTube::init_mm_listener(int port)
{
    if(connected)
        return SSERR;

    mm_sock = new FrameSocket;

    char la[200];
    sprintf(la, "any:%d", port);

    if(!mm_sock->non_blocking(1) || !mm_sock->init("any:any", la))
    {
        int ret = SSERR;
        if(mm_sock->wouldblock())
            ret = SSTRYAGAIN;
        toss();
        return ret;
    }
    connected = 1;
    return 1;
}

vc
MMTube::remote_addr_ctrl()
{
    if(!connected || !ctrl_sock)
        return vcnil;
    DwString a = ctrl_sock->peer_addr();
    vc ret(a.c_str());
    return ret;
}


int
MMTube::accept(SimpleSocket *s)
{
    if(connected)
        return SSERR;
    if(!s->non_blocking(1))
    {
        toss();
        return SSERR;
    }
    ctrl_sock = s;
    connected = 1;
    log_signal.emit(mklog("event", "ctrl accepted"));
    return 1;

}

int
MMTube::disconnect()
{
    toss();
    return 1;
}

int
MMTube::disconnect_ctrl()
{
    if(!ctrl_sock)
        return 1;
    last_ctrl_error = ctrl_sock->last_error;
    log_signal.emit(mklog("event", "ctrl terminated"));
    delete ctrl_sock;
    ctrl_sock = 0;
    GRTLOG("toss ctrl %s", (const char *)last_ctrl_error, 0);
    return 1;
}

int
MMTube::has_mmdata()
{
    if(!connected)
        return 0;
    return mm_sock && mm_sock->has_data();
}

int
MMTube::can_write_mmdata()
{
    if(!connected)
        return 0;
    return mm_sock && mm_sock->can_write();
}

int
MMTube::has_ctrl()
{
    if(!connected || !ctrl_sock)
        return 0;
    return ctrl_sock->has_data();
}

int
MMTube::can_write_ctrl()
{
    if(!connected || !ctrl_sock)
        return 0;
    return ctrl_sock->can_write();
}

int
MMTube::has_data(int chan)
{
    if(!connected)
        return 0;
    if(chan >= socks.num_elems() || !socks[chan])
        return 0;
    return socks[chan]->has_data();
}

int
MMTube::can_write_data(int chan)
{
    if(!connected)
        return 0;
    if(chan >= socks.num_elems() || !socks[chan])
        return 0;
    return socks[chan]->can_write();
}

int
MMTube::is_disconnected()
{
    return !connected;
}

int
MMTube::buf_drained(int chan)
{
    //return 1;
    GRTLOGA("drain %p %d %d %d", this, chan, (int)in_bits[chan], (int)baud[chan], 0);
    if(in_bits[chan] <= 0)
    {
        in_bits[chan] = 0;
        return 1;
    }
    return 0;
}

int
MMTube::clear_buf_ctrl(int chan)
{
    in_bits[chan] = 0;
    return 1;
}

// note: errors in the mm channel
// are not treated as fatal.
//
// note: in order for these calls to produce a
// stream that is compatible with ver 0.95
// user len must be 1, and seq must be 0.
// likewise, the receiver has to be told it is an
// old style stream by setting the user_len to 1
// and the seq to 0.

int
MMTube::send_mm_data(DWBYTE *buf, int len, int chan, int user_byte, int user_len, int seq)
{
    if(!mm_sock)
        return SSERR;
    int ret = mm_sock->send(buf, len, chan, user_byte, user_len, seq);
    if(ret)
    {
        in_bits[chan] += 8 * (long)len +  8 + user_len * 8 + (seq != 0 ? 32 : 0);
        GRTLOG("in bits chan %d %d", chan, in_bits[chan]);
    }
    return ret;
}

int
MMTube::recv_mm_data(DWBYTE*& buf, int& len, int& chan, int& user, int user_len, int& seq)
{
    int t = mm_sock->recv(buf, len, chan, user, user_len, seq);
    if(!t)
    {
        if(mm_sock->wouldblock())
            return SSTRYAGAIN;
        return SSERR;
    }
    return 1;
}

//
// new style
//
int
MMTube::get_mm_data(DWBYTE*& buf, int& len, int chan, int& subchan,
                    int& user, int user_len, int& seq)
{
    if(chan < FIRST_UNRELIABLE_CHAN || chan >= FIRST_RELIABLE_CHAN ||
            socks[chan] == 0)
        return SSERR;
    FrameSocket *s = (FrameSocket *)socks[chan];
    int t = s->recv(buf, len, subchan, user, user_len, seq);
    if(!t)
    {
        if(s->wouldblock())
            return SSTRYAGAIN;
        return SSERR;
    }
    return 1;
}

int
MMTube::put_mm_data(DWBYTE *buf, int len, int chan, int subchan, int user,
                    int user_len, int seq)
{
    if(chan < FIRST_UNRELIABLE_CHAN || chan >= FIRST_RELIABLE_CHAN ||
            socks[chan] == 0)
        return SSERR;
    int ret = ((FrameSocket *)socks[chan])->send(buf, len, subchan, user, user_len, seq);
    if(ret)
        in_bits[chan] += 8 * (long)len +  8 + user_len * 8 + (seq != 0 ? 32 : 0);
    return ret;
}


int
MMTube::send_ctrl_data(vc v)
{
    if(!connected || !ctrl_sock)
        return SSERR;
    if(enc_ctrl)
    {
        GRTLOG("send ctrl ENC", 0, 0);
        GRTLOGVC(v);
        vc tmp = vclh_encdec_xfer_enc_ctx(enc_ctx, v);
        if(tmp.is_nil())
            return SSERR;
        v = tmp;
    }
    if(!ctrl_sock->sendvc(v))
    {
        if(ctrl_sock->wouldblock())
            return SSTRYAGAIN;
        disconnect_ctrl();
        return SSERR;
    }
    // assume control and data are going over
    // same link
    //in_bits += 8 * (long)len;
    return 1;
}

int
MMTube::recv_ctrl_data(vc& v)
{
    if(!connected || !ctrl_sock)
        return SSERR;
    // eat keepalive messages
    do
    {
        if(!ctrl_sock->recvvc(v))
        {
            if(ctrl_sock->wouldblock())
                return SSTRYAGAIN;
            disconnect_ctrl();
            return SSERR;
        }
        if(dec_ctrl)
        {
            GRTLOG("DEC ctrl ", 0, 0);
            vc  tmp;
            if(encdec_xfer_dec_ctx(dec_ctx, v, tmp).is_nil())
            {
                GRTLOG("DEC fail", 0, 0);
                return SSERR;
            }
            GRTLOGVC(tmp);
            v = tmp;
        }
    }
    while(v == Ping);
    return 1;
}


int
MMTube::send_data(vc v, int chan)
{
    return send_data(v, chan, chan);
}

int
MMTube::send_data(vc v, int chan, int bwchan)
{
    int len = 0;
    if(socks[chan] == 0)
        return SSERR;
    // note: bogosity, sendvc doesn't return len, just 0 or 1
    if(enc_chan[chan])
    {
        GRTLOG("ENC chan %d", chan, 0);
        vc tmp = vclh_encdec_xfer_enc_ctx(enc_ctx, v);
        if(tmp.is_nil())
            return SSERR;
        v = tmp;
        len = v[1].len();
    }
    else
    {
        // guestimate
        len = v.len();
    }
    if(!socks[chan]->sendvc(v))
    {
        if(socks[chan]->wouldblock())
            return SSTRYAGAIN;
        drop_channel(chan);
        return SSERR;
    }
    in_bits[bwchan] += len * 8;
    return 1;
}

#if 0
int
MMTube::send_data(DWBYTE *buf, int len, int chan, enum wbok w)
{
    return send_data(buf, len, chan, chan, w);
}

int
MMTube::send_data(DWBYTE *buf, int len, int chan, int bwchan, enum wbok w)
{
    if(socks[chan] == 0)
        return SSERR;
    if(!socks[chan]->sendlc(buf, len, (w == WB_OK) ? 1 : 0))
    {
        if(socks[chan]->wouldblock())
            return SSTRYAGAIN;
        drop_channel(chan);
        return SSERR;
    }
    in_bits[bwchan] += len * 8;
    return 1;
}
#endif

int
MMTube::recv_data(vc& v, int chan)
{
    if(socks[chan] == 0)
        return SSERR;
    if(!socks[chan]->recvvc(v))
    {
        if(socks[chan]->wouldblock())
            return SSTRYAGAIN;
        drop_channel(chan);
        return SSERR;
    }
    if(dec_chan[chan])
    {
        GRTLOG("DEC chan %d ", chan, 0);
        vc  tmp;
        if(encdec_xfer_dec_ctx(dec_ctx, v, tmp).is_nil())
        {
            GRTLOG("DEC fail", 0, 0);
            return SSERR;
        }
        GRTLOGVC(tmp);
        v = tmp;
    }
    return 1;
}

#if 0
int
MMTube::recv_data(DWBYTE *&buf, int& len, int chan)
{
    if(socks[chan] == 0)
        return SSERR;
    if(!socks[chan]->recvlc(buf, len))
    {
        if(socks[chan]->wouldblock())
            return SSTRYAGAIN;
        drop_channel(chan);
        return SSERR;
    }
    return 1;
}
#endif


int
MMTube::tick()
{
    // called for a timer tick, tho shouldn't
    // be at "interrupt time" since it might
    // want to fire packets, etc.
    // note: need to acertain actual time since
    // last call...
    if(!connected)
        return 0;
    unsigned long t = GetTickCount();
    tick_time = t - last_tick;
    last_tick = t;
    //if(mm_sock)
    //mm_sock->tick();
    time += tick_time;
    //keepalive_timer.tick();
    if(keepalive_timer.is_expired())
    {
        keepalive_timer.ack_expire();
        if(!keepalive())
        {
            toss();
            return 0;
        }
    }
    int n = baud.num_elems();
    for(int i = 0; i < n; ++i)
    {
        unsigned long bits_pumped = (tick_time * baud[i]) / 1000;
        in_bits[i] -= bits_pumped;
        if(in_bits[i] < 0)
            in_bits[i] = 0;
    }
    return 1;
}

unsigned long
MMTube::set_est_baud(unsigned long t, int chan)
{
    if(baud.num_elems() < chan + 1)
    {
        baud.set_size(chan + 1);
        in_bits.set_size(chan + 1);
    }
    //in_bits[chan] = 0;
    unsigned long tmp = baud[chan];
    baud[chan] = t;
    GRTLOG("set est baud chan %d to %d", chan, t);
#if 0
    // note: this isn't used right now
    if(mm_sock)
        mm_sock->baud = baud;
#endif
    return tmp;
}

unsigned long
MMTube::get_est_baud(int chan)
{
    if(baud.num_elems() < chan + 1)
    {
        baud.set_size(chan + 1);
        in_bits.set_size(chan + 1);
        baud[chan] = 0;
        in_bits[chan] = 0;
    }
    if(socks[chan] == 0)
        return 0;
    return baud[chan];
}

int
MMTube::keepalive()
{
    if(!connected || !ctrl_sock)
        return 0;
    if(!ctrl_sock->sendvc(Ping))
    {
        if(ctrl_sock->wouldblock())
            return 1; // hmmmm.... something probably wrong... but...
        return 0;
    }
    return 1;
}

void
MMTube::set_keepalive_time(unsigned long t)
{
    keepalive_timer.set_interval(t);
}

void
MMTube::set_keepalive(int t)
{
    if(t)
    {
        keepalive_timer.reset();
        keepalive_timer.start();
    }
    else
        keepalive_timer.stop();
}

int
MMTube::dropped_mm_packets()
{
    if(!connected)
        return 0;
    return mm_sock && mm_sock->get_packets_lost();
}

void
MMTube::reset_dropped_mm_packets()
{
    if(!connected)
        return;
    if(mm_sock)
        mm_sock->set_packets_lost(0);
}

void
MMTube::mm_packet_stats(int& dropped, int& s, int& r)
{
    if(!connected)
    {
        dropped = 0;
        s = 0;
        r = 0;
        return;
    }
    if(mm_sock)
    {
        dropped = mm_sock->get_packets_lost();
        mm_sock->get_packet_counts(s, r);
    }
    else
    {
        dropped = 0;
        s = 0;
        r = 0;
    }
}

void
MMTube::set_mm_packet_stats(int dropped, int s, int r)
{
    if(!connected)
        return;
    if(mm_sock)
    {
        mm_sock->set_packets_lost(dropped);
        mm_sock->set_packet_counts(s, r);
    }
}

void
MMTube::set_mm_sock(FrameSocket *fs)
{
    delete mm_sock;
    mm_sock = fs;
}

FrameSocket *
MMTube::get_mm_sock()
{
    return mm_sock;
}

int
MMTube::reconnect_mm_sock(const char *remote_addr)
{
    if(!mm_sock)
        return SSERR;
    if(mm_sock->reconnect(remote_addr))
        return 1;
    return SSERR;
}

int
MMTube::reconnect_chan(const char *remote_addr, int chan)
{
    if(chan < FIRST_UNRELIABLE_CHAN || chan >= FIRST_RELIABLE_CHAN ||
            socks[chan] == 0)
        return SSERR;
    FrameSocket *s = (FrameSocket *)socks[chan];
    if(s->reconnect(remote_addr))
        return 1;
    return SSERR;
}

int
MMTube::set_key_iv(vc key, vc iv)
{

    if(enc_ctx.is_nil())
        enc_ctx = vclh_encdec_open();
    else
    {
        vclh_encdec_close(enc_ctx);
        enc_ctx = vclh_encdec_open();
    }
    if(dec_ctx.is_nil())
        dec_ctx = vclh_encdec_open();
    else
    {
        vclh_encdec_close(dec_ctx);
        dec_ctx = vclh_encdec_open();
    }

    vclh_encdec_init_key_ctx(enc_ctx, key, iv);
    vclh_encdec_init_key_ctx(dec_ctx, key, iv);
    return 1;
}

int
MMTube::start_decrypt_chan(int chan)
{
    dec_chan[chan] = 1;
    return 1;
}

int
MMTube::start_encrypt_chan(int chan)
{
    enc_chan[chan] = 1;
    return 1;
}

int
MMTube::start_decrypt_ctrl()
{
    dec_ctrl = 1;
    return 1;
}

int
MMTube::start_encrypt_ctrl()
{
    enc_ctrl = 1;
    return 1;
}

int
MMTube::get_encrypt_chan(int chan)
{
    return enc_chan[chan];
}

int
MMTube::get_decrypt_chan(int chan)
{
    return dec_chan[chan];
}

int
MMTube::get_encrypt_ctrl()
{
    return enc_ctrl;
}

int
MMTube::get_decrypt_ctrl()
{
    return dec_ctrl;
}

