
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/qdirth.cc 1.9 1999/01/10 16:09:50 dwight Checkpoint $

#include "dirth.h"
#include "qdirth.h"
#include "dwvec.h"
#include "dwrtlog.h"
#include "dwstr.h"
#include "mmchan.h"
#include "cdcver.h"
#include "dhsetup.h"
#include "dhgsetup.h"
#include "vcudh.h"
#include "qmsgsql.h"

using namespace dwyco;

void clear_local_ignore();
void add_local_ignore(vc uid);
void del_local_ignore(vc uid);
vc get_local_ignore();

int send_to_secondary(vc ip, vc port, QckMsg m, QckDone d);
vc to_hex(vc);
vc vclh_serialize(vc&);
vc vclh_sha(vc);
extern int Database_id;
extern int Chat_id;

namespace dwyco {
DwListA<QckDone> Waitq;
DwListA<QckDone> PWaitq;
DwListA<vc> Response_q;
int ReqType::Serial;

// this is used to figure out if there is a bulk operation
// in progress and we want to eliminate timeouts
// temporarily.
static int Enable_timeout;

static vc
reqtype(const char *name, const QckDone& d)
{
    vc v(VC_VECTOR);
    v.append(name);
    v.append(d.type.serial);
    return v;
}



static void
dirth_send(const QckMsg& m, QckDone& d)
{
    static vc cs1("create-user-lobby");
    static vc cs2("remove-user-lobby");
    static vc dhsf("dhsf");

    vc what = m[QTYPE][0];
    if(Enable_timeout)
        d.set_timeout(30);
    d.time_qed = time(0);
    if(what == cs1 || what == cs2)
    {
        d.channel = Chat_id;
        Waitq.append(d);
        MMChannel::send_to_db(m, Chat_id);
    }
    else
    {
        d.channel = Database_id;
        Waitq.append(d);
        // don't allow anything to happen until crypto is up
        if(what == dhsf)
        {
            MMChannel::send_to_db(m, Database_id);
            return;
        }
        if(!Database_online)
        {
            dirth_simulate_error_response(d);
            return;
        }
        // send it to the database server
        MMChannel::send_to_db(m, Database_id);
    }
}


vc
dirth_get_response()
{
    vc ret;

    if(Response_q.num_elems() > 0)
    {
        ret = Response_q.get_first();
        Response_q.remove_first();
    }
    return ret;
}

int
dirth_pending_callbacks(QDFUNCP f, void *arg1, const ReqType& type, const vc& arg2)
{
    const QckDone *wp;
    dwlista_foreach_peek_back(wp, Waitq)
    {
        const QckDone& w = *wp;
        //GRTLOG("PEND %s", (char *)Waitq[i].type, 0);
        if(w.callback == f)
        {
            if(type.name.is_nil() || w.type == type)
            {
                if(arg1 == 0 || w.arg1 == arg1)
                {
                    if(arg2.is_nil() || w.arg2 == arg2)
                    {
                        GRTLOG("pend %s", (char *)w.type, 0);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

void
dirth_cancel_callbacks(QDFUNCP f, void *arg1, const ReqType& type)
{
    Waitq.rewind();
    while(!Waitq.eol())
    {
        const QckDone& w = *Waitq.peek_ptr();
        if(w.callback == f)
        {
            if(type.name.is_nil() || (!type.name.is_nil() && w.type == type))
            {
                if(arg1 == 0 || (arg1 != 0 && w.arg1 == arg1))
                {
                    GRTLOG("cancel %s", (char *)w.type, 0);
                    Waitq.remove();
                    continue;
                }
            }
        }
        Waitq.forward();
    }
}

void
dirth_cancel_callbacks(QDFUNCP f, ValidPtr vp, const ReqType& type)
{
    int vpvalid = vp.is_valid();
    Waitq.rewind();
    while(!Waitq.eol())
    {
        const QckDone& w = *Waitq.peek_ptr();
        if(w.callback == f)
        {
            if(type.name.is_nil() || w.type == type)
            {
                if(vpvalid == 0 || w.vp == vp)
                {
                    GRTLOG("cancel %s", (char *)w.type, 0);
                    Waitq.remove();
                    continue;
                }
            }
        }
        Waitq.forward();
    }
}

// this function should be called when a channel
// is destroyed. it fabricates error responses for
// all the callbacks that are q'd for the channel.
// this ensures that all the callbacks will be fired
// for the channel.
void
dirth_dead_channel_cleanup(int chan)
{
    const QckDone *wp;
    dwlista_foreach_peek(wp, Waitq)
    {
        if(wp->channel == chan)
        {
            vc resp(VC_VECTOR);
            resp.append(wp->type.response_type());
            // note: rest of response will look like
            // a generic error
            resp.append(vcnil);
            resp.append("server error");
            Response_q.append(resp);
        }
    }
}

// when canceling a server operation, call this function
// instead of the "cancel callbacks" functions above.
// it fabricates error responses for
// the scheduled callback so anything waiting will get
// what looks like an error response.
void
dirth_simulate_error_response(const QckDone& q)
{
    const QckDone *wp;
    dwlista_foreach_peek(wp, Waitq)
    {
        if(*wp == q)
        {
            GRTLOG("simul err %s", (char *)wp->type, 0);
            vc resp(VC_VECTOR);
            resp.append(wp->type.response_type());
            // note: rest of response will look like
            // a generic error
            resp.append(vcnil);
            resp.append("server error");
            Response_q.append(resp);
        }
    }
}

static
int
find_in_q(const vc& v, DwListA<QckDone>& waitq)
{
    int ret = 0;
    waitq.rewind();
    while(!waitq.eol())
    {
        const QckDone *wp;
        wp = waitq.peek_ptr();
        GRTLOG("wq %s", (char *)wp->type, 0);
        if(wp->type.name == v[0][0] && wp->type.serial == (int)v[0][1])
        {
            QckDone q = *wp;
            if(q.permanent == QckDone::TRANSIENT)
                waitq.remove();
            // note: as long as you don't reference the waitq after this
            // it is safe for callbacks from "done" to call more commands
            // that might q more commands
            q.done(v);
            if(q.time_qed != -1)
            {
                if(time(0) - q.time_qed > 15)
                {
                    Enable_timeout = 0;
                }
                else
                    Enable_timeout = 1;
            }

            ret = 1;
            break;
        }
        waitq.forward();
    }
    return ret;
}

int
dirth_poll_response()
{
    vc v;
    int ret = 0;
    for(v = dirth_get_response(); !v.is_nil(); v = dirth_get_response())
    {
        GRTLOGVC(v);
        if(v.type() != VC_VECTOR || v[0].type() != VC_VECTOR ||
                v[0][0].type() != VC_STRING || v[0][1].type() != VC_INT)
        {
            // hmmm, this happens in the field a bit, not a lot, which
            // makes me think it is due to some sporadic network issue.
            // so instead of crashing, just drop the response
            //oopanic("debug proto fail");
            continue;
        }
        if(find_in_q(v, Waitq))
            ret = 1;
        else if(find_in_q(v, PWaitq))
            ret = 1;
    }
    return ret;
}

void
dirth_poll_timeouts()
{
    const QckDone *wp;
    dwlista_foreach_peek(wp, Waitq)
    {
        if(wp->expired())
        {
            GRTLOG("wq expired %s", (char *)wp->type, 0);
            const QckDone& q = *wp;

            vc resp(VC_VECTOR);
            resp.append(q.type.response_type());
            // note: rest of response will look like
            // a generic error
            resp.append(vcnil);
            resp.append("server timeout");
            Response_q.append(resp);
        }
    }
}

// q up an action that doesn't go to the server, but
// instead is just asynchronously called as if the
// server had sent a response. used to process local-only
// message processing more easily.

void
dirth_q_local_action(vc response_vector, QckDone d)
{
    d.type = ReqType("local");
    vc type = reqtype("local", d);
    Waitq.append(d);
    // now stick a response in the response q to
    // get this request picked up and fired off
    vc resp(VC_VECTOR);
    resp.append(type);
    int n = response_vector.num_elems();
    for(int i = 0; i < n; ++i)
        resp.append(response_vector[i]);
    Response_q.append(resp);
}


void
dirth_send_new4(vc id, vc handle, vc email, vc user_spec_id, vc pw, vc pal_auth, QckDone d)
{
    QckMsg m;

    d.type = ReqType("new4");
    m[QTYPE] = reqtype("new4", d);
    m[QFROM] = id;
    m[QHANDLE] = handle;
    m[QEMAIL] = email;
    m[QUSER_SPECED_ID] = user_spec_id;
    m[QPW] = vclh_sha(pw);
    //m[QPAL_AUTH] = pal_auth;
    m[QSTUFF] = build_directory_entry2();
    m[QIGNORE_LIST] = get_local_ignore();
    vc static_public = dh_my_static();
    static_public = static_public[DH_STATIC_PUBLIC];
    m[QSTATIC_PUBLIC] = static_public;
    if(Current_alternate)
    {
        static_public = Current_alternate->my_static_public();
        m[QSTATIC_PUBLIC_ALTERNATE] = static_public[DH_STATIC_PUBLIC];
        m[QSTATIC_ALT_NAME] = Current_alternate->alt_name();
    }
    dirth_send(m, d);
}

// this gets the public keys (including the group pk, if the
// user is currently in a group) for a uid.
void
dirth_send_get_pk(vc id, vc uid, QckDone d)
{
    QckMsg m;

    d.type = ReqType("get-pk");
    m[QTYPE] = reqtype("get-pk", d);
    m[QFROM] = id;
    m[2] = uid;
    dirth_send(m, d);
}

// this is a kluge for "selfs", map handle to
// uid. if handle isn't unique, uid returned
// is a random one. don't use this.
void
dirth_send_get_uid(vc id, vc handle, QckDone d)
{
    QckMsg m;

    d.type = ReqType("get-uid");
    m[QTYPE] = reqtype("get-uid", d);
    m[QFROM] = id;
    m[2] = handle;
    Waitq.append(d);
    dirth_send(m, d);
}

// set the google FCM token
void
dirth_send_set_token(vc id, vc token, QckDone d)
{
    QckMsg m;

    d.type = ReqType("set-token");
    m[QTYPE] = reqtype("set-token", d);
    m[QFROM] = id;
    m[2] = token;
    dirth_send(m, d);
}

// this asks for the current list of uid's that are in
// the group we are currently logged in with
void
dirth_send_get_group(vc id, QckDone d)
{
    QckMsg m;

    d.type = ReqType("get-group");
    m[QTYPE] = reqtype("get-group", d);
    m[QFROM] = id;
    dirth_send(m, d);
}

// this gets the group pk from the name.
// used for bootstrapping into a group if you
// are brand new and don't know anyone else
// in the group but you do know the group name.
// you also get the uid's in the group.
void
dirth_send_get_group_pk(vc id, vc gname, QckDone d)
{
    QckMsg m;

    d.type = ReqType("get-group-pk");
    m[QTYPE] = reqtype("get-group-pk", d);
    m[QFROM] = id;
    m[2] = gname;
    dirth_send(m, d);
}

// attempt to create/enter group gname.
// prov_pk is the provisional public static group key
// in case the group doesn't exist.
// if the group already exists, this will return the key for the
// group directly, essentially ignoring the key you sent in.
//
// if the group does not exist, a challenge is returned, which
// we must decrypt and issue another command (below)
//
void
dirth_send_set_get_group_pk(vc id, vc gname, vc prov_pk, QckDone d)
{
    QckMsg m;

    d.type = ReqType("set-get-group-pk");
    m[QTYPE] = reqtype("set-get-group-pk", d);
    m[QFROM] = id;
    m[2] = gname;
    m[3] = prov_pk[DH_STATIC_PUBLIC];

    dirth_send(m, d);
}

// after you have decrypted the challenge nonce, you send it
// back with this command. the server will answer with the group public
// key and signature. (this just makes sure you have the private key
// associated with the group and aren't asking us to just sign
// some random stuff.)
// NOTE: the server provisionally puts you in the group
// if you succeed with the challenge. this helps make it a little less
// confusing because group observers will get a message that you are
// in the group before you login the next time. (it is assumed the
// client will exit pretty quickly after succeeding in the challenge.)
// note this provisional assignment isn't really a problem if it turns out
// to be bogus, because you would be the only one in the group (ie, it is
// brand new.)
void
dirth_send_group_chal(vc id, vc nonce, QckDone d)
{
    QckMsg m;

    d.type = ReqType("group-chal");
    m[QTYPE] = reqtype("group-chal", d);
    m[QFROM] = id;
    m[2] = nonce;

    dirth_send(m, d);
}

// this is a command you can do *after* you have
// deleted all the local information associated with
// group sync state. it tells the server to record the
// fact you have left the group, rather than waiting until you
// login the next time (which may be never.)
void
dirth_send_prov_leave(vc id, QckDone d)
{
    QckMsg m;

    d.type = ReqType("prov-leave");
    m[QTYPE] = reqtype("prov-leave", d);
    m[QFROM] = id;
    // note: we need to exit fairly quickly in this case, and it isn't
    // crucial the command completes, so just give it a few seconds
    d.set_timeout(5);

    dirth_send(m, d);
}

void
dirth_send_get(vc id, vc which, QckDone d)
{
    QckMsg m;

    d.type = ReqType("get");
    m[QTYPE] = reqtype("get", d);
    m[QFROM] = id;
    m[2] = which;
    dirth_send(m, d);
}

void
dirth_send_get2(vc id, vc which, QckDone d)
{
    QckMsg m;

    d.type = ReqType("get2");
    m[QTYPE] = reqtype("get2", d);
    m[QFROM] = id;
    m[2] = which;
    dirth_send(m, d);
}

void
dirth_send_store(vc id, vc recipients, vc msg, vc no_group, vc no_self, QckDone d)
{
    QckMsg m;

    d.type = ReqType("store");
    m[QTYPE] = reqtype("store", d);
    m[QFROM] = id;
    m[QSTORE_RECIPIENTS] = recipients;
    m[QSTORE_MSG] = msg;
    m[QSTORE_NO_GROUP] = no_group;
    m[QSTORE_NO_SELF] = no_self;
    if(recipients.num_elems() == 1)
    {
        vc port;
        vc ip = get_server_ip_by_uid(recipients[0], port);
        vc name = get_server_name_by_uid(recipients[0], port);
        if(ip == My_server_ip && port == My_server_port)
        {
            dirth_send(m, d);
        }
        else
        {
            // send to secondary server
            vc mm = generate_mac_msg(m.v);
            // note: response is to *encapsulated* command
            // so we don't update d
            // note: auth-command response will bounce to the
            // the encapsulated command's callback

            QckMsg m2;
            m2[QTYPE] = reqtype("auth-command", d);
            m2[QFROM] = id;
            m2[2] = mm;
            if(mm.is_nil() || !send_to_secondary(ip, port, m2, d))
            {
                vc resp(VC_VECTOR);
                resp[0] = vcnil;
                dirth_q_local_action(resp, d);
                return;
            }
        }
        return;
    }
    oopanic("multiple recipients not supported using this command anymore");
}

void
dirth_send_query(vc uid, QckDone d)
{
    QckMsg m;

    d.type = ReqType("query");
    m[QTYPE] = reqtype("query", d);
    m[QFROM] = uid;
    dirth_send(m, d);
}

void
dirth_send_query2(vc uid, QckDone d)
{
    QckMsg m;

    d.type = ReqType("query2");
    m[QTYPE] = reqtype("query2", d);
    m[QFROM] = uid;
    dirth_send(m, d);
}

// NOTE: this is now "delete the message for everyone in group"
void
dirth_send_ack_get(vc uid, vc mid, QckDone d)
{
    QckMsg m;

    d.type = ReqType("ack-get");
    m[QTYPE] = reqtype("ack-get", d);
    m[QFROM] = uid;
    m[2] = mid;
    dirth_send(m, d);
}

// NOTE: contrast to ack_get: ack_get2 is "don't return this
// message to me anymore, but let others in the same group see it"
void
dirth_send_ack_get2(vc uid, vc mid, QckDone d)
{
//    // NOTE: in all cases, if you are not in a group, deleting a message
//    // means just delete it, so there is no need to do the group-based
//    // filtering.
//    if(!Current_alternate)
//        return dirth_send_ack_get(uid, mid, d);

    QckMsg m;

    d.type = ReqType("ack-get2");
    m[QTYPE] = reqtype("ack-get2", d);
    m[QFROM] = uid;
    m[2] = mid;
    dirth_send(m, d);
    sql_add_tag(mid, "_ack");
}

void
dirth_send_addtag(vc uid, vc mid, vc tag, QckDone d)
{
    QckMsg m;

    d.type = ReqType("addtag");
    m[QTYPE] = reqtype("addtag", d);
    m[QFROM] = uid;
    m[2] = mid;
    m[3] = tag;
    dirth_send(m, d);
}

void
dirth_send_ignore(vc id, vc uid, QckDone d)
{
    QckMsg m;

// NOTE: in this version, we assume only local
// ignore lists are used. we don't send commands
// to the server to store the ignore info.
// note: in cdcx, we send the ignore in, mainly to
// update the asshole factor, nothing else is
// updated

    d.type = ReqType("ignore");
    m[QTYPE] = reqtype("ignore", d);
    m[QFROM] = id;
    m[2] = uid;
    dirth_send(m, d);
}

void
dirth_send_unignore(vc id, vc uid, QckDone d)
{
    QckMsg m;
// NOTE: in this version, we assume only local
// ignore lists are used. we don't send commands
// to the server to store the ignore info.

    d.type = ReqType("unignore");
    m[QTYPE] = reqtype("unignore", d);
    m[QFROM] = id;
    m[2] = uid;
    dirth_send(m, d);
}

// note: this should go to the server of the *recipient* of the
// ignore.
void
dirth_send_ignore_count(vc id, vc uid, vc delta, QckDone d)
{
    QckMsg m;

    d.type = ReqType("ignore-count");
    m[QTYPE] = reqtype("ignore-count", d);
    m[QFROM] = id;
    m[2] = uid;
    m[3] = delta;

    vc who = uid;
    vc port;
    vc ip = get_server_ip_by_uid(who, port);
    vc name = get_server_name_by_uid(who, port);
    if(ip == My_server_ip && port == My_server_port)
    {
        dirth_send(m, d);
    }
    else
    {
        // send to secondary server
        vc mm = generate_mac_msg(m.v);
        // note: auth-command response will bounce to the
        // the encapsulated command's callback

        QckMsg m2;
        m2[QTYPE] = reqtype("auth-command", d);
        m2[QFROM] = id;
        m2[2] = mm;
        if(mm.is_nil() || !send_to_secondary(ip, port, m2, d))
        {
            vc resp(VC_VECTOR);
            resp[0] = vcnil;
            dirth_q_local_action(resp, d);
            return;
        }
    }
}

#if 0
void
dirth_send_get_ignore(vc id, QckDone d)
{
    QckMsg m;

// NOTE: in this version, we assume only local
// ignore lists are used. we don't send commands
// to the server to store the ignore info.

    vc resp(VC_VECTOR);
    resp[0] = get_local_ignore();
    dirth_q_local_action(resp, d);
    return;

#if 0
    d.type = ReqType("get-ignore");
    m[QTYPE] = reqtype("get-ignore", d);
    m[QFROM] = id;
    dirth_send(m, d);
#endif
}
#endif


void
dirth_send_ack_all(vc id, QckDone d)
{
    QckMsg m;

    d.type = ReqType("ack-all");
    m[QTYPE] = reqtype("ack-all", d);
    m[QFROM] = id;
    dirth_send(m, d);
}

#if 0
void
dirth_send_clear_ignore(vc id, QckDone d)
{
    QckMsg m;

    d.type = ReqType("clear-ignore");
    m[QTYPE] = reqtype("clear-ignore", d);
    m[QFROM] = id;
    dirth_send(m, d);
}
#endif


void
dirth_send_get_server_list2(vc id, QckDone d)
{
    QckMsg m;

    d.type = ReqType("get-server-list2");
    m[QTYPE] = reqtype("get-server-list2", d);
    m[QFROM] = id;
    dirth_send(m, d);
}

QckMsg
dirth_get_setup_session_key_cmd(vc id, vc sf_material, QckDone& d)
{
    QckMsg m;

    d.type = ReqType("dhsf");
    m[QTYPE] = reqtype("dhsf", d);
    m[QFROM] = id;
    m[2] = sf_material;
    return m;
}

void
dirth_send_setup_session_key(vc id, vc sf_material, QckDone d)
{
    QckMsg m;

    d.type = ReqType("dhsf");
    m[QTYPE] = reqtype("dhsf", d);
    m[QFROM] = id;
    m[2] = sf_material;
    dirth_send(m, d);
}

void
dirth_send_set_interest_list(vc id, vc list, QckDone d)
{
    QckMsg m;

    d.type = ReqType("set-interest");
    m[QTYPE] = reqtype("set-interest", d);
    m[QFROM] = id;
    m[2] = list;
    dirth_send(m, d);
}


void
dirth_send_debug(vc id, vc crashed, vc stack, vc field_track, QckDone d)
{
    QckMsg m;

    d.type = ReqType("debug");
    m[QTYPE] = reqtype("debug", d);
    m[QFROM] = id;
    m[2] = crashed;
    m[3] = stack;
    m[4] = field_track;
    dirth_send(m, d);
}

void
dirth_send_recommend2(vc id, vc from, vc to, QckDone d)
{
    QckMsg m;

    d.type = ReqType("recommend2");
    m[QTYPE] = reqtype("recommend2", d);
    m[QFROM] = id;
    m[2] = from;
    m[3] = to;
    dirth_send(m, d);
}

void
dirth_send_recommend3(vc id, vc from, vc to, vc email, QckDone d)
{
    QckMsg m;

    d.type = ReqType("recommend3");
    m[QTYPE] = reqtype("recommend3", d);
    m[QFROM] = id;
    m[2] = from;
    m[3] = to;
    m[4] = email;
    dirth_send(m, d);
}

void
dirth_send_set_state(vc id, vc state, QckDone d)
{
    QckMsg m;

    d.type = ReqType("set-state");
    m[QTYPE] = reqtype("set-state", d);
    m[QFROM] = id;
    m[2] = state;
    dirth_send(m, d);
}

vc
generate_mac_msg(vc m)
{
    if(Current_session_key.is_nil() ||
            Current_authenticator.is_nil())
        return vcnil;

    vc smsg = vclh_serialize(m);
    DwString s((const char *)Current_session_key, 0, Current_session_key.len());
    s += DwString((const char *)smsg, 0, smsg.len());
    s += DwString((const char *)Current_authenticator, 0, Current_authenticator.len());
    vc mac = vclh_sha(vc(VC_BSTRING, s.c_str(), s.length()));
    mac = vc(VC_BSTRING, (const char *)mac, 12);

    vc v(VC_VECTOR);
    v[0] = smsg;
    v[1] = Current_authenticator;
    v[2] = mac;
    return v;
}

// note: same as set/get profile, only gets sent to msg server
void
dirth_send_get_info(vc id, vc uid, vc cache_check, QckDone d)
{
    QckMsg m;
    d.type = ReqType("get-info");
    m[QTYPE] = reqtype("get-info", d);
    m[QFROM] = id;
    m[2] = uid;
    // use authenticator as a hash, server will send back
    // indication if our item is current
    m[3] = cache_check;
    dirth_send(m, d);
}

void
dirth_send_set_info(vc id, vc info, QckDone d)
{
    QckMsg m;

    d.type = ReqType("set-info");
    m[QTYPE] = reqtype("set-info", d);
    m[QFROM] = id;
    m[2] = info;
    dirth_send(m, d);
}

void
dirth_send_check_for_update(vc id, QckDone d)
{
    QckMsg m;

    d.type = ReqType("check-update");
    m[QTYPE] = reqtype("check-update", d);
    m[QFROM] = id;
    // send in a version number
    // hash value of the main executable
    m[2] = dwyco_get_version_string();
    m[3] = "";
    dirth_send(m, d);
}

#if 0
void
dirth_send_get_pal_auth_state(vc id, vc who, QckDone d)
{
    QckMsg m;

    d.type = ReqType("get-pal-auth");
    m[QTYPE] = reqtype("get-pal-auth", d);
    m[QFROM] = id;
    m[2] = who;

    vc port;
    vc ip = get_server_ip_by_uid(who, port);
    vc name = get_server_name_by_uid(who, port);
    if(ip == My_server_ip && port == My_server_port)
    {
        dirth_send(m, d);
    }
    else
    {
        // send to secondary server
        vc mm = generate_mac_msg(m.v);
        // note: auth-command response will bounce to the
        // the encapsulated command's callback

        QckMsg m2;
        m2[QTYPE] = reqtype("auth-command", d);
        m2[QFROM] = id;
        m2[2] = mm;
        if(mm.is_nil() || !send_to_secondary(name, ip, port, m2, d))
        {
            vc resp(VC_VECTOR);
            resp[0] = vcnil;
            dirth_q_local_action(resp, d);
            return;
        }
    }
}

void
dirth_send_set_pal_auth_state(vc id, vc state, QckDone d)
{
    QckMsg m;

    d.type = ReqType("set-pal-auth");
    m[QTYPE] = reqtype("set-pal-auth", d);
    m[QFROM] = id;
    m[2] = state;
    dirth_send(m, d);
}
#endif

void
dirth_send_server_assist(vc id, vc to_id, QckDone d)
{
    QckMsg m;

    d.type = ReqType("server-assist");
    m[QTYPE] = reqtype("server-assist", d);
    m[QFROM] = id;
    m[2] = to_id;

    vc port;
    vc ip = get_server_ip_by_uid(to_id, port);
    vc name = get_server_name_by_uid(to_id, port);
    if(ip == My_server_ip && port == My_server_port)
    {
        dirth_send(m, d);
    }
    else
    {
        // send to secondary server
        vc mm = generate_mac_msg(m.v);
        // note: auth-command response will bounce to the
        // the encapsulated command's callback

        QckMsg m2;
        m2[QTYPE] = reqtype("auth-command", d);
        m2[QFROM] = id;
        m2[2] = mm;
        if(mm.is_nil() || !send_to_secondary(ip, port, m2, d))
        {
            vc resp(VC_VECTOR);
            resp[0] = vcnil;
            dirth_q_local_action(resp, d);
            return;
        }
    }
}

// NOTE: this is sent to the chat server rather than the directory
// or the database server.
void
dirth_send_create_user_lobby(vc id, vc dispname, vc category, vc sub_god_uid, vc pw, vc user_limit, QckDone d)
{
    QckMsg m;

    d.type = ReqType("create-user-lobby");
    m[QTYPE] = reqtype("create-user-lobby", d);
    m[QFROM] = id;
    m[2] = dispname;
    m[3] = category;
    m[4] = sub_god_uid;
    m[5] = pw;
    m[6] = user_limit;
    // this hack keeps us from creating multiple lobbies in
    // rapid succession, most of which have identical names.
    // this is really a UI issue, but for now we just hack around it.
    // ok, this isn't exactly right, we really need to see if
    // there is *any* create-user-lobby command q-d up, but
    // we don't have a function that checks that right now, so for
    // this hack, we just assume the callback is unique
    if(dirth_pending_callbacks(d.callback, 0, ReqType(), vcnil))
    {
        vc resp(VC_VECTOR);
        resp[0] = vcnil;
        dirth_q_local_action(resp, d);
        return;
    }

    dirth_send(m, d);
}

void
dirth_send_remove_user_lobby(vc id, vc lobby_id, QckDone d)
{
    QckMsg m;

    d.type = ReqType("remove-user-lobby");
    m[QTYPE] = reqtype("remove-user-lobby", d);
    m[QFROM] = id;
    m[2] = lobby_id;

    dirth_send(m, d);
}
}
