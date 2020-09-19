
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/mmconn.cc 1.11 1999/01/10 16:09:47 dwight Checkpoint $
#ifndef LINUX
#include <WinSock2.h>
#endif
//#include "vc.h"
#include "vcwsock.h"
#include "mmchan.h"
#include "netvid.h"
#include "msgdisp.h"
#include "gvchild.h"
//#include "aconn.h"
//#include "qauth.h"
#include "dwrtlog.h"

using namespace dwyco;


#ifdef _Windows
void
async_handler(SOCKET s, DWORD lp)
{
    if(WSAGETSELECTEVENT(lp) == FD_CONNECT &&
            WSAGETSELECTERROR(lp) != 0)
    {
        vc_winsock::set_async_error(s, WSAGETSELECTERROR(lp));
    }
    // win95 bug hack-around
    if(WSAGETSELECTEVENT(lp) == FD_CLOSE)
    {
        WSAAsyncSelect(s, (*MMChannel::get_main_window_callback)(0),
                       0, 0);
    }
}

void
async_lookup_handler(HANDLE h, DWORD lp)
{
    ChanList &cl = *MMChannel::get_serviced_channels();
    ChanListIter cli(&cl);
    for(; !cli.eol(); cli.forward())
    {
        MMChannel *m = cli.getp();
        if(!m)
            return;
        if(m->hr == h)
        {
            m->resolve_done = 1;
            m->resolve_result = WSAGETASYNCERROR(lp);
            m->resolve_len = WSAGETASYNCBUFLEN(lp);
            break;
        }
    }
    delete &cl;
}

#endif

void
MMChannel::show_winsock_error(int err)
{
    const char *a = vc_wsget_errstr(err);
    if(!a)
        a = "huh?";
    fail_reason = a;
    msg_out(a);
}


void
MMChannel::msg_out(const char *m)
{
    if(msg_output)
        msg_output->show(m);
}

#ifdef LIBUV_ASYNC_LOOKUPS
#include "uv.h"

static void
addrinfo_cb(uv_getaddrinfo_t *req, int status, struct addrinfo *res)
{
    ValidPtr &vp = *(ValidPtr *)req->data;
    if(!vp.is_valid())
        return;
    MMChannel *m = (MMChannel *)(void *)vp;
    if(!m)
        return;
    if(status == -1)
    {
        m->resolve_failed = 1;
        m->resolve_done = 1;
        delete &vp;
        return;
    }
    m->resolve_done = 1;
    m->addr_out.s_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    delete &vp;
}

int
MMChannel::start_resolve(enum resolve_how how, unsigned long addr, const char *hostname)
{
    call_setup = 1;
    resolve_failed = 0;
    resolve_done = 0;
    resolve_timer.set_oneshot(1);
    resolve_timer.load(5000L);
    resolve_timer.start();
    GRTLOG("resolve start", 0, 0);
    int hr;

    async_lookup_req = new uv_getaddrinfo_t;
    switch(how)
    {
    case BYNAME:
        req->data = new ValidPtr(vp);
        hr = uv_getaddrinfo(uv_lookup_loop, req, addinfo_cb, hostname, 0, 0);
        if(hr == -1)
            goto fail;
        break;
    default:
        oopanic("resolving only uses byname");
        break;
    }

    pstate = RESOLVING_NAME;
    cancel = 0;
    msg_out("Looking up name...");
    return 1;

fail:
    GRTLOG("resolve failed immediately", 0, 0);
    delete async_lookup_req;
    resolve_failed = 1;
}

void
MMChannel::cancel_resolve()
{
}

int
MMChannel::poll_resolve()
{
    if(resolve_timer.is_expired())
    {
        resolve_timer.ack_expire();
        resolve_timer.reset();
        pstate = FAILED;
        fail_reason = "timeout";
        msg_out("hostname lookup timeout");
        // libuv doesn't support windows for canceling requests
        resolve_failed = 1;
        return 0;
    }
    resolve_timer.reset();
    if(resolve_done)
    {
        if(resolve_failed)
        {
            pstate = FAILED;
            return 0;
        }
        else
        {
            return 1;
        }
    }
    return -1;
}

#else
int
MMChannel::start_resolve(enum resolve_how how, unsigned long addr, const char *hostname)
{
#ifndef LINUX
    call_setup = 1;
    resolve_failed = 0;
    resolve_done = 0;
    resolve_timer.set_oneshot(1);
    resolve_timer.load(15000L);
    resolve_timer.set_autoreload(0);
    resolve_timer.start();
    GRTLOG("resolve start", 0, 0);
    HWND h = (*get_main_window_callback)(this);
    char *wbuf = new char[MAXGETHOSTSTRUCT];

    switch(how)
    {
    case BYNAME:
        hr = WSAAsyncGetHostByName(h, WM_USER + 402, hostname, wbuf, MAXGETHOSTSTRUCT);
        if(hr == 0)
            goto fail;
        break;
    default:
        hr = WSAAsyncGetHostByAddr(h, WM_USER + 402, (const char *)&addr, sizeof(addr),
                                   PF_INET, wbuf, MAXGETHOSTSTRUCT);
        if(hr == 0)
            goto fail;
        break;
    }

    pstate = RESOLVING_NAME;
    cancel = 0;
    resolve_buf = wbuf;
    msg_out("Looking up name...");
    return 1;

fail:
    GRTLOG("resolve failed immediately", 0, 0);
    delete [] wbuf;
    resolve_failed = 1;
#endif
    return 0;
}

void
MMChannel::cancel_resolve()
{
#ifndef LINUX
    if(hr)
        WSACancelAsyncRequest(hr);
#endif
    hr = 0;
}

int
MMChannel::poll_resolve()
{
#ifdef LINUX
    pstate = FAILED;
    resolve_failed = 1;
    return 0;
#else
    int wsaerror;


    if(resolve_timer.is_expired())
    {
        resolve_timer.ack_expire();
        resolve_timer.reset();
        pstate = FAILED;
        fail_reason = "timeout";
        msg_out("hostname lookup timeout");
        if(hr)
            WSACancelAsyncRequest(hr);
        hr = 0;
        delete [] resolve_buf;
        resolve_buf = 0;
        resolve_failed = 1;
        return 0;
    }
    if(resolve_done)
    {
        wsaerror = resolve_result;
        if(wsaerror != 0)
        {
            show_winsock_error(wsaerror);
            pstate = FAILED;
            resolve_timer.reset();
            hr = 0;
            delete [] resolve_buf;
            resolve_buf = 0;
            resolve_failed = 1;
            return 0;
        }
        else
        {
            struct hostent *h = (struct hostent *)resolve_buf;
            addr_out.s_addr = *(unsigned long *)h->h_addr;

            resolve_timer.reset();
            delete [] resolve_buf;
            resolve_buf = 0;
            hr = 0;
            return 1;
        }
    }
    return -1;
#endif
}

#endif

int
MMChannel::start_connect()
{
    GRTLOG("connect started", 0, 0);
    call_setup = 1;
    char *a = inet_ntoa(addr_out);
    if(a == 0)
    {
        msg_out("address isn't a valid internet address");
        return 0;
    }
    addrstr = a;
    if(port == 0)
    {
        //strcat(addrstr, DEFAULT_PORT_SUFFIX);
        // note this is WRONG... need to pick up port from
        // directory (or same place we got the IP)
        // this should actually be hardwired (or another option
        // for default OUTGOING port) to default outgoing port,
        // which is what we want to use if we don't have any
        // other information.
        DwString b(DwNetConfigData.get_primary_suffix(addrstr.c_str()));
        addrstr = b;
    }
    else
    {
        char b[255];
        sprintf(b, ":%d", port);
        addrstr += b;
    }
    ouraddr = "any:any";
    pstate = CONNECTING;
    msg_out("Connecting...");
    tube = new MMTube;
    return 1;
}

int
MMChannel::poll_connect()
{
    // don't set up unreliable right now, breaks
    // server based calling, need to fix eventually.
    int err = tube->connect(addrstr.c_str(), ouraddr.c_str(), 0 /*,0, force_unreliable_video||force_unreliable_audio*/);
    if(err == SSTRYAGAIN)
    {
        return -1;
    }
    else if(err == SSERR)
    {
        DwString a("connection failed ");
        a += (const char *)tube->last_ctrl_error;
        fail_reason = a.c_str();
        msg_out(a.c_str());
        pstate = FAILED;
        GRTLOG("poll connect failed %s", a.c_str(), 0);

        if(low_level_connect_callback)
            (*low_level_connect_callback)(this, vcnil, llcc_arg2, llcc_arg3);
        return 0;
    }
    if(low_level_connect_callback)
        (*low_level_connect_callback)(this, vctrue, llcc_arg2, llcc_arg3);
    return 1;
}

int
MMChannel::start_negotiation()
{
    //tube->init_listener();
    start_crypto();
    nego_timer.start();
    negotiating = 1;
    cancel = 0;
    if(proxy_info.is_nil())
        msg_out("Ringing direct...");
    else
        msg_out("Ringing (via server)...");
    return 1;
}

#define FAILNEGO(x) do {negotiating = 0; FAILRET(x)} while(0)
#define FAILRET(x) {fail_reason = (x); msg_out(x); return 0;}

int
MMChannel::poll_negotiation()
{
    if(do_destroy != KEEP)
        FAILNEGO("No answer");
    if(pstate == ESTABLISHED)
    {
        negotiating = 0;
        nego_timer.reset();
        msg_out("Connection successful.");
        call_setup = 0;
        if(established_callback)
            (*established_callback)(this, ecb_arg1, ecb_arg2, ecb_arg3);
        established_callback_called = 1;
        return 1;
    }
    if(pstate == FAILED)
    {
        // need this, since it has the reason like "password incorrect"
        // from the other side.
        FAILNEGO(fail_reason);
    }
    if(cancel)
    {
        cancel = 0;
        FAILNEGO("Cancel");
    }
    return -1;
}

#if 0

int
MMChannel::poll_stun()
{
    if(sent_stun)
        return 1;
    int i;
    for(i = 0; i < MM_STUN_SOCK_COUNT; ++i)
    {
        if(stun_socks[i].is_nil())
        {
            vc s = stun_pool_get_sock();
            if(s.is_nil())
                break;
            else
                stun_socks[i] = s;
        }
    }
    if(i == MM_STUN_SOCK_COUNT)
    {
        // got all the sockets we need, now get everything
        // all wired up, and send the control msgs needed by
        // the other side.
        GRTLOG("got all STUN sockets", 0, 0);
        vc c(VC_VECTOR);
        c.append("stun");
        for(int i = 0; i < MM_STUN_SOCK_COUNT; ++i)
        {
            c.append(stun_socks[i][1][0]); // the mapped-address from the stun server
        }
        send_ctrl(c);
        sent_stun = 1;

        // set the unreliable channels up in the tube
        tube->set_mm_sock(new FrameSocket(stun_socks[0][0]));
        tube->set_channel(new FrameSocket(stun_socks[1][0]), 0, 0, AUDIO_CHANNEL_V2);
        // hmmmm... may not want to go to established state until we get
        // the stun setup from the other side... if we go to established
        // now, we will start spraying packets at the wrong place.
        if(!peer_stun.is_nil())
            got_peer_stun(peer_stun);


        return 1;
    }
    else
    {
        // NEED A TIMER HERE SO WE CAN BAIL IF WE DON'T GET SET UP
    }
    return -1;
}

void
MMChannel::got_peer_stun(vc s)
{
    // now we have the mapped peer address on the other
    // side of the link. we reconnect the sockets on our
    // side from the STUN server over to the peer we want to talk to.
    // i'll be amazed if this actually works.
    // note: it does work, sometimes. but because of the varying
    // implementations of various NAT things, it really isn't worth
    // the trouble. it causes way more tech support headaches
    // trying to deal with a customer with a flakey hardware setup
    // than just paying for the extra bandwidth to send the media
    // via tcp. and the tcp latency isn't too bad. so, this STUN
    // stuff is not very useful in this context.
    //
    GRTLOG("got peer stun mm_sock %p", tube->get_mm_sock(), 0);
    GRTLOGVC(s);
    if(tube->get_mm_sock() == 0)
    {
        peer_stun = s;
        return;
    }
    tube->reconnect_mm_sock(s[1]);
    tube->reconnect_chan(s[2], AUDIO_CHANNEL_V2);
    // spray a couple of dummy packets out here
    // in order to punch the hole in our side on the proper
    // local ports, in case we are port restricted. need this in cases
    // where we are going to recv video, and we a NOT sending
    // any media (like receiving a broadcast, for example.)
    char buf[1];
    tube->send_mm_data((DWBYTE *)buf, 0, VIDEO_CHANNEL, 0, 4, 0);
    tube->put_mm_data((DWBYTE *)buf, 0, AUDIO_CHANNEL_V2, AUDIO_SUBCHANNEL, 0, 4, 0);
    GRTLOG("sent caller hole punches", 0, 0);
    peer_stun = s;
    pstate = ESTABLISHED;
    if(stun_established_callback)
        (*stun_established_callback)(this, sec_arg1, sec_arg2, sec_arg3);
    //msg_out("Connected.");
    //timer1.stop();
    //destroy_callback = 0;
}
#endif


