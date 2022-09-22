
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "qmsgsql.h"
#undef LOCAL_TEST
#include "vc.h"
#include "qmsg.h"
#include "qdirth.h"
#include "dwrtlog.h"
#include "dwstr.h"
#include "cdcpal.h"
#include "dwrtlog.h"
#include "se.h"
#include "qauth.h"
#include "simple_property.h"
#include "ezset.h"
#include "dirth.h"
#include "dhgsetup.h"

using namespace dwyco;

extern vc My_UID;
extern vc Online;
extern vc Client_disposition;
extern vc Client_ports;

int is_invisible();

namespace dwyco {
// not perfect if packets are dropped, the
// response we get may not match, but it isn't
// fatal
static vc Last_sent;
static int Init;

int Pal_logged_in;
int Inhibit_pal;

// note: this stuff is kinda bogus because
// it assumes the pal list doesn't change
// at all. if it does, there is a risk the
// online info will be slightly wrong, but
// this is a best-guess protocol anyways, so
// it is no big deal.
static
vc
tree_to_vec(const DwTreeKaz<int, vc>& tr)
{

    vc ret(VC_VECTOR);
    DwTreeKazIter<int, vc> i(&tr);
    for(; !i.eol(); i.forward())
    {
        auto kv = i.get();
        ret.append(kv.get_key());
    }
    return ret;
}

static
vc
transient_online_list()
{
    DwTreeKaz<int, vc> tmpl(0);

    vc p = pal_to_vector(0);
    // note that p may contain group representatives, but we
    // really need the group uid's explicitly since we are going
    // to use the info try and target message delivery.
    for(int i = 0; i < p.num_elems(); ++i)
    {
        const vc uids = map_uid_to_uids(p[i]);
        for(int i = 0; i < uids.num_elems(); ++i)
            tmpl.add(uids[i], 0);
    }
    const vc gv = Group_uids;
    if(!gv.is_nil())
    {
        for(int i = 0; i < gv.num_elems(); ++i)
        {
            tmpl.add(gv[i], 0);
        }
    }
    // it is debatable whether having notifications of online
    // for others in your msg list is really what you want.
    // for now, we get rid of it.
#if 1
    int left_over = MAXPALS - p.num_elems();
    if(left_over <= 0)
    {
        tmpl.del(My_UID);
        return tree_to_vec(tmpl);
    }
    // fill out remainder with recent conversations
    vc rm = sql_get_recent_users2(3600 * 24 * 14, left_over);
    if(rm.is_nil())
    {
        tmpl.del(My_UID);
        return tree_to_vec(tmpl);
    }
    for(int i = 0; i < rm.num_elems(); ++i)
    {
        const vc u = from_hex(rm[i]);
        const vc uids = map_uid_to_uids(u);
        for(int i = 0; i < uids.num_elems(); ++i)
            tmpl.add(uids[i], 0);
        //tmpl.add(u, 0);
    }
#endif
    tmpl.del(My_UID);
    return tree_to_vec(tmpl);
}


static
void
group_changed(vc)
{
    pal_relogin();
}

static
void
clear_online(int i)
{
    if(i == 0)
    {
        Online = vc(VC_TREE);
        Client_ports = vc(VC_TREE);
        Client_disposition = vc(VC_TREE);
    }
}

static
void
invis_changed(vc, vc)
{
    pal_relogin();
}

int
init_pal()
{
    if(Init)
        return 1;
    Last_sent = vc(VC_VECTOR);
    Init = 1;

    Group_uids.value_changed.connect_ptrfun(group_changed, 1);
    Database_online.value_changed.connect_ptrfun(clear_online, 1);
    bind_sql_setting("server/invis", invis_changed);
    return 1;
}

void
exit_pal()
{
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

static void
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

        vc cal = Last_sent;
        int i;
        // return is vectors like this
        // vector(uid ip type ports)
        //
        for(i = 0; i < n; ++i)
        {
            cal.del(uo[i][0]);
            Online.add_kv(uo[i][0], uo[i][1]);
            Client_disposition.add_kv(uo[i][0], uo[i][2]);
            Client_ports.add_kv(uo[i][0], uo[i][3]);
            se_emit(SE_STATUS_CHANGE, uo[i][0]);
        }
        // everyone else we asked about is *offline*
        n = cal.num_elems();
        for(i = 0; i < n; ++i)
        {
            Online.del(cal[i]);
            Client_disposition.del(cal[i]);
            Client_ports.del(cal[i]);
            se_emit(SE_STATUS_CHANGE, cal[i]);
        }
        // v[2] is ping interval from the server
        if(v[2].type() == VC_INT)
        {

        }

        Pal_logged_in = 1;
    }
    else if(v[0] == vcon)
    {
        vc uo = v[1];
        int n = uo.num_elems();
        int i;
        // return is vectors like this
        // vector(uid ip type ports)
        //
        for(i = 0; i < n; ++i)
        {
            Online.add_kv(uo[i][0], uo[i][1]);
            Client_disposition.add_kv(uo[i][0], uo[i][2]);
            Client_ports.add_kv(uo[i][0], uo[i][3]);
            se_emit(SE_STATUS_CHANGE, uo[i][0]);
        }
        // v[2] is ping interval from the server
        if(v[2].type() == VC_INT)
        {

        }
    }
    else if(v[0] == vcoff)
    {
        Online.del(v[1]);
        Client_disposition.del(v[1]);
        Client_ports.del(v[1]);
        se_emit(SE_STATUS_CHANGE, v[1]);
    }
    else if(v[0] == vcrelog)
    {
        // reset and log in again, server lost our state.
        pal_reset();
    }
    else if(v[0] == vclogoutok)
    {
        //Logout_ok = 1;
        Pal_logged_in = 0;
    }
}

void async_pal(vc resp, void *, vc, ValidPtr)
{
    process_pal_resp(resp[1]);
}

void
pal_relogin()
{
    pal_login();
}


int
pal_login()
{
    if(!Init)
        return 0;
    if(Inhibit_pal)
        return 0;

    vc v(VC_VECTOR);
    v[0] = "online";
    v[1] = My_UID;
    v[2] = transient_online_list();
    Last_sent = v[2];
    v[3] = is_invisible() ? "invis" : "vis";

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
    //v[7] = vc(VC_VECTOR);
    // kluge, the server still honors "always visible" and even if we are
    // in invisible mode, it is ok if we continue to sync with others
    // in the group, so lets just say we are always visible to them.
    v[7] = Group_uids;
    v[8] = vc(VC_VECTOR);

    v[4] = make_fw_setup();
    v[9] = MMChannel::My_disposition;
    dirth_send_set_interest_list(My_UID, v, QckDone());

    return 1;
}

int
pal_set_state(vc state)
{
    return 1;
}

// this is a NOOP for now, handled automatically
int
pal_logout()
{
    return 1;
}

int
pal_tick()
{
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
}

