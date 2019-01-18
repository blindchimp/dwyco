
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// functions for bringing up a connection
// to a secondary server on demand when
// a message is q'd for it.
//
// note: originally, we thought we could
// encapsulate commands with auth info without
// creating new commands, so we have (auth-info (command))
// and the server would check the auth-info, and if it
// was ok, execute the command as if it were sent as
// in the old style.
// unfortunately, this causes some problems with
// response processing, because it is essentially
// two commands piggy-backed, and instead of
// exactly one response, there can be one of
// 3 or 4 possibilities (auth failed, command isn't done at all;
// auth ok, command fails; auth ok, command works).
// also, we don't want to simulate it here as if it
// was two commands (auth-info)(command) with the callbacks
// needed for two serial commands.
//
// we could automatically interpose a callback for
// processing a piggy-backed response (ie, the response
// would be (auth-resp (command-resp)) ) and then if the
// auth-resp was nil, the callback could generate a
// command-resp, but that would require it to know the
// possible error response forms for the command.
// *or* the server could provide an error response in
// the proper form... even if the auth failed.
//
//
#include "mmchan.h"
#include "qdirth.h"
#include "dwrtlog.h"
#include "dwstr.h"
#include "netvid.h"
#include "dhsetup.h"
#include "vcudh.h"
#include "dwscoped.h"
using namespace dwyco;

extern vc My_UID;

static void
hangup_all_servers()
{
    scoped_ptr<ChanList> cl(MMChannel::get_serviced_channels_server());
    ChanListIter i(cl.get());
    for(; !i.eol(); i.forward())
    {
        MMChannel *mc = i.getp();
        mc->schedule_destroy(MMChannel::HARD);
    }
}

static void
bounce_auth(vc m, void *v, vc chanid, ValidPtr)
{
    QckDone& d = *(QckDone *)v;
    // d is the QckDone from the original client
    // of the call... we need to q it and then
    // q the proper response from the server

    if(!m[1].is_nil())
    {
        // authentication succeeded...
        // extract the command response and
        // q that
        vc encap_response = m[1];
        Waitq.append(d);
        delete &d;
        Response_q.append(encap_response);
        return;
    }
    // either auth failed or couldn't get to server
    // the "Extras" field will contain the server
    // generated error response for the command...
    // or nil if we couldn't get to the server.
    if(m[2] == vc("bad auth"))
    {
        // try to reschedule getting some new
        // authentication info
        hangup_all_servers();
        goto generic_error_response;
    }
    // maybe we should check for timeouts here and redo the connection
    // for cases where routers have decided the connection was dead.
    // yes, we should, because we have to close it otherwise all subsequent
    // sends will fail.
    if(!m[3].is_nil())
    {
        Waitq.append(d);
        delete &d;
        Response_q.append(m[3]);
        return;
    }


generic_error_response:
    // couldn't get there... just bogus up a
    // response. also, drop the connection to that server since we'll
    // probably never be able to get back to it. this is caused by
    // crappy routers that leave connections up, but otherwise completely
    // unusable (sometimes due to the "firewall" built in that seems to
    // thing long open connections are some kind of attack.)
    MMChannel *mc = MMChannel::channel_by_id((int)chanid);
    if(mc)
    {
        mc->schedule_destroy(MMChannel::HARD);
        GRTLOG("toasted trashed channel %d", (int)chanid, 0);
        GRTLOGVC(m);
    }
    vc r(VC_VECTOR);
    r[0] = d.type.response_type();
    r[1] = vcnil;
    r[2] = m[2]; // error detail
    r[3] = vcnil;
    Waitq.append(d);
    delete &d;
    Response_q.append(r);
}


void
secondary_db_offline(MMChannel *mc, vc, void *, ValidPtr)
{
    // check to see if it has things q'd to send,
    // and if so, callbacks for failure should be
    // fired.
    for(int i = 0; i < mc->ctrl_q.num_elems(); ++i)
    {
        GRTLOG("punt q'd msgs", 0, 0);
        // have to figure out the way to get the
        // encapsulated type from this, otherwise
        // wrong thing is q's up

        // this q's response to "auth-command" wrappers
        // which will then bounce to a generic error
        // above.

        // note: when talking to server we may have
        // a "decrypt" command q'd up, which caused the
        // following to crash
        if(mc->ctrl_q[i].type() != VC_VECTOR)
            continue;
        vc bad_resp(VC_VECTOR);
        bad_resp.append(mc->ctrl_q[i][0]); // qckmsg type
        bad_resp.append(vcnil);
        bad_resp.append("server disconnected");
        GRTLOG("q'ing bad response", 0, 0);
        GRTLOGVC(bad_resp);
        Response_q.append(bad_resp);
    }
    GRTLOG("db offline", 0, 0);
}


static void
drop_connection_timeout(MMChannel *mc, vc, void *v, ValidPtr)
{
    mc->schedule_destroy(MMChannel::HARD);
    GRTLOG("timeout idle sec db", 0, 0);
}

static void
start_secondary_encryption(vc m, void *v, vc cmd, ValidPtr vp)
{
    QckMsg qm;
    qm.v = cmd;
    QckDone& d = *(QckDone *)v;
    // need this because in the callback, we don't have access to the
    // qckdone object which fired the callback. might want to change this
    // someday.
    d.arg2 = d.channel;

    MMChannel *mc = vp.is_valid() ? (MMChannel *)(void *)vp : 0;
    if(m[1].is_nil())
    {
        if(mc)
        {
            mc->agreed_key = vcnil;
            Waitq.append(d);
            MMChannel::send_to_db(qm, mc->myid);
            GRTLOG("sec db up (no enc)", 0, 0);
        }
        return;
    }
    if(!mc)
        return;
    mc->send_decrypt();
    mc->tube->set_key_iv(mc->agreed_key, 0);
    Waitq.append(d);
    MMChannel::send_to_db(qm, mc->myid);
    GRTLOG("sec db up (enc)", 0, 0);
}

static void
secondary_db_online(MMChannel *mc, vc m, void *v, ValidPtr)
{
    mc->timer1.stop();
    mc->destroy_callback = secondary_db_offline;
    mc->ctrl_timer_callback = drop_connection_timeout;
    mc->ctrl_timer.reset();
    mc->ctrl_timer.load(SECONDARY_CHANNEL_DROP_TIMEOUT);
    mc->ctrl_timer.set_oneshot(1);
    mc->ctrl_timer.start();

    // setup encryption, old servers will error on the dhsf
    // command so we can at least fall back on the old servers to
    // unencrypted mode.
    QckMsg em;
    QckDone ed(start_secondary_encryption, v, m, mc->vp);
    vc material = dh_store_and_forward_material(Server_publics, mc->agreed_key);
    em = dirth_get_setup_session_key_cmd(My_UID, material, ed);
    Waitq.append(ed);
    MMChannel::send_to_db(em, mc->myid);

#if 0
    QckMsg qm;
    qm.v = m;
    QckDone& d = *(QckDone *)v;
    // need this because in the callback, we don't have access to the
    // qckdone object which fired the callback. might want to change this
    // someday.
    d.arg2 = d.channel;
    Waitq.append(d);
    MMChannel::send_to_db(qm, mc->myid);
    GRTLOG("sec db up", 0, 0);
#endif
}

static void
secondary_db_call_failed_last(MMChannel *mc, vc m, void *v, ValidPtr)
{
    // callback on this context to say send failed
    //...//
    mc->timer1.stop();
    QckDone& d = *(QckDone *)v;
    // note: this q the action then q's the response immediately
    // so the qckdone gets fired on the next response poll
    Waitq.append(d);
    vc bad_resp(VC_VECTOR);
    bad_resp.append(d.type.response_type()); // qckmsg type
    bad_resp.append(vcnil);
    bad_resp.append("can't connect to server");
    Response_q.append(bad_resp);
    GRTLOG("sec db call failed last", 0, 0);
}

static void
connect_timeout(MMChannel *mc, vc, void *v, ValidPtr)
{
    mc->schedule_destroy(MMChannel::HARD);
    GRTLOG("timeout init con to sec db", 0, 0);
#if 0
    QckDone& d = *(QckDone *)v;
    vc bad_resp(VC_VECTOR);
    bad_resp.append(d.type.response_type()); // qckmsg type
    bad_resp.append(vcnil);
    bad_resp.append("can't connect to server (timeout).");
    Response_q.append(bad_resp);
    GRTLOG("sec db call failed timeout", 0, 0);
#endif
}

static void
secondary_db_call_failed(MMChannel *md, vc m, void *v, ValidPtr)
{

    QckDone& d = *(QckDone *)v;     // LEAK

    //if(md->resolve_failed)
    //{
    MMChannel *mc = MMChannel::start_server_channel(
                        MMChannel::BYADDR,
#ifdef LOCAL_TEST
                        inet_addr("204.75.209.40"),
#else
                        inet_addr(m[0]),
#endif
                        0,
                        m[1]);
    if(!mc)
    {
        goto fail;
    }
    d.channel = mc->myid; // so we can figure out which server failed if it times out
    mc->established_callback = secondary_db_online;
    mc->ecb_arg1 = m[2]; // the msg to send
    mc->ecb_arg2 = new QckDone(d); //md->ecb_arg2; LEAK
    mc->destroy_callback = secondary_db_call_failed_last;
    mc->dcb_arg1 = m; //md->dcb_arg1;
    mc->dcb_arg2 = new QckDone(d); //md->dcb_arg2; LEAK
    mc->set_string_id("Dwyco Secondary Database");
    mc->secondary_server_channel = 1;
    mc->attempt_ip = m[4]; //md->attempt_ip;
    mc->attempt_port = m[5]; //md->attempt_port;
    mc->attempt_name = m[3]; //md->attempt_name;
    mc->timer1_callback = connect_timeout;
    mc->t1cb_arg2 = new QckDone(d); // LEAK
    mc->timer1.set_oneshot(1);
    mc->timer1.load(6 * 1000);
    mc->timer1.reset();
    mc->timer1.start();
    GRTLOG("sec sec db resolve failed", 0, 0);
    return;
    //}
fail:
    vc bad_resp(VC_VECTOR);
    bad_resp.append(d.type.response_type()); // qckmsg type
    bad_resp.append(vcnil);
    bad_resp.append("can't connect to server.");
    Response_q.append(bad_resp);
    GRTLOG("sec db call failed", 0, 0);
}


static
int
start_secondary_database_thread(vc name, vc ip, vc port, QckMsg m, QckDone d)
{
    GRTLOG("sec trying", 0 , 0);
    GRTLOGVC(name);
    GRTLOGVC(port);
    vc v(VC_VECTOR);
    v[0] = ip;
    v[1] = port;
    // this is bad
    v[2] = m.v;
    v[3] = name;
    v[4] = ip;
    v[5] = port;
    // note: it takes too long to do the DNS crap
    // in some cases, so we just go straigh for IP now.
#if 0
    MMChannel *mc = MMChannel::start_server_channel(
                        MMChannel::BYNAME,
                        0,
#ifdef LOCAL_TEST
                        "shinbiter.dwyco.com",
#else
                        name,
#endif
                        port);
    if(!mc)
    {
#endif
        secondary_db_call_failed(0, v, new QckDone(d), ValidPtr());
        return 1;
#if 0
    }
    mc->established_callback = secondary_db_online;
    mc->ecb_arg1 = m.v; //this is bad
    mc->ecb_arg2 = new QckDone(d);
    mc->destroy_callback = secondary_db_call_failed;
    mc->secondary_server_channel = 1;
    mc->attempt_name = name;
    mc->attempt_ip = ip;
    mc->attempt_port = port;
    mc->dcb_arg1 = v;
    mc->dcb_arg2 = new QckDone(d);

    mc->set_string_id("Dwyco Secondary Database");
    GRTLOG("secondary db start", 0, 0);
    return 1;
#endif
}

static
MMChannel *
already_connected_secondary(vc ip, vc port)
{
    scoped_ptr<ChanList> cl(MMChannel::get_serviced_channels_server());
    ChanListIter i(cl.get());
    DwString a = (const char *)ip;
    DwString b = port.peek_str();
    a += ":";
    a += b;
    for(; !i.eol(); i.forward())
    {
        MMChannel *mc = i.getp();
        if(mc->call_setup &&
                ip == mc->attempt_ip && port == mc->attempt_port)
        {

            return mc;
        }
        if(mc->tube && !mc->tube->is_connected())
            continue;
        if(!mc->tube)
            continue;
        if(vc(a.c_str()) == mc->tube->remote_addr_ctrl())
        {

            return mc;
        }
    }

    return 0;
}


// note: there is no need to q the messages up
// at this point, because all of them to secondary
// servers require a response. so we can just jam
// them all into the channel and wait for the
// responses to come back as usual.
//
int
send_to_secondary(vc name, vc ip, vc port, QckMsg m, QckDone d)
{
    MMChannel *mc = already_connected_secondary(ip, port);
    QckDone d2(bounce_auth, new QckDone(d));
    d2.type = ReqType("auth-command", d.type.serial);
    // we put a timeout on the auth-command, even tho there
    // may also be a timeout on the encapsulated command.
    // NOTE: all the timeouts are running concurrently, so
    // take that into account when you set the timeouts


    if(mc)
    {
        d2.set_timeout(5);
        d2.channel = mc->myid; // so we can figure out which server we actually sent it to if it fails
        d2.arg2 = d2.channel;
        Waitq.append(d2);
        MMChannel::send_to_db(m, mc->myid);
        return 1;
    }
    // interpose a QckDone event between the one that
    // comes in here
    // give it a little more time to connect
    d2.set_timeout(15);
    // note: wait until we are actually connected to put it on the Waitq
    if(!start_secondary_database_thread(name, ip, port, m, d2))
        return 0;
    return 1;
}

