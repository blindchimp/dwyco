
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifdef DWYCO_UDP_PAL
#undef LOCAL_TEST
#undef CDC32
#include "vc.h"
#include "netcod.h"
#include "dwtimer.h"
#include "vcdecom.h"
#include "qmsg.h"
#include "cdcver.h"
#include "snds.h"
#include "qdirth.h"
#include "dwrtlog.h"
#include "dwstr.h"
#include "cdcpal.h"
#include "dwrtlog.h"
#include "aconn.h"
#include "se.h"
#include "dhsetup.h"
#include "vcudh.h"
#include "vccrypt2.h"


extern vc My_UID;
extern vc Online;
extern vc Client_types;
extern int Refresh_users;
extern vc Pal_server_list;
vc Pal_auth_state;
extern vc I_grant;
extern vc They_grant;
extern vc Pals;
extern vc Client_ports;

FrameSocket *Pal_recv_sock;
FrameSocket *Pal_send_sock;
int Stop_sending_login;
static DwTimer Pal_login_timer("pal login");
static DwTimer Pal_ping_timer("pal ping");
// not perfect if packets are dropped, the
// response we get may not match, but it isn't
// fatal
static vc Last_sent;
int Pal_recvs;
int Pal_sends;
static int Logout_ok;
static int Init;
int Pal_logged_in;
#define MAX_LOGIN_TRIES 5
static int Login_tries;
//void pal_login_failed(int);
// note: there is a problem with the windows
// implementation of UDP. it seams that even if
// a socket is not connected, any error from that
// socket closes the socket. so, once you get
// an error, you have to completely reset the pal
// client. this wasn't the case previously, and errors
// could just be ignored, which was better.
static int Total_failures;
#define MAX_TOTAL_FAILURES 5
int Inhibit_pal;
int Old_pal_recv;

static vc Pal_enc_ctx;
static vc Pal_dec_ctx;

#define MAXPALS 300

// note: this stuff is kinda bogus because
// it assumes the pal list doesn't change
// at all. if it does, there is a risk the
// online info will be slightly wrong, but
// this is a best-guess protocol anyways, so
// it is no big deal.

vc
set_to_vector(vc s)
{
    vc v(VC_VECTOR);

    vc_bag *vs = (vc_bag *)s.nonono();
    VcBagIter i(&vs->set);
    int max = 200;
    for(; !i.eol() && max; i.forward())
    {
        vc uid = i.get();
        v.append(uid);
        --max;
    }
    return v;
}

// note: this function is also used to transmit pal list to
// chat server, so don't update it unless you have to.
vc
copy_accept()
{
    vc v(VC_VECTOR);
    int max = MAXPALS;
    int n = Pals.num_elems();
    for(int i = 0; i < n && max; ++i)
    {
        if(!uid_ignored(Pals[i]))
        {
            v.append(Pals[i]);
            --max;
        }
    }
    return v;
}

static
vc
transient_online_list()
{
    vc v(VC_VECTOR);
    int max = MAXPALS;
    int n = Pals.num_elems();
    for(int i = 0; i < n && max; ++i)
    {
        if(!uid_ignored(Pals[i]))
        {
            v.append(Pals[i]);
            --max;
        }
    }
#if 0
    if(max != 0)
    {
        // fill out the balance with others from the
        // zap list, sort it so users that
        // are more likely to be online are
        // near the top (if you have unread messages
        // from them, for example.)
        if(UserList.num_elems() == 0)
            load_users();
        for(int j = 0; max && j < UserList.num_elems(); ++j)
        {
            vc uid = UserList[j][UL_INFO][QIR_FROM];
            if(!Pals.contains(uid) && !uid_ignored(uid))
            {
                v.append(uid);
                --max;
            }
        }
    }
#endif
    return v;
}

static
vc
build_enc_packet(vc v)
{
    vc p(VC_VECTOR);
    if(Pal_enc_ctx.is_nil())
        return vcnil;

    p[0] = "decrypt";
    p[1] = My_UID;
    p[2] = vclh_encdec_xfer_enc_ctx(Pal_enc_ctx, v);
    vc p2(VC_VECTOR);
    p2[0] = p;
    return p2;
}

int
init_pal()
{
    if(Init)
        return 1;
    Init = 1;
    Pal_logged_in = 0;
    Login_tries = 0;
    Logout_ok = 0;
    Stop_sending_login = 0;
    Last_sent = vc(VC_VECTOR);
    if(Inhibit_pal)
        return 1;

    int ret = 1;
    DwString sip;
    Pal_recv_sock = new FrameSocket;
#ifdef CDC32
    if(!Pal_recv_sock->init("any:any", "any:6783"))
#else
    DwString suf("any:6782");
    if(!DwNetConfigData.get_force_non_firewall_friendly())
    {
        suf = DwNetConfigData.get_pal_suffix("any");
    }

    if(!Pal_recv_sock->init("any:any", suf.c_str()))
#endif
    {
        delete Pal_recv_sock;
        Pal_recv_sock = 0;
        ret = 0;
    }

    Pal_send_sock = new FrameSocket;
    if(Pal_server_list.type() != VC_VECTOR ||
            Pal_server_list[0].type() != VC_VECTOR)
    {
#ifdef LOCAL_TEST
        if(!Pal_send_sock->init("204.75.209.43:10096"))
#else
#ifdef CDC32
        if(!Pal_send_sock->init("204.75.209.97:10096"))
#else
        if(!Pal_send_sock->init("209.61.158.209:10096"))
#endif
#endif
        {
            delete Pal_send_sock;
            Pal_send_sock = 0;
            ret = 0;
        }
    }
    else
    {
        // get pal server from list
        sip = (const char *)Pal_server_list[0][1];
        sip += ":";
        sip += Pal_server_list[0][2].peek_str();
        if(!Pal_send_sock->init(sip.c_str()))
        {
            delete Pal_send_sock;
            Pal_send_sock = 0;
            ret = 0;
        }
    }

    Pal_login_timer.set_autoreload(1);
    Pal_login_timer.set_interval(10000);
    Pal_login_timer.set_oneshot(0);
    Pal_login_timer.reset();
    Pal_login_timer.load(500);

    Pal_ping_timer.load(120000);
    Pal_ping_timer.set_autoreload(1);
    Pal_ping_timer.set_interval(120000);
    Pal_ping_timer.set_oneshot(0);
    Pal_ping_timer.reset();
    Pal_ping_timer.start();
    // for now, the always_accept set is the pal list
    return ret;
}

void
exit_pal()
{
    delete Pal_send_sock;
    Pal_send_sock = 0;
    delete Pal_recv_sock;
    Pal_recv_sock = 0;
    Pal_login_timer.reset();
    Pal_ping_timer.reset();
    Last_sent = vc(VC_VECTOR);
    Init = 0;
}

static void
total_reset()
{
    GRTLOG("total reset", 0, 0);
    exit_pal();
    init_pal();
}

void
process_pal_resp(vc v)
{
    if(!Init)
        return;
    if(v.type() != VC_VECTOR)
        return;
    static vc vcon("on");
    static vc vcoff("off");
    static vc vcrelog("relog");
    static vc vclogin("login");
    static vc vclogoutok("logoutok");
    GRTLOG("pal resp", 0, 0);
    GRTLOGVC(v);
    if(v[0] == vclogin)
    {
        vc uo = v[1];
        int n = uo.num_elems();
        int alert = 0;
        vc cal = Last_sent;
        int i;
        // return is vectors like this
        // vector(uid ip type ports)
        //
        for(i = 0; i < n; ++i)
        {
            cal.del(uo[i][0]);
            if(!Online.contains(uo[i][0]))
            {
                if(online_noise(uo[i][0]))
                {
                    alert = 1;
                }
            }
            Online.add_kv(uo[i][0], uo[i][1]);
            Client_types.add_kv(uo[i][0], uo[i][2]);
            Client_ports.add_kv(uo[i][0], uo[i][3]);
            se_emit(SE_STATUS_CHANGE, uo[i][0]);
        }
        // everyone else we asked about is *offline*
        n = cal.num_elems();
        for(i = 0; i < n; ++i)
        {
            Online.del(cal[i]);
            Client_types.del(cal[i]);
            Client_ports.del(cal[i]);
            se_emit(SE_STATUS_CHANGE, cal[i]);
        }

        Refresh_users = 1;
        Stop_sending_login = 1;
        Pal_login_timer.stop();
        // v[2] is ping interval from the server
        if(v[2].type() == VC_INT)
        {
            Pal_ping_timer.load(1000 * (int)v[2]);
            Pal_ping_timer.set_interval(1000 * (int)v[2]);
            Pal_ping_timer.reset();
            Pal_ping_timer.start();
        }
        if(alert)
        {
            play_online_alert();
        }
        Pal_logged_in = 1;
    }
    else if(v[0] == vcon)
    {
        vc uo = v[1];
        int n = uo.num_elems();
        int alert = 0;
        int i;
        // return is vectors like this
        // vector(uid ip type ports)
        //
        for(i = 0; i < n; ++i)
        {
            //int hu = have_user(uo[i][0]);
            if(/*hu && */!Online.contains(uo[i][0]))
            {
                if(online_noise(uo[i][0]))
                {
                    alert = 1;
                }
                Refresh_users = 1;
                GRTLOG("pal on refresh users", 0, 0);
            }
            //if(hu)
            //{
            Online.add_kv(uo[i][0], uo[i][1]);
            Client_types.add_kv(uo[i][0], uo[i][2]);
            Client_ports.add_kv(uo[i][0], uo[i][3]);
            //}
            se_emit(SE_STATUS_CHANGE, uo[i][0]);
        }
        // v[2] is ping interval from the server
        if(v[2].type() == VC_INT)
        {
            Pal_ping_timer.load(1000 * (int)v[2]);
            Pal_ping_timer.set_interval(1000 * (int)v[2]);
            Pal_ping_timer.reset();
            Pal_ping_timer.start();
        }
        if(alert)
        {
            play_online_alert();
        }
    }
    else if(v[0] == vcoff)
    {
        if(Online.contains(v[1]))
        {
            Refresh_users = 1;
            GRTLOG("pal off refresh users", 0, 0);
        }
        Online.del(v[1]);
        Client_types.del(v[1]);
        Client_ports.del(v[1]);
        se_emit(SE_STATUS_CHANGE, v[1]);
    }
    else if(v[0] == vcrelog)
    {
        // reset and log in again, server lost our state.
        Stop_sending_login = 0;
        Login_tries = 0;
        Pal_login_timer.reset();
    }
    else if(v[0] == vclogoutok)
    {
        Logout_ok = 1;
        Pal_logged_in = 0;
    }
}

void
pal_relogin()
{
    if(!Init)
        return;
    Stop_sending_login = 0;
    Login_tries = 0;
    Pal_login_timer.reset();
    Pal_login_timer.load(100); // issue next login nearly instantly
    Pal_logged_in = 0;
}

static int
poll_pal_socks(int sec, int usec)
{
    if(!Init)
        return 0;
    if(Old_pal_recv)
    {
        while(Pal_recv_sock && Pal_recv_sock->has_data())
        {
            vc v;
            if(Pal_recv_sock->recvvc(v))
                process_pal_resp(v);
            else
                break;
        }
    }
    else
    {
        while(Pal_send_sock && Pal_send_sock->has_data(sec, usec))
        {
            vc v;
            if(Pal_send_sock->recvvc(v))
            {
                vc decvc;
                if(Pal_dec_ctx.is_nil())
                {
                    total_reset();
                    break;
                }
                if(encdec_xfer_dec_ctx(Pal_dec_ctx, v, decvc).is_nil())
                {
                    total_reset();
                    break;
                }
                // key check
                if(decvc.type() != VC_VECTOR || decvc[0].type() != VC_INT || (long)decvc[0] != 0)
                {
                    total_reset();
                    break;
                }
                v = decvc[1];
                process_pal_resp(v);
            }
            else
            {
                ++Total_failures;
                GRTLOG("recv failed", 0, 0);
                total_reset();
                break;
            }
        }
    }
    return 1;
}

int
pal_login()
{
    if(!Init)
        return 0;
    if(Login_tries > MAX_LOGIN_TRIES)
    {
        static int been_here;
        if(!been_here)
        {
            been_here = 1;
            //pal_login_failed(PAL_LIST_CANT_LOGIN);
            ++Total_failures;
        }
        exit_pal();
        return 0;
    }
    ++Login_tries;
    vc v(VC_VECTOR);
    v[0] = "online";
    v[1] = My_UID;
    v[2] = transient_online_list();
    Last_sent = v[2];
    vc v2(VC_VECTOR);
    v2[0] = v;
    int is_invisible();
    v[3] = is_invisible() ? "invis" : "vis";
    // invis gone 1/2011
    //v[3] = "vis";
    // output: vector(vector(command) peer-override vector(client-info))
    // peer override is nil
    //
    // command is vector(online uid vector(pals list) vis fw-info)
    //
    v2[2] = vc(VC_VECTOR);
    v2[2][0] = PALTYPE;

    // note: firewall stuff isn't added by
    // cdc at the moment
    v[4] = vcnil; // fw info
    // note: manual pal auth, always/never stuff is done ca 1/2011
#if 0
    v[5] = Pal_auth_state.is_nil() ? vcnil : I_grant; // i grant
    // note: always send in "they grant" because if we are
    // in non-auth mode, we still want other users who
    // are in auth mode to be visible on our pal list
    //
    v[6] = They_grant; // they grant
    extern vc Always_visible;
    extern vc Never_visible;
    v[7] = set_to_vector(Always_visible);
    v[8] = set_to_vector(Never_visible);
#endif

    v[5] = vcnil;
    v[6] = vcnil;
    // experiment, ca 1/2014, make yourself always visible to
    // people on your pal list
    //v[7] = copy_accept();
    // experiment abandoned. confused people a bit.
    v[7] = vc(VC_VECTOR);
    v[8] = vc(VC_VECTOR);

    // add new firewall stuff
    if(!DwNetConfigData.get_force_non_firewall_friendly())
    {
        vc fw(VC_VECTOR);
        if(DwNetConfigData.get_advertise_nat_ports())
        {
            fw[0] = DwNetConfigData.get_nat_primary_port();
            fw[1] = DwNetConfigData.get_nat_secondary_port();
            fw[2] = DwNetConfigData.get_nat_pal_port();
        }
        else
        {
            fw[0] = DwNetConfigData.get_primary_port();
            fw[1] = DwNetConfigData.get_secondary_port();
            fw[2] = DwNetConfigData.get_pal_port();
        }
        v[4] = fw;
    }
    if(Pal_send_sock && Pal_send_sock->can_write())
    {
        vc pal_key;
        vc material = dh_store_and_forward_material(Server_publics, pal_key);
        Pal_enc_ctx = vclh_encdec_open();
        Pal_dec_ctx = vclh_encdec_open();
        vclh_encdec_init_key_ctx(Pal_enc_ctx, pal_key, 0);
        vclh_encdec_init_key_ctx(Pal_dec_ctx, pal_key, 0);
        vc setup(VC_VECTOR);
        setup[0] = "setup";
        setup[1] = material;
        setup[2] = vclh_encdec_xfer_enc_ctx(Pal_enc_ctx, v2);
        vc v3(VC_VECTOR);
        v3[0] = setup;
        v3[2] = vc(VC_VECTOR);
        v3[2][0] = PALTYPE;

        if(Pal_send_sock->sendvc(v3) == 0)
        {
            //pal_login_failed(PAL_LIST_CANT_SEND);
            // unlikely it will ever succeed
            ++Total_failures;
            GRTLOG("online send failed", 0, 0);
            exit_pal();
        }
        ++Pal_sends;
    }
    return 1;
}

int
pal_set_state(vc state)
{
    if(!Init)
        return 0;
    // note: invis not supported anymore
    return 1;
#if 0

    vc v(VC_VECTOR);
    v[0] = "state";
    v[1] = My_UID;
    v[2] = state;
    vc v2(VC_VECTOR);
    v2[0] = v;
    if(Pal_send_sock && Pal_send_sock->can_write())
    {
        if(Pal_send_sock->sendvc(v2) == 0)
        {
            ++Total_failures;
            GRTLOG("set state send failed", 0, 0);
            total_reset();
        }
        ++Pal_sends;
    }
    return 1;
#endif
}

int
pal_logout()
{
    if(!Init)
        return 0;
    vc v(VC_VECTOR);
    v[0] = "logged-out";
    v[1] = My_UID;
    v[2] = vctrue;
    vc v2(VC_VECTOR);
    v2[0] = v;
    if(Pal_send_sock)
    {
        int count = 0;
        while(!Logout_ok && count++ < 4)
        {
            if(Pal_send_sock->can_write())
            {
                vc v3 = build_enc_packet(v2);
                if(v3.is_nil())
                    break;
                if(Pal_send_sock->sendvc(v3) == 0)
                    break;
            }
            poll_pal_socks(0, 500*1000);

        }
        ++Pal_sends;
    }
    Logout_ok = 0;
    Login_tries = 0;
    Pal_logged_in = 0;
    return 1;
}

int
pal_ping()
{
    if(!Init)
        return 0;
    vc v(VC_VECTOR);
    v[0] = "ping";
    v[1] = My_UID;
    int is_invisible();
    v[2] = is_invisible() ? "invis" : "vis";
    // note: invis got around 1/2011
    //v[2] = "vis";
    vc v2(VC_VECTOR);
    v2[0] = v;
    if(Pal_send_sock && Pal_send_sock->can_write())
    {
        vc v3 = build_enc_packet(v2);
        if(v3.is_nil())
            return 1;
        if(Pal_send_sock->sendvc(v3) == 0)
        {
            ++Total_failures;
            GRTLOG("ping send fail", 0, 0);
            total_reset();
        }
        ++Pal_sends;
    }
    return 1;
}

int
pal_tick()
{
    if(!Init)
        return 0;
//return 1;
    //if(Login_tries > MAX_LOGIN_TRIES)
    //	return 0;

// note: normally pal_tick is responsible for
// sending the initial login, and we don't want
// to do that until they have been logged into the
// server, and we have a valid pal_auth_state
//
//Wed Nov 17 14:37:47 MST 2010
// note: this used to be commented out for icuii and cdcx
// but there are race conditions related to the pals-only
// and ignore list processing in the servers when the client
// is accessing the pal server directly. for now, kluge
// until we can cause this info to go thru authenticated
// consistent channels

    extern int Auth_remote;
    if(!Auth_remote)
        return 0;

// end kluge
//
    if(Total_failures > MAX_TOTAL_FAILURES)
    {
        GRTLOG("total fail %d", Total_failures, 0);
        static int been_here;
        if(!been_here)
        {
            been_here = 1;
            //pal_login_failed(PAL_LIST_TOO_MANY_FAILURES);
        }
        exit_pal();
        return 0;
    }
    if(!Pal_login_timer.is_running() && !Stop_sending_login)
    {
        Pal_login_timer.start();
        //pal_login();
    }
    Pal_login_timer.tick();
    if(Pal_login_timer.is_expired())
    {
        Pal_login_timer.ack_expire();
        pal_login();
    }

    if(Stop_sending_login)
    {
        Pal_ping_timer.tick();
        if(Pal_ping_timer.is_expired())
        {
            Pal_ping_timer.ack_expire();
            pal_ping();
        }
    }
    poll_pal_socks(0, 0);
    return 1;
}

void
pal_reset()
{
    pal_logout();
    exit_pal();
    init_pal();
    pal_login();
}
#endif
