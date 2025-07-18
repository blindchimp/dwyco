
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// $Header: g:/dwight/repo/cdc32/rcs/qmsg.cc 1.9 1999/01/10 16:09:51 dwight Checkpoint $

// manage messages

// messages are stored on disk like this:
//
// each sender has a directory whose name is
// their <uid>.usr
// each message that is received is stored in a file
// with the message id as the name: <mid>.bod or <mid>.snt. if a message
// has attachments, then all those attachments are stored
// in the directory as well.



#include "dwdate.h"

#include <stdio.h>
#include "mmchan.h"
#include "filetube.h"
#include "vc.h"
#include "dwstr.h"
#include "qauth.h"
#include "qdirth.h"
#ifdef _Windows
#ifdef _MSC_VER
#include <direct.h>
#endif
#include <dos.h>
#include <io.h>
#endif
#ifdef LINUX
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "miscemu.h"
#ifdef ANDROID
#include "glob/glob.h"
#else
#include <glob.h>
#endif
#include <dirent.h>
#endif

#include "dwvec.h"
#include "dwtree2.h"
#include "qmsg.h"
#include "dwlog.h"
#include "pval.h"
#include "doinit.h"
#include "ta.h"
#include "files.h"
#include "sha.h"
#include "aes.h"
#include "gcm.h"

using namespace CryptoPP;
#include "filters.h"

#include "netvid.h"
#include "dwrtlog.h"
#include "dwstr.h"
#include "profiledb.h"
#include "fnmod.h"
#ifdef _Windows
#define strcasecmp strcmpi
#endif
#include "dwtree2.h"
#include "se.h"
#include "pgdll.h"
#include "sepstr.h"
#include "ser.h"
#include "xinfo.h"
#include "vcudh.h"
#include "dhsetup.h"
#include "dhgsetup.h"
#include "qsend.h"
#include "ssns.h"
#include "dwyco_rand.h"
#include "qmsgsql.h"
#include "aconn.h"
#include "ezset.h"
#include "grpmsg.h"
#include "aconn.h"
#include "cdcpal.h"
#include "vccrypt2.h"

using namespace dwyco;
using namespace dwyco::qmsgsql;

namespace dwyco {
int Rescan_msgs;
vc Cur_ignore;
vc No_direct_msgs;
vc No_direct_att;
vc Session_infos;
vc MsgFolders;
vc Pals;
}

// list of message summaries both from server and direct
static vc Cur_msgs;

vc Session_ignore;
// mutual is separated out because we want to have a
// part of the ignore list that cannot be maniplated
// via the api
vc Mutual_ignore;


vc Online;
vc Client_disposition;

//vc Never_visible;
//vc Always_visible;
//vc I_grant;
//vc They_grant;
//int Pal_auth_warn;

vc Client_ports;
vc Chat_ips;
vc Chat_ports;
static vc In_progress;



static int64_t Logical_clock;

// this is used in order to assign clock values to
// messages when we first see them from the server.
static vc Mid_to_logical_clock;
extern vc Current_chat_server_id;

void new_pipeline();
int save_msg(vc m, vc msg_id);

#include "qmsgsql.h"

// XXX there is a problem with just fetching the logical clock
// from the msg index... there are paths through the code where
// we create a message, use the logical clock (and increment it) but
// then we never store the message (ie, a sent message that isn't saved.)
// if we quit and restart, our logical clock will not reflect that we
// sent that message. so, we really need to make sure the logical clock
// is stored somewhere persistently to get the best presentation of the
// message ordering.
// note that using the max value from the index is a perfectly reasonable
// estimate. likewise for the current system time, if it looks sane.
//

void
update_global_logical_clock(int64_t lc)
{
    if(lc > Logical_clock)
        Logical_clock = lc + 1;
}
void
boost_logical_clock()
{
    long tmplc;
    try {
        qmsgsql::sql_start_transaction();
        tmplc = sql_get_max_logical_clock();
        qmsgsql::sql_commit_transaction();
    } catch (...) {
        qmsgsql::sql_rollback_transaction();
        ++Logical_clock;
        return;
    }

    if(tmplc > Logical_clock)
        Logical_clock = tmplc + 1;
}

int64_t
diff_logical_clock(int64_t tm)
{
    return Logical_clock - tm;
}

#ifdef WIN32
int
move_replace(const DwString& s, const DwString& d)
{
    return MoveFileEx(s.c_str(), d.c_str(), MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
}
#else
int
move_replace(const DwString& s, const DwString& d)
{
    return rename(s.c_str(), d.c_str()) == 0;
}
#endif

static
int
add_msg_folder(const vc& uid)
{
    if(MsgFolders.contains(uid))
        return 0;
    MsgFolders.add_kv(uid, vcnil);
    se_emit(SE_USER_ADD, uid);
    return 1;
}

vc
get_local_pals()
{
    //vc res = sql_get_tagged_mids2("_pal");

    vc res = map_uid_list_from_tag("_pal");
    // if the database is locked or something, just return the
    // current set, and hope it gets updated at a later time.
    if(res.is_nil())
        return Pals;
    vc ret(VC_TREE);
    for(int i = 0; i < res.num_elems(); ++i)
        ret.add_kv(from_hex(res[i][0]), vcnil);
    return ret;
}

static vc Res;
static
void
append_to_res(vc raw, vc kv)
{
    if(raw.is_nil())
    {
        if(Res.num_elems() > MAXPALS)
            return;
        if(uid_ignored(kv[0]))
            return;
    }
    Res.append(kv[0]);
}

vc
pal_to_vector(int raw)
{
    Res = vc(VC_VECTOR);
    Pals.foreach(raw ? vctrue : vcnil, append_to_res);
    return Res;
}

static
void
pals_to_tags()
{
    vc v = pal_to_vector(1);
    for(int i = 0; i < v.num_elems(); ++i)
    {
        sql_add_tag(to_hex(v[i]), "_pal");
    }
}

static
bool
is_foreground(const vc& uid)
{
    vc disp;
    if(Client_disposition.find(uid, disp))
    {
        if(disp == vc("foreground"))
            return 1;
        // if there is no disposition, it is likely an old
        // client, so we just pretend it is foreground for compat
        if(disp.is_nil())
            return 1;
    }
    else
    {
        // if there is no disposition, it is likely an old
        // client, so we just pretend it is foreground for compat
        return 1;
    }
    return 0;
}


int
uid_online_display(const vc& uid)
{
    if(Online.contains(uid) && is_foreground(uid))
        return 1;
    if(Broadcast_discoveries.contains(uid))
        return 1;
    const vc uids = map_uid_to_uids(uid);
    if(uids.num_elems() == 1)
        return 0;

    for(int i = 0; i < uids.num_elems(); ++i)
    {
        const vc tuid = uids[i];
        if(Online.contains(tuid) && is_foreground(uid))
            return 1;
        if(Broadcast_discoveries.contains(tuid))
            return 1;

    }

    return 0;
}

int
uid_online(vc uid)
{
    if(Online.contains(uid))
        return 1;
    if(Broadcast_discoveries.contains(uid))
        return 1;
    return 0;
}

unsigned long
uid_to_ip(vc uid, int& can_do_direct, int& prim, int& sec, int& pal)
{

    vc u;
    prim = 0;
    sec = 0;
    pal = 0;
    can_do_direct = 0;

    if(Broadcast_discoveries.find(uid, u))
    {
        can_do_direct = 1;
        prim = u[BD_PRIMARY_PORT];
        sec = u[BD_SECONDARY_PORT];
        pal = u[BD_PAL_PORT];
        GRTLOGA("uid %s to ip locally ONLINE %s %d %d %d", (const char *)to_hex(uid), (const char *)u[0], prim, sec, pal);
        GRTLOGVC(u);
        return inet_addr((const char *)u[BD_IP]);
    }
    if(Online.find(uid, u))
    {
        can_do_direct = 1;
        DwString a((const char *)u);
        int c = a.find(":");
        if(c != DwString::npos)
            a.erase(c);
        if(Client_ports.find(uid, u))
        {
            if(u.type() == VC_VECTOR)
            {
                prim = (int)u[0];
                sec = (int)u[1];
                pal = (int)u[2];
            }
        }
        else
        {
            // something is wrong, but don't return 0 for ports, as
            // that is interpreted by some routers as "loopback"
            can_do_direct = 0;
            GRTLOG("ONLINE no ports %s", a.c_str(), 0);
            return 0;
        }
        GRTLOGA("uid %s to ip ONLINE %s %d %d %d", (const char *)to_hex(uid), a.c_str(), prim, sec, pal);
        return inet_addr(a.c_str());

    }
    if(Chat_ips.find(uid, u))
    {
        can_do_direct = 1;
        DwString a((const char *)u);
        int c = a.find(":");
        if(c != DwString::npos)
            a.erase(c);
        if(Chat_ports.find(uid, u))
        {
            if(u.type() == VC_VECTOR)
            {
                prim = (int)u[0];
                sec = (int)u[1];
                pal = (int)u[2];
            }
        }
        else
        {
            // something is wrong, but don't return 0 for ports, as
            // that is interpreted by some routers as "loopback"
            can_do_direct = 0;
            GRTLOG("CHAT no ports %s", a.c_str(), 0);
            return 0;
        }
        GRTLOGA("uid %s to ip ONLINE/CHAT %s %d %d %d", (const char *)to_hex(uid), a.c_str(), prim, sec, pal);
        return inet_addr(a.c_str());
    }

    return 0;
}

int
is_invisible()
{
    vc invis = get_settings_value("server/invis");
    return (int)invis;
    //return Invis;
}

void
set_invisible(int i)
{
    set_settings_value("server/invis", !!i);
    //Invis = !!i;
}

void
ack_get_done2(vc m, void *, vc del2_args, ValidPtr )
{
    // note: already deleted, might want to think about
    // the consequences of failed deletes here.
    // mainly i think messages may magically reappear the
    // next time the server is contacted. the "pending delete"
    // thing we had before didn't really do anything, but it might
    // make sense to have some kind of "pending" filter
    // depending on how much trouble it causes.
    //delete_msg2(del2_args[0], del2_args[1], vcnil);

    // NOTE: probably need to do another tag update to make sure
    // everything is gone in case it gets reinstated in the database
    // somehow
}

void
init_qmsg()
{
    Cur_msgs = vc(VC_VECTOR);
    Mutual_ignore = vc(VC_SET);
    Online = vc(VC_TREE);
    Client_disposition = vc(VC_TREE);
    No_direct_msgs = vc(VC_SET);
    No_direct_att = vc(VC_SET);
    MsgFolders = vc(VC_TREE);
    In_progress = vc(VC_TREE);
    // not perfect, but better than scanning for a max value
    // somewhere.
    Logical_clock = time(0);
#if 0
    if(!load_info(Never_visible, "never"))
    {
        Never_visible = vc(VC_SET);
        save_info(Never_visible, "never");
    }
    if(!load_info(Always_visible, "always"))
    {
        Always_visible = vc(VC_SET);
        save_info(Always_visible, "always");
    }
    if(!load_info(They_grant, "theygrnt"))
    {
        They_grant = vc(VC_TREE);
        save_info(They_grant, "theygrnt");
    }
    vc me;
    if(They_grant.find("me", me))
    {
        if(me != My_UID)
        {
            if(They_grant.num_elems() == 1)
            {
                // just update it.
                They_grant.add_kv("me", My_UID);
                save_info(They_grant, "theygrnt");
            }
            else
                Pal_auth_warn = 1;
        }
    }
    else
    {
        They_grant.add_kv("me", My_UID);
        save_info(They_grant, "theygrnt");
    }

    if(!load_info(I_grant, "igrant"))
    {
        I_grant = vc(VC_TREE);
        save_info(I_grant, "igrant");
    }

    if(I_grant.find("me", me))
    {
        if(me != My_UID)
        {
            if(I_grant.num_elems() == 1)
            {
                // just update it.
                I_grant.add_kv("me", My_UID);
                save_info(I_grant, "igrant");
            }
            else
                Pal_auth_warn = 1;
        }
    }
    else
    {
        I_grant.add_kv("me", My_UID);
        save_info(I_grant, "igrant");
    }
#endif

    if(!load_info(Pals, "pals") || Pals.is_nil())
    {
        Pals = vc(VC_TREE);
        save_info(Pals, "pals");
    }
    // one off conversion
    if(Pals.type() == VC_VECTOR)
    {
        int n = Pals.num_elems();
        vc np(VC_TREE);
        for(int i = 0; i < n; ++i)
        {
            np.add_kv(Pals[i], vcnil);
        }
        Pals = np;
        save_info(Pals, "pals");
    }

    Client_ports = vc(VC_TREE);
    if(!load_info(Session_infos, "sinfo") || Session_infos.type() != VC_MAP)
    {
        Session_infos = vc(VC_MAP);
        save_info(Session_infos, "sinfo");
    }
    Chat_ips = vc(VC_TREE);
    Chat_ports = vc(VC_TREE);

    init_qmsg_sql();
    init_group_map();

    if(!Pals.is_nil())
    {
        // compat hack. don't obliterate their old pal list
        // leave it intact so the old software doesn't crash
        // if they downgrade.
        vc loaded_pals;
        if(!load_info(loaded_pals, "loadpals.dif") || loaded_pals.is_nil())
        {
            pals_to_tags();
            save_info(vctrue, "loadpals.dif");
        }
    }
    Pals = get_local_pals();

    boost_logical_clock();

    new_pipeline();

    Mid_to_logical_clock = vc(VC_TREE);

    Cur_ignore = get_local_ignore();
}

void
save_qmsg_state()
{
    save_info(Session_infos, "sinfo");
}

void
exit_qmsg()
{
    exit_qmsg_sql();
    Cur_ignore = vcnil;
    //Session_ignore = vcnil;
    Mutual_ignore = vcnil;
    Online = vcnil;
    Client_disposition = vcnil;
    No_direct_msgs = vcnil;
    No_direct_att = vcnil;

    Cur_msgs = vcnil;
    //Online_noise = vcnil;
    //Never_visible = vcnil;
    //Always_visible = vcnil;
    //I_grant = vcnil;
    //They_grant = vcnil;
    //UID_infos = vcnil;
    MsgFolders = vcnil;
    Session_infos = vcnil;
    Chat_ips = vcnil;
    Chat_ports = vcnil;
    Mid_to_logical_clock = vcnil;
}

// note: this gets rid of a lot of ephemeral info that
// is better if it is refreshed on the next reconnect.
// access to the various sql databases is still ok
// even when suspended.
// this suspend/resume assumes that things might have changed
// a little while we were suspended, ie, maybe the inbox and other
// message storage items were changed by a background process, or
// the ignore list or other persistent info could have been updated.
// generally, we assume we are *not* connected to the network during
// the suspended state, so all the server derived info would have to be
// refreshed on next connect.
//
// note: the reason we don't clear this out now is because we can get some
// spurious (but mostly harmless) calls into the API even after a
// "suspend", and we don't want to crash those calls.
void
suspend_qmsg()
{
#if 0
    Cur_ignore = vcnil;
    Session_ignore = vcnil;
    Mutual_ignore = vcnil;
    Online = vcnil;
    Client_types = vcnil;
    No_direct_msgs = vcnil;
    No_direct_att = vcnil;

    Cur_msgs = vcnil;
    Online_noise = vcnil;
    //Never_visible = vcnil;
    //Always_visible = vcnil;
    //I_grant = vcnil;
    //They_grant = vcnil;
    //UID_infos = vcnil;
    MsgFolders = vcnil;
    Session_infos = vcnil;
    Chat_ips = vcnil;
    Chat_ports = vcnil;
    Mid_to_logical_clock = vcnil;
#endif
}

void
resume_qmsg()
{
    // these items will get updates when we connect
    // to a server, so just clear them out.
    Cur_msgs = vc(VC_VECTOR);
    Mutual_ignore = vc(VC_SET);
    Online = vc(VC_TREE);
    Client_disposition = vc(VC_TREE);

    // let's keep this info, since it is unlikely to
    // change while we were suspended
    //No_direct_msgs = vc(VC_SET);
    //No_direct_att = vc(VC_SET);

    // keep this info, since we load new stuff incrementally now
    // this might have to change if we reload everything in the
    // future
    //MsgFolders = vc(VC_TREE);

    In_progress = vc(VC_TREE);
    
    // not perfect, but better than scanning for a max value
    // somewhere.
    Logical_clock = time(0);

#if 0
    if(!load_info(Never_visible, "never"))
    {
        Never_visible = vc(VC_SET);
        save_info(Never_visible, "never");
    }
    if(!load_info(Always_visible, "always"))
    {
        Always_visible = vc(VC_SET);
        save_info(Always_visible, "always");
    }
    if(!load_info(They_grant, "theygrnt"))
    {
        They_grant = vc(VC_TREE);
        save_info(They_grant, "theygrnt");
    }
    vc me;
    if(They_grant.find("me", me))
    {
        if(me != My_UID)
        {
            if(They_grant.num_elems() == 1)
            {
                // just update it.
                They_grant.add_kv("me", My_UID);
                save_info(They_grant, "theygrnt");
            }
            else
                Pal_auth_warn = 1;
        }
    }
    else
    {
        They_grant.add_kv("me", My_UID);
        save_info(They_grant, "theygrnt");
    }

    if(!load_info(I_grant, "igrant"))
    {
        I_grant = vc(VC_TREE);
        save_info(I_grant, "igrant");
    }

    if(I_grant.find("me", me))
    {
        if(me != My_UID)
        {
            if(I_grant.num_elems() == 1)
            {
                // just update it.
                I_grant.add_kv("me", My_UID);
                save_info(I_grant, "igrant");
            }
            else
                Pal_auth_warn = 1;
        }
    }
    else
    {
        I_grant.add_kv("me", My_UID);
        save_info(I_grant, "igrant");
    }
#endif

    if(!load_info(Pals, "pals"))
    {
        Pals = vc(VC_TREE);
        save_info(Pals, "pals");
    }
    // one off conversion
    if(Pals.type() == VC_VECTOR)
    {
        int n = Pals.num_elems();
        vc np(VC_TREE);
        for(int i = 0; i < n; ++i)
        {
            np.add_kv(Pals[i], vcnil);
        }
        Pals = np;
        save_info(Pals, "pals");
    }
    if(!Pals.is_nil())
    {
        // compat hack. don't obliterate their old pal list
        // leave it intact so the old software doesn't crash
        // if they downgrade.
        vc loaded_pals;
        if(!load_info(loaded_pals, "loadpals.dif") || loaded_pals.is_nil())
        {
            pals_to_tags();
            save_info(vctrue, "loadpals.dif");
        }
    }
    Pals = get_local_pals();
    Client_ports = vc(VC_TREE);
    if(!load_info(Session_infos, "sinfo") || Session_infos.type() != VC_MAP)
    {
        Session_infos = vc(VC_MAP);
        save_info(Session_infos, "sinfo");
    }
    Chat_ips = vc(VC_TREE);
    Chat_ports = vc(VC_TREE);

    //init_qmsg_sql();
    long tmplc = sql_get_max_logical_clock();
    if(tmplc > Logical_clock)
        Logical_clock = tmplc + 1;
    Cur_ignore = get_local_ignore();
    //init_fav_sql();

    //new_pipeline();

    //Mid_to_logical_clock = vc(VC_TREE);
}

// need this because some info files get
// trashed mysteriously...
static
int
valid_info(vc v)
{
    if(v.type() != VC_VECTOR)
        return 0;
    for(int i = 0; i < 7; ++i)
    {
        // note: a bit of a regression... old "info"
        // files may have nils which is no problem since
        // we don't use these fields anyplace anyway.
        // and we don't want to reject those old files
        // if we can't get the profile info anywhere else.
        if(i == QIR_USER_SPECED_ID || i == QIR_LOCATION)
            continue;
        if(v[i].type() != VC_STRING)
            return 0;
    }
    if(v[QIR_FROM].len() < 10)
        return 0;
    return 1;
}

int
valid_qd_message(const vc& v)
{
// vector(vector(recipients...) vector(fromid text_message attachid vector(yy dd hh mm ss))
    if(v.type() != VC_VECTOR)
        return 0;
    if(v[QQM_RECIP_VEC].type() != VC_VECTOR)
        return 0;
    int n = v[QQM_RECIP_VEC].num_elems();
    if(n <= 0)
        return 0;
    for(int i = 0; i < n; ++i)
        if(v[QQM_RECIP_VEC][i].type() != VC_STRING ||
                v[QQM_RECIP_VEC][i].len() < 10)
            return 0;
    if(v[QQM_MSG_VEC].type() != VC_VECTOR)
        return 0;
    vc mvec = v[QQM_MSG_VEC];
    if(mvec[QQM_BODY_FROM].type() != VC_STRING ||
            mvec[QQM_BODY_FROM].len() < 10)
        return 0;
    if(mvec[QQM_BODY_NEW_TEXT].type() != VC_STRING)
        return 0;
    if(!mvec[QQM_BODY_ATTACHMENT].is_nil())
    {
        if(mvec[QQM_BODY_ATTACHMENT].type() != VC_STRING ||
                !is_attachment(mvec[QQM_BODY_ATTACHMENT]))
            return 0;
    }
    if(mvec[QQM_BODY_DATE].type() != VC_VECTOR)
        return 0;
    return 1;
}

// note: this "secret" was used to implement a checksum that
// couldn't easily be generated by 3rd parties. it was used
// mainly to enforce the "no forward" flag for messages.
// ca 2018, it is more used as a checksum for validating
// messages. and even then, some of that validation is too
// expensive on mobile devices, so it is not used consistently.
// if you change this, old software is likely to start rejecting
// and possibly deleting old messages since it may think they
// are corrupted. SO DON'T CHANGE IT, unless that is what you want.

#define HASH_SECRET "\x98\x43\xaa\xf3\x60\x00\x33\x09\x14\x44"

vc
gen_hash(DwString filename)
{
    vc val;
    filename = newfn(filename);
    if(access(filename.c_str(), 04) == -1)
    {
        val = gen_id();
        GRTLOG("bogus hash filename %s", filename.c_str(), 0);
        return val;
    }
    SHA1 sha;
    HashFilter *hf = new HashFilter(sha);
    FileSource fs(filename.c_str(), true, hf);
    int n = hf->MaxRetrievable();
    if(n != 20)
        val = gen_id(); // gen some garbage so it won't match anything
    else
    {
        byte a[20];
        hf->Get(a, sizeof(a));
        vc v(VC_BSTRING, (const char *)a, sizeof(a));
        val = v;
    }
    GRTLOG("gen hash %s =", filename.c_str(), 0);
    GRTLOGVC(to_hex(val));
    return val;
}

static
void
gen_authentication(vc qmsg, const vc& att_hash)
{
    // put the auth vector into a q'd message
    vc av(VC_VECTOR);
    vc mvec = qmsg[QQM_MSG_VEC];

    SHA1 sha;
    byte *secret = (byte *)HASH_SECRET;
    sha.Update(secret, sizeof(HASH_SECRET) - 1);
    // note: old clients may or may not be able to authenticate newer
    // messages.
    //if(mvec[QQM_BODY_FORWARDED_BODY].is_nil())
    //	sha.Update((const byte *)(const char *)mvec[QQM_BODY_TEXT], mvec[QQM_BODY_TEXT].len());
    //else
    sha.Update((const byte *)(const char *)mvec[QQM_BODY_NEW_TEXT], mvec[QQM_BODY_NEW_TEXT].len());
    sha.Update((const byte *)(const char *)My_UID, My_UID.len());
#if 0
    // don't do this now since it leaves a window open where
    // a user could switch filenames and have a good auth
    // vector created from the bogus material. so we compute
    // the hash of the material elsewhere, and hash that in
    // instead of hashing in the contents of the current file.
    if(!mvec[QQM_BODY_ATTACHMENT].is_nil())
    {
        HashFilter *hf = new HashFilter(sha);
        FileSource fs(mvec[QQM_BODY_ATTACHMENT], FALSE, hf);
        fs.PumpAll();
        // note: don't want to close, since we have more to add.
    }
#endif
    sha.Update((const byte *)(const char *)att_hash, att_hash.len());
    uint32_t tm;
    tm = (uint32_t)(int)mvec[QQM_BODY_DATE][5]; // time(0) system time, bad choice, but what the hey
    tm = htonl(tm);
    sha.Update((const byte *)&tm, sizeof(tm));

    // newer style of message with the no_forward flag added in.
    // old messages will have this as nil, and we want the checksum
    // for those messages to remain the same (and remain forward-able
    // as well. however, we don't want to allow people to change the
    // forward flag and we want to allow the core to enforce this
    // via the checksum.
    if(mvec[QQM_BODY_NO_FORWARD].is_nil())
    {
        // old style, don't do anything
    }
    else if(mvec[QQM_BODY_NO_FORWARD].type() == VC_INT)
    {
        uint32_t nf;

        nf = (uint32_t)(int)mvec[QQM_BODY_NO_FORWARD];
        nf = htonl(nf);
        sha.Update((const byte *)&nf, sizeof(nf));
        av[QMBA_NO_FORWARD] = mvec[QQM_BODY_NO_FORWARD];
    }
    else
    {
        // bogus, don't do anything
    }


    // use first 96 bits only

    byte a[20];
    sha.Final(a);
    vc auth(VC_BSTRING, (const char *)a, 96 / 8);
    av[QMBA_COMB_AUTH_HASH] = auth;
    mvec[QQM_BODY_AUTH_VEC] = av;
    GRTLOG("gen auth", 0, 0);
    GRTLOGVC(qmsg);
    GRTLOGVC(to_hex(auth));
}

int
verify_authentication(const vc& text, const vc& uid, const vc& att_hash, const vc& datevec, const vc& no_forward, const vc& mac)
{
    SHA1 sha;
    byte *secret = (byte *)HASH_SECRET;
    sha.Update(secret, sizeof(HASH_SECRET) - 1);
    sha.Update((const byte *)(const char *)text, text.len());
    sha.Update((const byte *)(const char *)uid, uid.len());
#if 0
// see above why we don't do this anymore
    if(!att_filename.is_nil())
    {
        HashFilter *hf = new HashFilter(sha);
        FileSource fs(att_filename, FALSE, hf);
        fs.PumpAll();
    }
#endif
#if 0
// don't do this because the filename may not exist
// if we are authenticating a chain. make the
// call generate the hash and send it in.
    if(!att_filename.is_nil())
    {
        vc hash = gen_hash((const char *)att_filename);
        sha.Update((const byte *)(const char *)hash, hash.len());
    }
#endif
    sha.Update((const byte *)(const char *)att_hash, att_hash.len());

    uint32_t tm;
    tm = (uint32_t)(int)datevec[5]; // time(0) system time, bad choice, but what the hey
    tm = htonl(tm);
    sha.Update((const byte *)&tm, sizeof(tm));

    if(no_forward.is_nil())
    {
        // do nothing, old style message
    }
    else if(no_forward.type() == VC_INT)
    {
        uint32_t nf;
        nf = (uint32_t)(int)no_forward;
        nf = htonl(nf);
        sha.Update((const byte *)&nf, sizeof(nf));
    }
    else
    {
        // do nothing
    }
    byte a[20];
    sha.Final(a);
    vc auth(VC_BSTRING, (const char *)a, 96 / 8);
    if(auth == mac)
        return 1;
    return 0;
}

int
verify_chain(const vc& body, int top, const vc& a_att_hash, const vc& attachment_dir)
{
    GRTLOG("verify chain %d", top, 0);
    GRTLOGVC(body);
    GRTLOGVC(a_att_hash);
    GRTLOGVC(attachment_dir);
    const vc from = body[QM_BODY_FROM];
    vc text = body[QM_BODY_NEW_TEXT];
    const vc attachment = body[QM_BODY_ATTACHMENT];
    const vc authvec = body[QM_BODY_AUTH_VEC];
    const vc new_text = body[QM_BODY_NEW_TEXT];
    const vc forwarded_body = body[QM_BODY_FORWARDED_BODY];
    const vc datevec = body[QM_BODY_DATE];
    const vc no_forward = body[QM_BODY_NO_FORWARD];
    vc att_hash = a_att_hash;

    int res = 0;
    if(authvec.is_nil())
    {
        res = VERF_AUTH_NO_INFO;
        return res;
    }
    const vc av_no_forward = authvec[QMBA_NO_FORWARD];
    if(!forwarded_body.is_nil())
        text = new_text;
    int auth;
    if(!attachment.is_nil())
    {
        vc h;
        if(top)
        {
            // create hash cuz we are at the top
            // level where the attachment file should
            // exist.
            DwString s;
            vc dir;
            // note: if the message is one we sent,
            // we have to find the attachment in the
            // directory of the *recipient*, not the
            // from dir (which is us.)
            if(attachment_dir.is_nil())
                dir = uid_to_dir(from);
            else
                dir = attachment_dir;
            s = (const char *)dir;
            s += "" DIRSEPSTR "";
            s += (const char *)attachment;
            att_hash = gen_hash(s.c_str());
        }
        else
        {
            // attachment file may not exist, so
            // just use the passed in hash value (we
            // are authenticating previous forwarded bodies)
        }
        auth = verify_authentication(text,
                                     from, att_hash, datevec, no_forward, authvec[QMBA_COMB_AUTH_HASH]);
    }
    else
    {
        auth = verify_authentication(text,
                                     from, vcnil, datevec, no_forward, authvec[QMBA_COMB_AUTH_HASH]);
    }
    if(auth)
        res = VERF_AUTH_OK;
    else
        res = VERF_AUTH_FAILED;

    // this means that the msg has been thru some old
    // software that doesn't honor the no_forward flag.
    // if this is true, you probably don't want to show the
    // message if any no_forward flags are present.

    if(av_no_forward != no_forward || no_forward.is_nil())
        res |= VERF_AUTH_THRU_OLD_FORWARD;

    if(!forwarded_body.is_nil())
    {
        res |= verify_chain(forwarded_body, 0, att_hash);
    }
    return res;
}

// this function strips all the forwards off a message
// and leaves just the creator of the message (the original)
// this is useful if something is getting forwarded a lot
// and it keeps growing, making it impossible to send thru
// the server efficiently. this does remove some of the
// security of the message since the forward chain is
// lost, but to date (12/2002), the forward chain security hasn't been
// used, so it is no big loss.
vc
strip_chain(vc body)
{
    if(body[QM_BODY_FORWARDED_BODY].is_nil())
        return body;
    vc forwarded_body = body[QM_BODY_FORWARDED_BODY];
    while(!forwarded_body[QM_BODY_FORWARDED_BODY].is_nil())
    {
        forwarded_body = forwarded_body[QM_BODY_FORWARDED_BODY];
    }
    // note: the body that is returned may have an
    // attachment field that doesn't exist on the disk
    return forwarded_body;
}

// creator no-forward
// this function is used to check to see if the
// creator of a message set the no-forward flag.
// this is meant to thwart the possibility that someone
// sends a message to old software in order to strip or
// hide the no-forward flag.
// note: ICUII decided that old msgs created before no-forwarding
// was released should be NO-FORWARD by default. CDC32 does the opposite.

int
creator_no_forward(vc body)
{
    if(body[QM_BODY_FORWARDED_BODY].is_nil())
    {
        if(body[QM_BODY_NO_FORWARD].is_nil())
#ifdef ICUII
            return 1;
#else
            return 0;
#endif
        else if(body[QM_BODY_NO_FORWARD].type() == VC_INT &&
                (int)body[QM_BODY_NO_FORWARD] != 0)
            return 1;
        return 0;
    }
    vc forwarded_body = body[QM_BODY_FORWARDED_BODY];
    while(!forwarded_body[QM_BODY_FORWARDED_BODY].is_nil())
    {
        forwarded_body = forwarded_body[QM_BODY_FORWARDED_BODY];
    }
    if(forwarded_body[QM_BODY_NO_FORWARD].is_nil())
#ifdef ICUII
        return 1;
#else
        return 0;
#endif
    else if(forwarded_body[QM_BODY_NO_FORWARD].type() == VC_INT &&
            (int)forwarded_body[QM_BODY_NO_FORWARD] != 0)
        return 1;
    return 0;
}

int
any_no_forward(vc body)
{
    if(body[QM_BODY_FORWARDED_BODY].is_nil())
    {
        if(body[QM_BODY_NO_FORWARD].type() == VC_INT &&
                (int)body[QM_BODY_NO_FORWARD] != 0)
            return 1;
        if(body[QM_BODY_AUTH_VEC].type() == VC_VECTOR &&
                body[QM_BODY_AUTH_VEC][QMBA_NO_FORWARD].type() == VC_INT &&
                (int)body[QM_BODY_AUTH_VEC][QMBA_NO_FORWARD] != 0)
            return 1;
        return 0;
    }

    vc forwarded_body = body[QM_BODY_FORWARDED_BODY];
    // old old message check
    if(forwarded_body.type() != VC_VECTOR)
        return 0;
    while(!forwarded_body[QM_BODY_FORWARDED_BODY].is_nil())
    {
        if(forwarded_body[QM_BODY_NO_FORWARD].type() == VC_INT &&
                (int)forwarded_body[QM_BODY_NO_FORWARD] != 0)
            return 1;
        if(forwarded_body[QM_BODY_AUTH_VEC].type() == VC_VECTOR &&
                forwarded_body[QM_BODY_AUTH_VEC][QMBA_NO_FORWARD].type() == VC_INT &&
                (int)forwarded_body[QM_BODY_AUTH_VEC][QMBA_NO_FORWARD] != 0)
            return 1;
        forwarded_body = forwarded_body[QM_BODY_FORWARDED_BODY];
    }
// this was causing a problem with keeping people from
// playing back msgs that had been forwarded with old
// software. we still want to keep people from
// *forwarding* those msgs, and that is handled via
// "can_forward" below.
#if 0 && defined(ICUII)
    if(forwarded_body[QM_BODY_NO_FORWARD].type() != VC_INT)
        return 1; // if the original msg was not created with new software, no forward allowed.
#endif
    if(forwarded_body[QM_BODY_NO_FORWARD].type() == VC_INT &&
            (int)forwarded_body[QM_BODY_NO_FORWARD] != 0)
        return 1;
    if(forwarded_body[QM_BODY_AUTH_VEC].type() == VC_VECTOR &&
            forwarded_body[QM_BODY_AUTH_VEC][QMBA_NO_FORWARD].type() == VC_INT &&
            (int)forwarded_body[QM_BODY_AUTH_VEC][QMBA_NO_FORWARD] != 0)
        return 1;
    return 0;
}

int
can_forward(vc body, vc att_dir)
{
#ifdef ICUII
    if(body[QM_BODY_NO_FORWARD].type() != VC_INT)
        return 0; // never allow forwarding of old msgs
#endif
    if(body[QM_BODY_NO_FORWARD].type() == VC_INT &&
            (int)body[QM_BODY_NO_FORWARD] != 0)
        return 0;
    if(any_no_forward(body))
        return 0;
    int res = verify_chain(body, 1, vcnil, att_dir);
    if(res != VERF_AUTH_OK)
    {
        // never forward msgs that are corrupt
        if(res & VERF_AUTH_FAILED)
            return 0;
        if((res & (VERF_AUTH_OK|VERF_AUTH_THRU_OLD_FORWARD|VERF_AUTH_NO_INFO)) &&
                creator_no_forward(body) == 0)
        {
            // allow forward of old msgs as long as the
            // creator didn't disallow forwarding
            return 1;
        }
        // don't allow forwarding of bogus messages
        return 0;
    }
    return 1;
}

vc
dir_to_uid(DwString s)
{
    if(s.length() == 0)
        return vc("");
    if(s.rfind(".usr") != s.length() - 4)
        return vc("");
    s.remove(s.length() - 4);
    vc who = from_hex(s.c_str());
    return who;
}

vc
uid_to_dir(const vc& uid)
{
    if(uid.len() == 0)
        return "";
    vc v = to_hex(uid);
    DwString s = (const char *)v;
    s += ".usr";
    return vc(s.c_str());
}

int
init_msg_folder(vc uid, DwString* fn_out)
{
    if(uid.len() == 0)
        return 0;
    DwString s((const char *)to_hex(uid));
    s += ".usr";
    s = newfn(s);
    if(fn_out)
        *fn_out = s;
    mkdir(s.c_str());

    int do_fetch = add_msg_folder(uid);

    if(do_fetch || !Session_infos.contains(uid))
    {
        fetch_info(uid);
    }
    return 1;
}

int
init_msg_folder(vc uid)
{
    vc uids = map_uid_to_uids(uid);
    for(int i = 0; i < uids.num_elems(); ++i)
        init_msg_folder(uids[i], 0);
    return 1;
}

void
FindVec::find_to_vec(const char *pat)
{
    int i = 1;

    DwVecP<WIN32_FIND_DATA>& ret = fv;
#ifdef _Windows
    WIN32_FIND_DATA n;
    HANDLE h = FindFirstFile(pat, &n);
    int idx;
    for(idx = 0; h != INVALID_HANDLE_VALUE && i != 0; i = FindNextFile(h, &n))
    {
        ret[idx] = new WIN32_FIND_DATA;
        *ret[idx] = n;
        ++idx;
    }
    if(h != INVALID_HANDLE_VALUE)
        FindClose(h);
#endif
#if defined(LINUX)
// HACK: in dos, *.* will match a filename "foo" even tho
// it has no ".", so we kluge here and translate *.* into *
    char *a;
    int free_pat = 0;
    int l = strlen(pat);
    if(l >= 3)
    {
        if(pat[l - 3] == '*' &&
                pat[l - 2] == '.' &&
                pat[l - 1] == '*')
        {
            a = strdup(pat);
            a[l - 2] = 0;
            pat = a;
            free_pat = 1;
        }
    }
    glob_t glb;
    glb.gl_offs = 0;
    int e = glob(pat, GLOB_NOSORT, 0, &glb);
    if(e != 0)
    {
        if(free_pat)
            free((void *)pat);
        return;
    }
    for(i = 0; i < glb.gl_pathc; ++i)
    {
        DwString a(glb.gl_pathv[i]);
        a = dwbasename(a.c_str());
        ret[i] = new WIN32_FIND_DATA;
        memset(ret[i], 0, sizeof(*ret[i]));
        strncpy(ret[i]->cFileName, a.c_str(), sizeof(ret[i]->cFileName) - 1);
    }
    globfree(&glb);
    if(free_pat)
        free((void *)pat);
#endif
}

static
int
compare_date_vector(vc v1, vc v2)
{
    // if they are both new-style, then
    // use GMT, else do old style.
    if(v1.num_elems() == 6 && v2.num_elems() == 6)
    {
        if(v1[5] < v2[5])
            return -1;
        else if(v1[5] > v2[5])
            return 1;
        return 0;
    }
    // new style are always newer than old style
    if(v1.num_elems() == 6 && v2.num_elems() == 5)
        return 1;
    else if(v1.num_elems() == 5 && v2.num_elems() == 6)
        return -1;
    for(int i = 0; i < 5; ++i)
        if(v1[i] < v2[i])
            return -1;
        else if(v1[i] > v2[i])
            return 1;
    return 0;
}


MMChannel *
fetch_attachment(vc fn, DestroyCallback dc, vc dcb_arg1, void *dcb_arg2, ValidPtr dcb_arg3,
                 StatusCallback sc, void *scb_arg1, ValidPtr scb_arg2, vc sip, vc port)
{
    if(sip.is_nil() || port.is_nil())
        return 0;
    MMChannel *mc = 0;

    mc = new MMChannel;
    mc->tube = new DummyTube;
    mc->tube->connect((const char *)sip, 0, 0);

    int i = -1;
    int state;
    if((state = mc->tube->gen_channel((int)port, i)) == SSERR)
    {
        delete mc->tube;
        delete mc;
        return 0;
    }
    mc->remote_filename = fn;
    sproto *s = new sproto(i, recv_attachment_pull, mc->vp);
    mc->simple_protos[i] = s;
    s->start();

    mc->destroy_callback = dc;
    mc->dcb_arg1 = dcb_arg1;
    mc->dcb_arg2 = dcb_arg2;
    mc->dcb_arg3 = dcb_arg3;
    mc->status_callback = sc;
    mc->scb_arg1 = scb_arg1;
    mc->scb_arg2 = scb_arg2;

    mc->init_config(1);
    mc->recv_matches(mc->config);
    //mc->firewall_friendly = 0;

    mc->set_string_id("<<Zap Msg Recv>>");

    mc->start_service();
    return mc;
}

static void
fetch_pk_done2(vc m, void *, vc uid, ValidPtr)
{
    if(m[1].is_nil())
    {
        if(m[2] == vc("no-key"))
        {
            pk_invalidate(uid);
            // to avoid repeatedly fetching a key that may never exist
            // (like it was an old account that never had a key generated
            // for it), for this session we'll just cache this response
            // from the server.
            pk_set_session_cache(uid);
            TRACK_ADD(QM_fetch_pk_no_key2, 1);
        }
        else
        {
            // if it was a failure (most likely went offline
            // while trying to deliver a message), cause a new
            // fetch to happen next time the send is attempted
            pk_force_check(uid);
            TRACK_ADD(QM_fetch_pk_failed2, 1);
        }
        return;
    }
    if(m[1].type() != VC_VECTOR)
    {
        TRACK_ADD(QM_fetch_pk_bogus_return2, 1);
        return;
    }
    vc static_public(VC_VECTOR);
    static_public[DH_STATIC_PUBLIC] = m[1][0];
    vc sig = m[1][1];
    // next is a vector containing the alt key
    vc alt = m[1][2];
    if(!alt.is_nil())
    {
        vc alt_pk = alt[0];
        vc alt_static_public(VC_VECTOR);
        alt_static_public[DH_STATIC_PUBLIC] = alt_pk;
        vc server_sig = alt[1];
        vc gname = alt[2];
        put_pk2(uid, static_public, sig, alt_static_public, server_sig, gname);
    }
    else
    {
        put_pk(uid, static_public, sig);
    }

    pk_set_session_cache(uid);
    TRACK_ADD(QM_fetch_pk_ok2, 1);

}

static void
fetch_group_uids(vc m, void *, vc, ValidPtr)
{
    if(m[1].is_nil())
        return;
    const vc pk = m[1][0];
    const vc uids = m[1][1];

    for(int i = 0; i < uids.num_elems(); ++i)
    {
        const vc uid = uids[i];
        if(dirth_pending_callbacks(fetch_pk_done2, 0, ReqType(), uid))
            return;
        dirth_send_get_pk(My_UID, uid, QckDone(fetch_pk_done2, 0, uid));
    }

}


static void
fetch_pk_done(vc m, void *user_arg, vc uid, ValidPtr)
{
    if(m[1].is_nil())
    {
        if(m[2] == vc("no-key"))
        {
            pk_invalidate(uid);
            // to avoid repeatedly fetching a key that may never exist
            // (like it was an old account that never had a key generated
            // for it), for this session we'll just cache this response
            // from the server.
            pk_set_session_cache(uid);
            TRACK_ADD(QM_fetch_pk_no_key, 1);
        }
        else
        {
            // if it was a failure (most likely went offline
            // while trying to deliver a message), cause a new
            // fetch to happen next time the send is attempted
            pk_force_check(uid);
            TRACK_ADD(QM_fetch_pk_failed, 1);
        }
        return;
    }
    if(m[1].type() != VC_VECTOR)
    {
        TRACK_ADD(QM_fetch_pk_bogus_return, 1);
        return;
    }
    vc static_public(VC_VECTOR);
    static_public[DH_STATIC_PUBLIC] = m[1][0];
    vc sig = m[1][1];
    // next is a vector containing the alt key
    vc alt = m[1][2];
    if(!alt.is_nil())
    {
        vc alt_pk = alt[0];
        vc alt_static_public(VC_VECTOR);
        alt_static_public[DH_STATIC_PUBLIC] = alt_pk;
        vc server_sig = alt[1];
        vc gname = alt[2];
        put_pk2(uid, static_public, sig, alt_static_public, server_sig, gname);
        // force fetch of all profiles in the gname
        // this probably needs to be changed so that a list of other uid's in
        // the group are returned as well, for consistency reasons (ie, things
        // might change while this operation in flight.
        // for now, we'll do it this way to see how things work out.
        int get_members = (user_arg == nullptr);
        if(get_members)
            dirth_send_get_group_pk(My_UID, gname, QckDone(fetch_group_uids, 0));

    }
    else
    {
        put_pk(uid, static_public, sig);
    }

    pk_set_session_cache(uid);
    TRACK_ADD(QM_fetch_pk_ok, 1);

}

static
void
fetch_info_done_profile(vc m, void *, vc other, ValidPtr)
{

    // other is vector(id vector(name desc location))
    // name and desc are used when we can't find
    // another source for the user info, so we
    // can display at least something
    vc uid = other[0];
    In_progress.del(uid);
//    vc name = other[1][0];
//    vc desc = other[1][1];
//    vc location = other[1][2];

    static const vc invalid("invalid");

    vc v;
    // special case: if issuing this command results in a local
    // error like a timeout, we want to allow the client to attempt
    // the fetch again. this is a little dangerous because we could
    // end up fetching forever, but since it is a local error, the
    // server probably is never seeing it anyway.
    // in the past, we provided "alternate info" we could use if the
    // server failed, and this was derived from secondary like the
    // directory. but if those sources are not available that
    // bogus info would get in and never get cleaned out.
#if 0
    static vc server_timeout("server timeout");
    static vc server_disconnected("server disconnected");
    static vc server_error("server error");
    if(m[1].is_nil() && (m[2] == server_timeout ||
                         m[2] == server_disconnected || m[2] == server_error))
    {
        GRTLOG("timeout/disconnect on fetch info", 0, 0);
        GRTLOGVC(m);
        return;
    }
#endif
    // NOTE: right now, an invalid is returned from the
    // server if the id doesn't exist.
    // XXX this should be fixed... if we have local info even if it is
    // stale, we should use it. *unless* the server responds that the
    // id just doesn't exist. in which case we should do nothing.
    if(m[1].is_nil() || m[2] == invalid)
    {
        GRTLOG("nil fetch info", 0, 0);
        GRTLOGVC(m);

        // if the server responded invalid, then stop trying to
        // fetch it
        if(m[2] == invalid)
            prf_set_cached(uid);
        const vc ai = make_best_local_info(uid, 0);
        v = vc(VC_VECTOR);
        v[QIR_FROM] = uid;
        v[QIR_HANDLE] = ai[0];
        v[QIR_EMAIL] = "";
        v[QIR_USER_SPECED_ID] = "";
        v[QIR_FIRST] = "";
        v[QIR_LAST] = "";
        v[QIR_DESCRIPTION] = ai[2];
        v[QIR_LOCATION] = ai[1];
    }
    else
    {
        // m[1] is the profile or "ok" if our cache is up to date
        if(m[1].type() == VC_STRING && m[1] == vc("ok"))
        {
            prf_set_cached(uid);
            const vc ai = make_best_local_info(uid, 0);
            v = vc(VC_VECTOR);
            v[QIR_FROM] = uid;
            v[QIR_HANDLE] = ai[0];
            v[QIR_EMAIL] = "";
            v[QIR_USER_SPECED_ID] = "";
            v[QIR_FIRST] = "";
            v[QIR_LAST] = "";
            v[QIR_DESCRIPTION] = ai[2];
            v[QIR_LOCATION] = ai[1];
        }
        else if(save_profile(uid, m[1]))
        {
            // create an old-style info vec
            prf_set_cached(uid);
            const vc ai = make_best_local_info(uid, 0);
            v = vc(VC_VECTOR);
            v[QIR_FROM] = uid;
            v[QIR_HANDLE] = ai[0];
            v[QIR_EMAIL] = "";
            v[QIR_USER_SPECED_ID] = "";
            v[QIR_FIRST] = "";
            v[QIR_LAST] = "";
            v[QIR_DESCRIPTION] = ai[2];
            v[QIR_LOCATION] = ai[1];
            //Refresh_users = 1;
            //se_emit(SE_USER_UID_RESOLVED, uid);

        }
        else
        {
            // had to use alternate info, don't set as cached
            v = vc(VC_VECTOR);
            const vc ai = make_best_local_info(uid, 0);
            v[QIR_FROM] = uid;
            v[QIR_HANDLE] = ai[0];
            v[QIR_EMAIL] = "";
            v[QIR_USER_SPECED_ID] = "";
            v[QIR_FIRST] = "";
            v[QIR_LAST] = "";
            v[QIR_DESCRIPTION] = ai[2];
            v[QIR_LOCATION] = ai[1];
        }

    }
    if(valid_info(v))
    {
        const vc user_id = v[0];

        Session_infos.add_kv(user_id, v);
    }
}

static
vc
make_alt_info(const vc& name, const vc& desc, const vc& location)
{
    vc r(VC_VECTOR);
    r.append(name);
    r.append(desc);
    r.append(location);
    return r;
}

vc
make_best_local_info(const vc& uid, int *cant_resolve_now)
{
    if(cant_resolve_now)
        *cant_resolve_now = 0;
    // local profile cache first
    vc prf;
    if(load_profile(uid, prf))
    {
        vc pack;
        if(deserialize(prf[PRF_PACK], pack))
        {
            static vc h("handle");
            static vc d("desc");
            static vc l("loc");
            return make_alt_info(pack[h], pack[d], pack[l]);
        }
    }
    // this is an indicator that the profile isn't available in the main cache, and should
    // be fetched from the server

    // if we found something on the local network, at least give a name for it
    {
        vc u;
        if(Broadcast_discoveries.find(uid, u))
        {
            const vc name = u[BD_NICE_NAME];
            if(!name.is_nil())
                return make_alt_info(name, "", "local-net");
        }
    }
    if(cant_resolve_now)
        *cant_resolve_now = 1;
    if(uid == My_UID)
    {
        return make_alt_info(
                    get_settings_value("user/username"),
                    get_settings_value("user/description"),
                    get_settings_value("user/location")
                    );

    }
    // try infos
    vc info;

    // try session infos
    if(Session_infos.find(uid, info))
    {
        return make_alt_info(info[QIR_HANDLE], info[QIR_DESCRIPTION], info[QIR_LOCATION]);
    }
    // try the info file in the directory
    if(uid.len() == 0)
        return make_alt_info("", "", "");
    DwString s((const char *)to_hex(uid));
    s += ".usr";
    s = newfn(s);
    s += DIRSEPSTR "info";
    if(load_info(info, s.c_str(), 1) && valid_info(info))
    {
        return make_alt_info(info[QIR_HANDLE], info[QIR_DESCRIPTION], info[QIR_LOCATION]);
    }

    return make_alt_info(to_hex(uid), "", "");
}

void
fetch_info(const vc& uid)
{
// fetch info is now a "get-profile", though for compat, we
// morph the profile info to old format for now.
// upshot of this is that we fetch from the profile cache, and if it
// isn't there, we can check a couple of other places (infos, and
// sinfo, then *.usr/info), then issue the get-profile command
// and then when it comes back, update all the secondary info.
// this allows us to eliminate the "already fetched" thing
// (the profile cache takes care of that).
    vc v(VC_VECTOR);
    v.append(uid);
    // load alt-info up with the best info we have locally, just in case
    //v.append(make_best_local_info(uid, 0));
    if(!In_progress.contains(uid))
    {
        if(prf_already_cached(uid))
        {
            vc resp(VC_VECTOR);
            vc vr;
            vr = "ok";
            resp[0] = vr;
            dirth_q_local_action(resp, QckDone(fetch_info_done_profile, 0, v));
        }
        else
        {

            vc prf;
            vc cache_check;
            if(load_profile(uid, prf))
                cache_check = prf[PRF_CHKSUM]; // dwyco/cdcx profile
            dirth_send_get_info(My_UID, uid, cache_check, QckDone(fetch_info_done_profile, 0, v));
        }
        In_progress.add(uid);
    }
    //prf_force_check(id);

    if(pk_session_cached(uid))
        return;
    if(dirth_pending_callbacks(fetch_pk_done, 0, ReqType(), uid))
        return;
    dirth_send_get_pk(My_UID, uid, QckDone(fetch_pk_done, 0, uid));
}

vc
save_body(vc msg_id, vc from, vc text, vc attachment_id, vc date, vc rating, vc authvec,
          vc forwarded_body, vc new_text, vc no_forward, vc user_filename, vc logical_clock, vc special_type,
          vc from_group)
{
    DwString s;
    init_msg_folder(from, &s);
    s += "" DIRSEPSTR "";
    s += (const char *)msg_id;
    s += ".bod";
    vc v(VC_VECTOR);
    v[QM_BODY_ID] = msg_id;
    v[QM_BODY_FROM] = from;
    v[QM_BODY_TEXT_do_not_use] = "";
    v[QM_BODY_ATTACHMENT] = attachment_id;
    v[QM_BODY_DATE] = date;
    v[QM_BODY_AUTH_VEC] = authvec;
    v[QM_BODY_FORWARDED_BODY] = forwarded_body;
    v[QM_BODY_NEW_TEXT] = new_text;
    v[QM_BODY_NO_FORWARD] = no_forward;
    v[QM_BODY_FILE_ATTACHMENT] = user_filename;
    v[QM_BODY_LOGICAL_CLOCK] = logical_clock;
    v[QM_BODY_SPECIAL_TYPE] = special_type;
    v[QM_BODY_FROM_GROUP] = from_group;
    if(save_info(v, s.c_str(), 1))
    {
        // can't do this here anymore because the index needs to
        // have the entire msg saved properly (including the
        // attachment.)
        //update_msg_idx(vcnil, v);
        return v;
    }
    return vcnil;
}

vc
direct_to_body2(vc m)
{
    vc v(VC_VECTOR);
    v[QM_BODY_FROM] = m[QQM_BODY_FROM];
    v[QM_BODY_TEXT_do_not_use] = "";
    v[QM_BODY_ATTACHMENT] = m[QQM_BODY_ATTACHMENT];
    v[QM_BODY_DATE] = m[QQM_BODY_DATE];
    v[QM_BODY_AUTH_VEC] = m[QQM_BODY_AUTH_VEC];
    v[QM_BODY_FORWARDED_BODY] = m[QQM_BODY_FORWARDED_BODY];
    v[QM_BODY_NEW_TEXT] = m[QQM_BODY_NEW_TEXT];
    v[QM_BODY_SPECIAL_TYPE] = m[QQM_BODY_SPECIAL_TYPE];
    v[QM_BODY_NO_FORWARD] = m[QQM_BODY_NO_FORWARD];
    v[QM_BODY_FILE_ATTACHMENT] = m[QQM_BODY_FILE_ATTACHMENT];
    v[QM_BODY_LOGICAL_CLOCK] = m[QQM_BODY_LOGICAL_CLOCK];
    v[QM_BODY_FROM_GROUP] = m[QQM_BODY_FROM_GROUP];

    return v;
}

vc
direct_to_body(vc msgid, vc& uid_out)
{
    vc huid = sql_get_uid_from_mid(msgid);
    if(huid.is_nil())
        return vcnil;
    uid_out = from_hex(huid);
    return load_body_by_id(uid_out, msgid);

}

static void
add_msg(vc vec, vc item)
{
    int n = vec.num_elems();
    int i;

    for(i = 0; i < n; ++i)
    {
        if(vec[i][QM_ID] == item[QM_ID])
        {
            vec[i] = item;
            return;
        }
    }
    //Refresh_users = 1;
    // can't do this here in a useful way, since the
    // addmsg can be done on a lot of different vectors
    //se_emit(SE_USER_MSG_RECEIVED, item[QM_ID]);

    // this is a bit of a hack... we don't have logical clock
    // values in the message summaries (maybe that should change)
    // so in order to present the messages in some kind of reasonable
    // time order, we insert them in order by date sent... as a side
    // effect, the list of unfetched messages will be presented to
    // the caller and if they just present it in order, it will look ok.
    // otherwise, if we don't do this, the messages will appear in random
    // order on startup (they are just loaded from the inbox in whatever order
    // the filesystem presents them.)
    // the proper way to fix this is to make the clients use the
    // logical clock and sort a model based on that... but some of the old
    // clients are too hard to fix in this area.

#if 1 || SORT_BY_DATE_SENT
// note: this is useful is the unread messages
// are displayed per-user. in icuii all unread
// messages are displayed together, so we
// just stick the message on the end of the list
// might want to sort by time received, but that
// is too big a change for right now.
    for(i = 0; i < n; ++i)
    {
        if(compare_date_vector(item[QM_DATE_SENT], vec[i][QM_DATE_SENT]) < 0)
        {
            vec.insert(item, i);
            break;
        }
    }
    if(i == n)
#endif
        vec.append(item);
}

vc
find_cur_msg(vc msg_id)
{
    int n = Cur_msgs.num_elems();
    int i;
    for(i = 0; i < n; ++i)
    {
        if(Cur_msgs[i][QM_ID] == msg_id)
        {
            return Cur_msgs[i];
        }
    }
    return vcnil;
}

static
vc
decrypt_special(vc mid, vc msg)
{
    vc dm(VC_VECTOR);
    dm[QQM_RECIP_VEC] = vc(VC_VECTOR);
    dm[QQM_RECIP_VEC][0] = My_UID;
    vc dmsg = msg;
    vc from = msg[QQM_BODY_FROM];
    if(!msg[QQM_BODY_DHSF].is_nil())
    {
        dmsg = decrypt_msg_body(msg);
        if(dmsg.is_nil())
        {
            // if we can't decrypt it, there is really something
            // wrong with the message, but it could be a lot of things
            // that we can't figure out easily (like the attachment
            // might be corrupt or truncated, our local private
            // key might have changed, etc.) chances are though, we
            // will never be able to decrypt it, so we might as well
            // delete it. if our key has gotten mucked up locally,
            // we may also *never* be able to decrypt tons of things
            // until the key is reset in the server.

            // note: don't ack it automatically, since we *might* be in a situation where we
            // are waiting for a group key. once the group key is installed we might be
            // able to decrypt it. tag the msg locally in case our key situation changes
            sql_add_tag(mid, "_decrypt_failed");
            TRACK_ADD(MR_msg_decrypt_failed, 1);
            return vcnil;
        }
        else
        {
            TRACK_ADD(MR_msg_decrypt_ok, 1);
        }
    }
    dm[QQM_MSG_VEC] = dmsg;

    //note: generate a new id for the message in its
    // local form. there is a lot of code that
    // depends on deleting the old message id
    // after it is fetched from the server. so
    // we just pretend this is a brand new message.

    // note: the above confused people, just try to
    // morph it into a different kind of message without
    // changing the id
    dm[QQM_LOCAL_ID] = mid;
    return dm;
}

struct special_map
{
    const char *name;
    int code;
};

static special_map Sm[] = {
    {"palreq", DWYCO_SUMMARY_PAL_AUTH_REQ},
    {"palok", DWYCO_SUMMARY_PAL_OK},
    {"palrej", DWYCO_SUMMARY_PAL_REJECT},
    {"dlv", DWYCO_SUMMARY_DELIVERED},
    {"user", DWYCO_SUMMARY_SPECIAL_USER_DEFINED},
    {"join1", DWYCO_SUMMARY_JOIN1},
    {"join2", DWYCO_SUMMARY_JOIN2},
    {"join3", DWYCO_SUMMARY_JOIN3},
    {"join4", DWYCO_SUMMARY_JOIN4},

    {0, 0}
};

static
int
special_state(vc body, int& what_out)
{
    vc sv = body[QM_BODY_SPECIAL_TYPE];
    if(sv.is_nil())
        return 0;
    vc what = sv[0];
    const char *whats = (const char *)what;
    // args are in a vector at sv[1]

    struct special_map *sm = &Sm[0];
    while(sm->name)
    {
        if(strcmp(sm->name, whats) == 0)
        {
            what_out = sm->code;
            return 1;
        }
        ++sm;
    }
    what_out = DWYCO_SUMMARY_SPECIAL_USER_DEFINED;
    return 1;
}

static
int
process_join(vc msg)
{
    vc body = direct_to_body2(msg[QQM_MSG_VEC]);
    int jstate;
    if(!special_state(body, jstate))
        return 0;
    switch(jstate)
    {
    case DWYCO_SUMMARY_JOIN1:
    case DWYCO_SUMMARY_JOIN2:
    case DWYCO_SUMMARY_JOIN3:
    case DWYCO_SUMMARY_JOIN4:
        break;
    default:
        return -1;
    }

    vc sv = body[QM_BODY_SPECIAL_TYPE];
    vc msg_type_vec = sv[1];

    vc payload = msg_type_vec[0];
    if(payload.type() != VC_STRING)
        return 0;
    int ret = 0;
    vc password;
//    if(Current_alternate)
//        password = Current_alternate->password;
//    else
        password = DH_alternate::Group_join_password;
    vc from = body[QM_BODY_FROM];
    switch(jstate)
    {
    case DWYCO_SUMMARY_JOIN1:
        ret = recv_gj1(from, payload, password);
        break;
    case DWYCO_SUMMARY_JOIN2:
        ret = recv_gj2(from, payload, password);
        break;
    case DWYCO_SUMMARY_JOIN3:
        ret = recv_gj3(from, payload, password);
        break;
    case DWYCO_SUMMARY_JOIN4:
        ret = install_group_key(from, payload, password);
        break;
    default:
        oopanic("huh?");
    }
    return ret;
}

static
void
get_special_done(vc m, void *, vc msg_id, ValidPtr vp)
{
    if(m[1].is_nil())
    {
        return;
    }
    // returned item is a vector of 2 items:
    // 0: msg id
    // 1: msg
    //	msg is a vector:
    // 0: sender id
    // 1: the text message
    // 2: id of any attachment
    // 3: date vector
    // 4: rating
    // 5: authvec
    // 6: forwarded body
    // 7: new text
    // 8: attachment loc
    // 9: special type

    vc msg = m[1][1];
    vc mid = m[1][0];

    if(msg.is_nil())
    {
        //q->MessageBox("Can't find message on server.");
        dirth_send_ack_get2(My_UID, mid, QckDone(0, 0));
        return;
    }

    //vc from = msg[QQM_BODY_FROM];

    if(msg[QQM_BODY_ATTACHMENT].is_nil())
    {
        vc nmsg = decrypt_special(mid, msg);
        if(!nmsg.is_nil())
        {
            if(process_join(nmsg) == 1)
            {
                // we processed it, so we delete it for everyone
                // else in the group. hopefully that will short-circuit
                // a lot of thrashing for bigger groups.
                // this might be a problem if we can't follow up with the
                // rest of the protocol for some reason, but a user can
                // retry it explicitly if it doesn't work the first time.
                //dirth_send_addtag(My_UID, mid, "_del", QckDone(0, 0));
            }
        }
    }
    // note: for now, we just delete all special msgs that are not
    // join, since we don't understand them anyways.

    // note: just send ack_get, with no return, assume it always works.
    // if it doesn't work, it is no big deal, we just get a message
    // twice.
    dirth_send_ack_get2(My_UID, mid, QckDone(0, 0));
}

static void
query_done(vc m, void *, vc, ValidPtr)
{
    if(m[1].is_nil())
        return;

    const vc v2 = m[1];

    try {
    sql_start_transaction();
    Cur_msgs = vc(VC_VECTOR);
    sql_remove_tag("_remote");

    int i;
    for(i = 0; i < v2.num_elems(); ++i)
    {
        vc v = v2[i];
        // 0: who from
        // 1: len
        // 2: id on server
        // 3: date vector

        vc from = v[QM_FROM];

        if(uid_ignored(from))
        {
            // rathole it
            {
                vc args(VC_VECTOR);
                args.append(from);
                args.append(v[QM_ID]);
                dirth_send_ack_get(My_UID, v[QM_ID], QckDone(ack_get_done2, 0, args));
            }
            continue;
        }
        init_msg_folder(from);
        //se_emit(SE_USER_ADD, from);
        // it was a problem trying to let special messages percolate
        // thru into the client api. so, just strip them out and
        // process them internally now
        if(!v[QM_SPECIAL_TYPE].is_nil())
        {
            dirth_send_get2(My_UID, v[QM_ID], QckDone(get_special_done, 0));
            continue;
        }

        // this logical clock stuff corresponds to "receiving" the message
        // the first time we see the message summary from the server.
        // this is to avoid a situation where we are assigning logical clocks
        // when the message is finished fetching, which isn't quite right (could
        // happen in random order, which looks goofy when we re-order by lc)
        // this is a hack, for sure... really need to have some consistent way of
        // assiging lc's for each path thru the system. maybe the server even
        // needs to have its own lc
        vc mid = v[QM_ID];
        // if we have already fetched the message and it is local, just
        // ack it automatically so we don't keep refetching it.
        if(sql_is_mid_local(mid))
        {
            dirth_send_ack_get2(My_UID, mid, QckDone(0, 0));
            continue;
        }
        if(sql_mid_has_tombstone(mid))
        {
            dirth_send_ack_get(My_UID, mid, QckDone(0, 0));
            // hmmm, wonder what i was thinking here, i left the
            // continue out  before...
            continue;
        }
        if(!Mid_to_logical_clock.contains(mid))
        {
            long lc;
            if(v[QM_LOGICAL_CLOCK].is_nil())
                lc = ++Logical_clock;
            else
                lc = (long)v[QM_LOGICAL_CLOCK];
            if(Logical_clock > lc)
                lc = ++Logical_clock;
            else if(lc > Logical_clock)
            {
                Logical_clock = lc + 1;
            }
            Mid_to_logical_clock.add_kv(mid, lc);
        }
        // NOTE: here is a case where if we already have the msg
        // in our local index, we could just immediately ack it.
        add_msg(Cur_msgs, v);
        sql_add_tag(mid, "_remote");
        DwString a("_");
        a += (const char *)to_hex(from);
        sql_remove_mid_tag(mid, a.c_str());
        sql_add_tag(mid, a.c_str());
    }
    sql_commit_transaction();
    }
    catch(...)
    {
        // we could probably restore the previous value of Cur_msgs here, but
        // if this happens, the system is probably on its way out
        Cur_msgs = vc(VC_VECTOR);
        sql_rollback_transaction();
    }

    Rescan_msgs = 1;
}

void
query_messages()
{
    dirth_send_query2(My_UID, QckDone(query_done, 0));
}

// return -1 if the message can never be delivered here
// callers can use this to determine when to delete
// messages that got delivered locally in error
int
store_direct(MMChannel *m, vc msg, void *)
{
    GRTLOG("store direct %p", m, 0);
    GRTLOGVC(msg);
    if(!valid_qd_message(msg))
    {
        TRACK_ADD(SD_invalid_msg, 1);
        if(m)
        {
            TRACK_ADD(SD_invalid_msg_link, 1);
            m->send_ctrl("ok");
        }
        return 1;
    }


    // if we're not the recipient then drop the
    // channel so it goes thru the server
    // vector(vector(recipients...) vector(fromid text_message attachid vector(yy dd hh mm ss) rating)

    if(!My_UID.is_nil() && msg[QQM_RECIP_VEC][0] != My_UID)
    {
        GRTLOG("ok not sent", 0, 0);
        TRACK_ADD(DR_bad_uid, 1);
        if(m)
            m->schedule_destroy(MMChannel::HARD);
        return -1;
    }

    // quick check to see if it is being spoofed
    // not foolproof, but close enough for now
    if(m && msg[QQM_MSG_VEC][QQM_BODY_FROM] != m->remote_uid())
    {
        TRACK_ADD(DR_bad_uid2, 1);
        m->send_ctrl("ok");
        return 1;
    }

    vc from = msg[QQM_MSG_VEC][QQM_BODY_FROM];

    if(uid_ignored(from))
    {
        TRACK_ADD(DR_bad_ignored, 1);
        // rathole it
        if(m)
        {
            m->send_ctrl("ok");
        }
        return 1;
    }

    // note: msgs sent directly are not encrypted because the channel
    // is encrypted.

    vc id;

    if(!m)
    {
        // we're loading the inbox from disk, so
        // just use the previous id that was stored
        id = msg[QQM_LOCAL_ID];
    }
    else
    {
        // generate a local id for the message
        id = to_hex(gen_id());
        msg[QQM_LOCAL_ID] = id;
    }

    // note: doing the logical clock stuff here isn't quite right...
    // we really need to do it when we actually receive the messages, not
    // when it is fetched and stored.
#if 1
    // if the clock was set when we first saw the summary, use that
    vc lc;
    if(Mid_to_logical_clock.find(id, lc))
    {
        msg[QQM_MSG_VEC][QQM_BODY_LOGICAL_CLOCK] = lc;
    }
    else
    {
        // set our logical clock to the max, and re-write the clock
        // value in the message
        if(msg[QQM_MSG_VEC][QQM_BODY_LOGICAL_CLOCK].is_nil())
        {
            // just stick our clock value in there
            msg[QQM_MSG_VEC][QQM_BODY_LOGICAL_CLOCK] = ++Logical_clock;
        }
        else
        {
            long rclock = (long)msg[QQM_MSG_VEC][QQM_BODY_LOGICAL_CLOCK];
            if(rclock > Logical_clock)
            {
                Logical_clock = rclock + 1;

            }
            else
            {
                msg[QQM_MSG_VEC][QQM_BODY_LOGICAL_CLOCK] = ++Logical_clock;
            }
        }
    }
#endif

    // if this is a direct msg with attachment, the att is already here, so
    // we can put in the estimated size if needed

    init_msg_folder(from);

    // if someone connects to us, assume we can use
    // the channel for direct messages even tho
    // we might have flagged it for server sends
    // before. note: this doesn't work for messages that
    // have attachments. for example, if A is behind
    // a firewall, and B isn't, and A sends a message to
    // B, the connection will be set up. however, B will
    // not be able to send a direct message with an attachment
    // back to A because of A's firewall, and B will get
    // annoying messages about this. we need to have B
    // know they can send simple messages back, but not
    // attachment (ie, attachment msgs must go via the server.)
    if(m)
    {
        // clear out all direct message blocks if they are in
        // a device group.
        const vc uids = map_uid_to_uids(from);
        for(int i = 0; i < uids.num_elems(); ++i)
            No_direct_msgs.del(uids[i]);
    }

    Rescan_msgs = 1;
    // save_msg indexes the message too, so after store_direct
    // you can map mid to uid and so on.
    try
    {
        sql_start_transaction();
        if(save_msg(msg, id))
        {
            // avoid creating extra _inbox in case it came from
            // multiple sources. note, even tho the _inbox tag
            // isn't synced, it participates in the default
            // guid generation, so dups can occur. might want to
            // redo this so we can avoid dups like this for tags
            // that are only used locally.
            sql_remove_mid_tag(id, "_inbox");
            sql_add_tag(id, "_inbox");
            sql_remove_mid_tag(id, "_remote");
            if(!msg[QQM_BODY_SPECIAL_TYPE].is_nil())
                sql_add_tag(id, "_special");
            DwString a("_");
            a += (const char *)to_hex(from);
            sql_remove_mid_tag(id, a.c_str());
            if(m)
            {
                m->send_ctrl("ok");
                TRACK_ADD(DR_ok, 1);
            }
        }
        else
        {
            if(m)
            {
                m->send_ctrl("store failed");
                TRACK_ADD(DR_store_failed, 1);
            }
        }
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

    return 1;
}

#if 0
void
load_users(int only_recent, int *total_out)
{
    DwString s;

    MsgFolders = vc(VC_TREE);

    vc ret = sql_get_recent_users(only_recent, total_out);
    if(ret.is_nil() || ret.num_elems() == 0)
    {
        if(ret.is_nil())
        {
            TRACK_ADD(QM_UL_recent_fail, 1);
        }
        else
        {
            TRACK_ADD(QM_UL_recent_empty, 1);
        }
        if(only_recent)
            return load_users(0, total_out);
    }
    int n = ret.num_elems();
    TRACK_MAX(QM_UL_recent_count, n);
    for(int i = 0; i < n; ++i)
    {
        vc uid = from_hex(ret[i]);
        add_msg_folder(uid);
    }
}
#endif


void
load_users_from_files(int *total_out)
{
    DwString s;

    MsgFolders = vc(VC_TREE);

    FindVec fv(newfn("*.usr"));
    auto n = fv.num_elems();
    TRACK_MAX(QM_UL_count, n);
    for(int i = 0; i < n; ++i)
    {
        const WIN32_FIND_DATA &d = *fv[i];
        s = d.cFileName;
        const vc uid = dir_to_uid(s);
        if(uid.len() != 10)
            continue;
        add_msg_folder(uid);
        //MsgFolders.add_kv(uid, vcnil);
    }

    if(total_out)
        *total_out = MsgFolders.num_elems();

}

void
load_users_from_index(int recent, int *total_out)
{
    MsgFolders = vc(VC_TREE);

    vc ret = sql_get_recent_users(recent, total_out);
    if(ret.is_nil())
    {
        TRACK_ADD(QM_UL_recent_fail, 1);
        if(total_out)
            *total_out = 0;
        return;
    }
    else
    {
        int n = ret.num_elems();
        TRACK_MAX(QM_UL_recent_count, n);
        for(int i = 0; i < n; ++i)
        {
            vc uid = from_hex(ret[i]);
            add_msg_folder(uid);
        }
    }
}

static
vc
uid_to_short_text(const vc& uid)
{
    vc u = to_hex(uid);
    DwString chop((const char *)u);
    chop.remove(8);
    DwString ret;
    ret += "#";
    ret += chop;
    return ret.c_str();
}

vc
uid_to_handle(const vc& uid, int *cant_resolve_now)
{
    const vc& ai = make_best_local_info(uid, cant_resolve_now);
    return ai[0];
}

vc
uid_to_handle2(const vc& uid)
{
    const vc& ai = make_best_local_info(uid, 0);
    return ai[0];
}

vc
uid_to_location(const vc& uid, int *cant_resolve_now)
{
    const vc& ai = make_best_local_info(uid, cant_resolve_now);
    return ai[1];
}

// this is called to clean out all messages that might
// be on the server from a particular sender
void
ack_all(vc uid)
{
    int i;
    vc ackset(VC_VECTOR);
    //ack_all_direct_from(uid);
    // only thing left in Cur_msgs for uid is server messages
    for(i = 0; i < Cur_msgs.num_elems(); ++i)
    {
        if(Cur_msgs[i][QM_FROM] == uid)
        {
            ackset.append(Cur_msgs[i][QM_ID]);
        }
    }

    sql_start_transaction();
    for(i = 0; i < ackset.num_elems(); ++i)
    {
        vc args(VC_VECTOR);
        args.append(vcnil);
        args.append(ackset[i]);
        dirth_send_ack_get(My_UID, ackset[i], QckDone(ack_get_done2, 0, args));
        //dirth_send_addtag(My_UID, ackset[i], "_del", QckDone(ack_get_done2, 0, args));
        delete_msg2(ackset[i]);
        sql_remove_mid_tag(ackset[i], "_remote");
    }
    DwString tag("_");
    tag += (const char *)to_hex(uid);
    vc mids = sql_get_tagged_mids2(tag.c_str());
    for(int i = 0; i < mids.num_elems(); ++i)
    {
        sql_remove_all_tags_mid(mids[i][0]);
    }
    sql_commit_transaction();

}

static
int
remove_user_files(vc dir, const char *pfx, int keep_folder)
{
    int i;
    if(dir.len() == 0)
        return 0;
    DwString s((const char *)dir);

    if(s.length() == 0)
        return 0;
    if(s.rfind(".usr") != s.length() - 4)
        return 0;
    DwString s2 = s;
    s2.remove(s2.length() - 4);
    if(s2.find_first_not_of("0123456789abcdef") != DwString::npos)
        return 0;
    vc uid = from_hex(s2.c_str());

    int god = (uid == TheMan);

    DwString p = pfx;
    p += s;
    p = newfn(p);
    p += "" DIRSEPSTR "*.*";
    s = p;

    int retval = 1;
    FindVec fv(s);
    int n = fv.num_elems();

    for(i = 0; i < n; ++i)
    {
        const WIN32_FIND_DATA& d = *fv[i];
        if(strcmp(d.cFileName, ".") == 0 ||
                strcmp(d.cFileName, "..") == 0)
            continue;
        if(god && strcmp(d.cFileName, "info") == 0)
            continue;
        DwString s2((const char *)dir);
        DwString p2 = pfx;
        p2 += s2;
        p2 = newfn(p2);
        p2 += "" DIRSEPSTR "";
        p2 += d.cFileName;
        if(!DeleteFile(p2.c_str()))
            retval = 0;
    }
    p = pfx;
    p += (const char *)dir;
    if(!god &&  (!keep_folder && !RemoveDirectory(newfn(p).c_str())))
        retval = 0;

    return 1; // tired of hearing tech support about not being able to
    // remove things, not sure why this happens (maybe they
    // have a video open when they try to remove it or something.)
    //return retval;
}

static
int
clear_user_msgs(const vc& uid)
{
    try
    {
        sql_start_transaction();
        sql_remove_all_tags_uid(uid);
        clear_msg_idx_uid(uid);
        DwString tag("_");
        tag += (const char *)to_hex(uid);
        vc mids = sql_get_tagged_mids2(tag.c_str());
        for(int i = 0; i < mids.num_elems(); ++i)
        {
            sql_remove_all_tags_mid(mids[i][0]);
        }
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
        return 0;
    }
    return 1;
}

int
remove_user(const vc& uid)
{

    int ret = clear_user_msgs(uid);
    if(!ret)
        return 0;

    const vc uids = map_uid_to_uids(uid);
    for(int i = 0; i < uids.num_elems(); ++i)
    {
        const vc u = uids[i];
        const vc dir = uid_to_dir(u);
        remove_user_files(dir, "", 0);
        MsgFolders.del(u);
        ack_all(u);
        pal_del(u, 1);
        prf_invalidate(u);
        Session_infos.del(u);
    }
    // note: the uid here is mapped down to representative
    se_emit(SE_USER_REMOVE, uid);
    Rescan_msgs = 1;
    return 1;
}

int
clear_user(const vc& uid)
{
    int ret = clear_user_msgs(uid);
    if(!ret)
        return 0;

    const vc uids = map_uid_to_uids(uid);
    for(int i = 0; i < uids.num_elems(); ++i)
    {
        const vc dir = uid_to_dir(uids[i]);
        remove_user_files(dir, "", 1);
        ack_all(uids[i]);
    }
    Rescan_msgs = 1;
    return 1;
}

static
int
trash_file(const DwString& dir, const DwString& fn)
{
    if(!is_user_dir(dir))
        return 0;
    if(!(is_msg_fn(fn) || is_attachment(fn)))
        return 0;

    DwString s2(dir);
    s2 = newfn(s2);
    s2 += DIRSEPSTR;
    s2 += fn;
    DwString trashdir = newfn("trash");
    trashdir += DIRSEPSTR;
    trashdir += dir;
    mkdir(trashdir.c_str());
    DwString trashfile = trashdir;
    trashfile += DIRSEPSTR;
    trashfile += fn;
    if(!move_replace(s2, trashfile))
        return 0;
    return 1;
}

void
trash_body(vc uid, vc msg_id, int inhibit_indexing)
{
    if(uid.type() != VC_STRING || uid.len() != 10)
        return;
    DwString huid((const char *)to_hex(uid));
    DwString mid((const char *)msg_id);

    huid += ".usr";
    DwString userdir = huid;
    huid = newfn(huid);
    huid += DIRSEPSTR;
    huid += mid;
    DwString s2 = huid;
    huid += ".bod";

    vc msg;
    if(load_info(msg, huid.c_str(), 1))
    {
        if(!inhibit_indexing)
        {
            sql_start_transaction();
            remove_msg_idx(uid, msg_id);
            sql_remove_all_tags_mid(msg_id);
            sql_commit_transaction();
        }
        if(!msg[QM_BODY_ATTACHMENT].is_nil())
            trash_file(userdir, (const char *)msg[QM_BODY_ATTACHMENT]);
        trash_file(userdir, (mid + ".bod"));
        return;
    }
    s2 += ".snt";
    if(load_info(msg, s2.c_str(), 1))
    {
        if(!inhibit_indexing)
        {
            sql_start_transaction();
            remove_msg_idx(uid, msg_id);
            sql_remove_all_tags_mid(msg_id);
            sql_commit_transaction();
        }
        if(!msg[QM_BODY_ATTACHMENT].is_nil())
            trash_file(userdir, (const char *)msg[QM_BODY_ATTACHMENT]);
        trash_file(userdir, (mid + ".snt"));
        return;
    }
}


void
untrash_users()
{
    int i;

    DwString trashdir = newfn("trash");
    trashdir += DIRSEPSTR;
    DwString s = trashdir;
    s += "*.usr";

    FindVec fv(s);
    int n = fv.num_elems();

    for(i = 0; i < n; ++i)
    {
        const WIN32_FIND_DATA& d = *fv[i];
        if(strcmp(d.cFileName, ".") == 0 ||
                strcmp(d.cFileName, "..") == 0)
            continue;
        vc uid = dir_to_uid(d.cFileName);
        init_msg_folder(uid);

        // now go thru all the files in the
        // trash dir and move them back
        DwString s2 = trashdir;
        s2 += d.cFileName;
        DwString dir = s2;

        s2 += "" DIRSEPSTR "*.*";
        FindVec fv2(s2);
        int n2 = fv2.num_elems();
        for(int j = 0; j < n2; ++j)
        {
            const WIN32_FIND_DATA& d2 = *fv2[j];
            if(strcmp(d2.cFileName, ".") == 0 ||
                    strcmp(d2.cFileName, "..") == 0)
                continue;
            DwString src = trashdir;
            src += d.cFileName;
            src += "" DIRSEPSTR "";
            src += d2.cFileName;
            DwString dst = d.cFileName;
            dst = newfn(dst);
            dst += "" DIRSEPSTR "";
            dst += d2.cFileName;
            // note: don't delete the target, want to
            // fail quietly if it exists, don't want
            // to overwrite info files and so on.
            move_replace(src, dst);
        }
        RemoveDirectory(dir.c_str());
        clear_indexed_flag(uid);
        load_msg_index(uid, 1);
    }
}

int
count_trashed_users()
{
    FindVec fv(newfn("trash" DIRSEPSTR "*.usr"));
    int num = fv.num_elems();
    return num;
}

int
empty_trash()
{
    FindVec fv(newfn("trash" DIRSEPSTR "*.usr"));
    int num = fv.num_elems();
    for(int i = 0; i < num; ++i)
    {
        const WIN32_FIND_DATA& d = *fv[i];
        if(strcmp(d.cFileName, ".") == 0 ||
                strcmp(d.cFileName, "..") == 0)
            continue;
        remove_user_files(d.cFileName, "trash" DIRSEPSTR "", 0);
    }
    return 1;
}

vc
load_msgs(vc uid)
{
    vc ret(VC_VECTOR);
    if(!uid.is_nil())
    {
        vc muid = map_to_representative_uid(uid);

        for(int i = 0; i < Cur_msgs.num_elems(); ++i)
        {
            const vc cm = Cur_msgs[i];
            vc fuid = map_to_representative_uid(cm[QM_FROM]);
            if(fuid == muid)
            {
                if(sql_mid_has_tag(cm[QM_ID], "_ack"))
                    continue;
                vc cpy = cm.copy();
                cpy[QM_FROM] = fuid;
                ret.append(cpy);
            }
        }
    }
    else
    {
        for(int i = 0; i < Cur_msgs.num_elems(); ++i)
        {
            const vc cm = Cur_msgs[i];
            if(sql_mid_has_tag(cm[QM_ID], "_ack"))
                continue;
            vc cpy = cm.copy();
            cpy[QM_FROM] = map_to_representative_uid(cpy[QM_FROM]);
            ret.append(cpy);
        }
    }
    return ret;
}



struct GroovyItem
{
    vc item;

    GroovyItem() {}
    GroovyItem(vc v) {
        item = v;
    }
    int operator==(const GroovyItem& g) const {
        if(item[QM_BODY_ID] == g.item[QM_BODY_ID])
            return 1;
        return 0;
    }
    int operator<(const GroovyItem& g) const {
        // if both items have logical clocks, use that
        if(!item[QM_BODY_LOGICAL_CLOCK].is_nil() &&
                !g.item[QM_BODY_LOGICAL_CLOCK].is_nil())
        {
            if(item[QM_BODY_LOGICAL_CLOCK] > g.item[QM_BODY_LOGICAL_CLOCK])
                return 1;
            return 0;
        }
        else if(!item[QM_BODY_LOGICAL_CLOCK].is_nil() && g.item[QM_BODY_LOGICAL_CLOCK].is_nil())
            return 1;
        else if(!g.item[QM_BODY_LOGICAL_CLOCK].is_nil() && item[QM_BODY_LOGICAL_CLOCK].is_nil())
            return 0;
        int cdv = compare_date_vector(item[QM_BODY_DATE], g.item[QM_BODY_DATE]);
        if(cdv == 1)
            return 1;
        else if(cdv == -1)
            return 0;
        // use mid to stabilize sort
        if(item[QM_BODY_ID] < g.item[QM_BODY_ID])
            return 1;
        return 0;
    }
};


vc
load_bodies(vc dir, int load_sent)
{
    int i = 1;
    DwString s((const char *)dir);
    vc ret(VC_VECTOR);

    DwTreeKaz<int, GroovyItem> sorter(0);

    DwString ss = s;
    s = newfn(s);
    DwString dpref = s;
    s += "" DIRSEPSTR "*.bod";
    int t = 0;

    FindVec fv(s);
    int n = fv.num_elems();
    for(i = 0; i < n; ++i)
    {
        const WIN32_FIND_DATA &d = *fv[i];
        DwString s2(dpref);
        s2 += "" DIRSEPSTR "";
        s2 += d.cFileName;
        vc info;
        if(load_info(info, s2.c_str(), 1))
        {
            //ret.append(info);
            sorter.add(GroovyItem(info), t++);
        }
    }

    if(load_sent)
    {
        ss = newfn(ss);
        ss += "" DIRSEPSTR "*.snt";
        s = ss;
        FindVec fv2(s);
        n = fv2.num_elems();
        for(i = 0; i < n; ++i)
        {
            const WIN32_FIND_DATA &d = *fv2[i];
            DwString s2(dpref);
            s2 += "" DIRSEPSTR "";
            s2 += d.cFileName;
            vc info;
            if(load_info(info, s2.c_str(), 1))
            {
                //ret.append(info);
                info[QM_BODY_SENT] = vctrue;
                sorter.add(GroovyItem(info), t++);
            }
        }
    }

    DwTreeKazIter<int, GroovyItem> iter(&sorter);

    i = 0;
    for(; !iter.eol(); iter.forward())
    {
        DwAssocImp<int, GroovyItem> a = iter.get();
        ret[i++] = a.peek_key().item;
    }
    return ret;
}


// note: this assumes that the largest logical_clock
// is in the first item in the list. this may not  be
// needed any longer, now that we have gone full koolaide
// on the indexing, and sqlite can get the largest lc out
// in an index without too much trouble.
void
boost_clock(vc mi)
{
    if(mi.num_elems() > 0 && !mi[0][QM_IDX_LOGICAL_CLOCK].is_nil())
    {
        if((long)mi[0][QM_IDX_LOGICAL_CLOCK] > Logical_clock)
        {
            Logical_clock = (long)mi[0][QM_IDX_LOGICAL_CLOCK] + 1;
        }
    }

}


static
vc
load_body(vc user_dir, vc msg_id)
{
    DwString s((const char *)user_dir);
    DwString t((const char *)msg_id);
    s = newfn(s);
    s += "" DIRSEPSTR "";
    s += t;
    DwString base = s;
    s += ".bod";
    vc info;
    if(load_info(info, s.c_str(), 1))
    {
        return info;
    }
    base += ".snt";
    if(load_info(info, base.c_str(), 1))
    {
        info[QM_BODY_SENT] = vctrue;
        return info;
    }
    return vcnil;
}

vc
load_body_by_id(vc uid, vc msg_id)
{
    if(uid.len() == 0)
        return vcnil;
    return load_body(uid_to_dir(uid), msg_id);
}

void
delete_msg2(vc msg_id)
{
    int n = Cur_msgs.num_elems();
    int i;
    for(i = 0; i < n; ++i)
    {
        if(Cur_msgs[i][QM_ID] == msg_id)
        {
            Cur_msgs.remove(i, 1);
            Rescan_msgs = 1;
            return;
        }
    }
}


//void
//delete_body2(vc user_id, vc msg_id)
//{
//    if(user_id.len() == 0)
//        return;
//    DwString s((const char *)to_hex(user_id));
//    DwString t((const char *)msg_id);

//    s += ".usr";
//    s = newfn(s);
//    s += DIRSEPSTR "";
//    s += t;
//    s += ".bod";

//    DeleteFile(s.c_str());
//    remove_msg_idx(user_id, msg_id);
//    sql_fav_remove_mid(msg_id);
//}

void
delete_body3(vc uid, vc msg_id, int inhibit_indexing)
{
    if(uid.len() == 0)
        return;
    DwString s((const char *)to_hex(uid));
    DwString t((const char *)msg_id);

    s += ".usr";
    s = newfn(s);
    s += DIRSEPSTR;
    s += t;
    DwString s2 = s;
    s += ".bod";

    vc msg;
    if(load_info(msg, s.c_str(), 1))
    {
        if(!inhibit_indexing)
        {
            sql_start_transaction();
            remove_msg_idx(uid, msg_id);
            sql_remove_all_tags_mid(msg_id);
            sql_commit_transaction();
        }
        if(!msg[QM_BODY_ATTACHMENT].is_nil())
            delete_attachment2(uid, msg[QM_BODY_ATTACHMENT]);
        DeleteFile(s.c_str());
        return;
    }
    s2 += ".snt";
    if(load_info(msg, s2.c_str(), 1))
    {
        if(!inhibit_indexing)
        {
            sql_start_transaction();
            remove_msg_idx(uid, msg_id);
            sql_remove_all_tags_mid(msg_id);
            sql_commit_transaction();
        }
        if(!msg[QM_BODY_ATTACHMENT].is_nil())
            delete_attachment2(uid, msg[QM_BODY_ATTACHMENT]);
        DeleteFile(s2.c_str());
        return;

    }
}

void
delete_attachment2(vc user_id, vc attachment_name)
{
    if(!is_attachment(attachment_name))
        return;
    if(user_id.len() == 0)
        return;
    DwString s((const char *)to_hex(user_id));
    DwString t((const char *)attachment_name);

    s += ".usr";
    s = newfn(s);
    s += DIRSEPSTR ;
    s += t;

    DeleteFile(s.c_str());
}


static vc
date_vector()
{
    vc v(VC_VECTOR);
    time_t t = time(0);
    DwTime tm;
    // this is the local time with timezone and all that, which is
    // useful for the recipient, and hard to calculate for them...
    v[2] = tm.Hour();
    v[3] = tm.Minute();
    v[4] = tm.Second();
    // note: this needs to be fixed so that internally all vc's are 64 bit
    // even in 32bit environments
    v[5] = (long)t;
    return v;
}


// q'd message has the form:
// vector(
//	 vector(recipients...)
//   vector(fromid text_message attachid vector(yy dd hh mm ss) rating)
// )
static int
q_message2(vc recip, const char *attachment, vc& msg_out,
           vc body_to_forward, const char *new_text, vc att_hash, vc special_type, vc st_arg1, int no_forward, vc user_filename, int save_sent)
{

    // if this has a file attachment, ignore
    // the no-forward stuff, since we can't control
    // the file attachment anyway.
    if(!user_filename.is_nil())
    {
        no_forward = 0;
//		if(!special_type.is_nil())
//			return 0;
    }

    vc qmsg(VC_VECTOR);

    qmsg[QQM_RECIP_VEC] = recip;
    qmsg[QQM_SAVE_SENT] = save_sent ? vctrue : vcnil;

    vc m(VC_VECTOR);
    m[QQM_BODY_FROM] = My_UID;
    m[QQM_BODY_TEXT_do_not_use] = "";
    if(!attachment || strlen(attachment) == 0)
        m[QQM_BODY_ATTACHMENT] = vcnil;
    else
        m[QQM_BODY_ATTACHMENT] = attachment;

    m[QQM_BODY_DATE] = date_vector();
    m[QQM_BODY_LOGICAL_CLOCK] = ++Logical_clock;
    m[QQM_BODY_FORWARDED_BODY] = body_to_forward;
    m[QQM_BODY_NEW_TEXT] = new_text;

    // sending a file as an attachment, this is
    // the name (like "c:\foo.jpg") that can be
    // used on the recipient side for storing the
    // file.
    m[QQM_BODY_FILE_ATTACHMENT] = user_filename;

    qmsg[QQM_MSG_VEC] = m;

    if(special_type == vc("palreq"))
    {
        // request for pal add
        vc v(VC_VECTOR);
        v.append("palreq");
        vc sv(VC_VECTOR);
        v.append(sv);
        sv.append(st_arg1);
        m[QQM_BODY_SPECIAL_TYPE] = v;
    }
    else if(special_type == vc("palok"))
    {
        vc v(VC_VECTOR);
        v.append("palok");
        vc sv(VC_VECTOR);
        v.append(sv);
        sv.append(st_arg1);
        m[QQM_BODY_SPECIAL_TYPE] = v;
    }
    else if(special_type == vc("palrej"))
    {
        vc v(VC_VECTOR);
        v.append("palrej");
        vc sv(VC_VECTOR);
        v.append(sv);
        m[QQM_BODY_SPECIAL_TYPE] = v;
    }
    else if(special_type == vc("admin"))
    {
        vc v(VC_VECTOR);
        v.append("admin");
        vc sv(VC_VECTOR);
        v.append(sv);
        m[QQM_BODY_SPECIAL_TYPE] = v;
    }
    else if(special_type == vc("backup"))
    {
        vc v(VC_VECTOR);
        v.append("backup");
        vc sv(VC_VECTOR);
        v.append(sv);
        m[QQM_BODY_SPECIAL_TYPE] = v;
    }
    else if(special_type == vc("user"))
    {
        vc v(VC_VECTOR);
        v.append("user");
        vc sv(VC_VECTOR);
        sv.append(st_arg1);
        v.append(sv);
        m[QQM_BODY_SPECIAL_TYPE] = v;
    }
    else if(special_type.len() > 4 && vc(VC_BSTRING, special_type, 4) == vc("join"))
    {
        vc v(VC_VECTOR);
        v.append(special_type);
        vc sv(VC_VECTOR);
        sv.append(st_arg1);
        v.append(sv);
        m[QQM_BODY_SPECIAL_TYPE] = v;
    }
    m[QQM_BODY_NO_FORWARD] = no_forward;
    // inhibit server-based delivery reporting for now
    m[QQM_BODY_NO_DELIVERY_REPORT] = vctrue;
    // this helps the recipient display the message in the right place.
    // it can get out of date, which will lead to strangeness, and
    // at the moment, we are not authenticating messages in a way that
    // would keep spoofing from happening. if the client really wants it
    // maybe they could verify the group membership of the uid, but that
    // isn't implemented right now.
    if(Current_alternate)
        m[QQM_BODY_FROM_GROUP] = Current_alternate->get_gid();
    else
        m[QQM_BODY_FROM_GROUP] = vcnil;

    gen_authentication(qmsg, att_hash);
    // note: get rid of no-forward encoding hackery.
    // means old software will not be able to view
    // no-forward msgs sent from new software.
    msg_out = qmsg;

    return 1;
}

#if 0
// msg should be in QQM format
vc
decrypt_msg_qqm(vc emsg)
{
    GRTLOG("dec msg qqm", 0, 0);
    GRTLOGVC(emsg);

    vc body = emsg[QQM_MSG_VEC];

    vc prvkeys(VC_VECTOR);
    prvkeys[0] = dh_my_static();
    prvkeys[1] = Current_alternate->my_static();

    vc key = dh_store_and_forward_get_key2(body[QQM_BODY_DHSF], prvkeys);
    if(key.type() != VC_STRING)
        return vcnil;
    vc ectx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(ectx, key, 0);
    vc msg_out;
    if(encdec_xfer_dec_ctx(ectx, body[QQM_BODY_EMSG], msg_out).is_nil())
    {
        GRTLOG("decrypt msg body failed %s", (const char *)to_hex(key), 0);
        return vcnil;
    }

    if(body[QQM_BODY_ATTACHMENT].is_nil())
    {
        GRTLOGVC(msg_out);
        return msg_out;
    }
    // if there is an attachment, we have the key needed to decrypt it now
    // so do that
    DwString source = newfn((const char *)body[QQM_BODY_ATTACHMENT]);
    DwString dest = newfn((const char *)msg_out[QQM_BODY_ATTACHMENT]);
    DeleteFile(dest.c_str());
    if(decrypt_attachment(source.c_str(), key, dest.c_str()) == 0)
        return vcnil;
    return msg_out;
}
#endif

// note: this is used mainly for compat with older clients that may
// send messages to clients that can't be decrypted. this is used to
// short-circuit having to download any attachment that we ultimately
// will not be able to decrypt.
int
can_decrypt_msg_body(vc body)
{
    vc prvkeys(VC_VECTOR);
    prvkeys[0] = dh_my_static();

    // this is something i have to think about. even if we are not
    // set up in the current group, if we have a group key available
    // we should be able to decrypt a message sent with that key.
    // well, maybe not, this might mean we could decrypt something
    // from a group we are not in anymore (like we might have gotten
    // booted out of for some reason.)
    vc ak = DH_alternate::get_all_keys();
    if(!ak.is_nil())
    {
        for(int i = 0; i < ak.num_elems(); ++i)
        {
            prvkeys.append(ak[i]);
        }
    }

    //prvkeys[1] = Current_alternate->my_static();

    vc key = dh_store_and_forward_get_key2(body[QQM_BODY_DHSF], prvkeys);
    if(key.type() != VC_STRING)
        return 0;
    vc ectx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(ectx, key, 0);
    vc msg_out;
    if(encdec_xfer_dec_ctx(ectx, body[QQM_BODY_EMSG], msg_out).is_nil())
    {
        GRTLOG("decrypt msg body failed %s", (const char *)to_hex(key), 0);
        return 0;
    }

    return 1;
}

vc
decrypt_msg_body(vc body)
{
    GRTLOG("dec msg body", 0, 0);
    GRTLOGVC(body);

    vc prvkeys(VC_VECTOR);
    prvkeys[0] = dh_my_static();

    // this is something i have to think about. even if we are not
    // set up in the current group, if we have a group key available
    // we should be able to decrypt a message sent with that key.
    // well, maybe not, this might mean we could decrypt something
    // from a group we are not in anymore (like we might have gotten
    // booted out of for some reason.)
    vc ak = DH_alternate::get_all_keys();
    if(!ak.is_nil())
    {
        for(int i = 0; i < ak.num_elems(); ++i)
        {
            prvkeys.append(ak[i]);
        }
    }

    //prvkeys[1] = Current_alternate->my_static();

    vc key = dh_store_and_forward_get_key2(body[QQM_BODY_DHSF], prvkeys);
    if(key.type() != VC_STRING)
        return vcnil;
    vc ectx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(ectx, key, 0);
    vc msg_out;
    if(encdec_xfer_dec_ctx(ectx, body[QQM_BODY_EMSG], msg_out).is_nil())
    {
        GRTLOG("decrypt msg body failed %s", (const char *)to_hex(key), 0);
        return vcnil;
    }
    // if there is an attachment, we have the key needed to decrypt it now
    // so do that
    if(body[QQM_BODY_ATTACHMENT].is_nil())
    {

        GRTLOGVC(msg_out);
        return msg_out;
    }
    // if there is an attachment, we have the key needed to decrypt it now
    // so do that
    DwString source = newfn((const char *)body[QQM_BODY_ATTACHMENT]);
    DwString dest = newfn((const char *)msg_out[QQM_BODY_ATTACHMENT]);
    DeleteFile(dest.c_str());
    if(decrypt_attachment(source.c_str(), key, dest.c_str()) == 0)
        return vcnil;
    return msg_out;

}

#ifdef DWYCO_CRYPTO_PIPELINE
// decrypt everything but the attachment
// this is used to send large attachment decryption to another thread
// so we don't block on it here
vc
decrypt_msg_body2(vc body, DwString& src, DwString& dst, DwString& key_out)
{
    GRTLOG("dec msg body", 0, 0);
    GRTLOGVC(body);

    vc prvkeys(VC_VECTOR);
    prvkeys[0] = dh_my_static();
    prvkeys[1] = Current_alternate->my_static();

    vc key = dh_store_and_forward_get_key2(body[QQM_BODY_DHSF], prvkeys);
    if(key.type() != VC_STRING)
        return vcnil;
    vc ectx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(ectx, key, 0);
    vc msg_out;
    if(encdec_xfer_dec_ctx(ectx, body[QQM_BODY_EMSG], msg_out).is_nil())
    {
        GRTLOG("decrypt msg body failed %s", (const char *)to_hex(key), 0);
        return vcnil;
    }
    // if there is an attachment, we have the key needed to decrypt it now
    // so do that
    if(body[QQM_BODY_ATTACHMENT].is_nil())
    {

        GRTLOGVC(msg_out);
        return msg_out;
    }
    // if there is an attachment, we have the key needed to decrypt it now
    // so do that
    src = newfn((const char *)body[QQM_BODY_ATTACHMENT]);
    dst = newfn((const char *)msg_out[QQM_BODY_ATTACHMENT]);
    key_out = DwString((const char *)key, 0, key.len());
    //DeleteFile(dest.c_str());
    //if(decrypt_attachment(source.c_str(), key, dest.c_str()) == 0)
    //    return vcnil;
    return msg_out;

}
#endif


int
encrypt_attachment(vc filename, vc key, vc filename_dst)
{
    DwString rfns = gen_random_filename();
    rfns += ".tmp";
    rfns = newfn(rfns);

    try
    {
        GCM<AES>::Encryption e;

        vc r = get_entropy();

        e.SetKeyWithIV((const byte *)(const char *)key, key.len(), (const byte *)(const char *)r, 12);
        FileSink *fs = new FileSink(rfns.c_str());
        fs->Put((const byte *)(const char *)r, 12);

        FileSource *fso = new FileSource((const char *)filename, true,
                                         new AuthenticatedEncryptionFilter(e, fs, false, 12));
        delete fso;
    }
    catch(...)
    {
        DeleteFile(filename_dst);
        return 0;
    }

    // we go to a temporary and rename here to try and make the windows
    // for partially generated files smaller
    //DeleteFile((const char *)filename_dst);
    if(move_replace(rfns, (const char *)filename_dst) == 0)
    {
        DeleteFile(rfns.c_str());
        DeleteFile((const char *)filename_dst);
        return 0;
    }
    return 1;

}

int
decrypt_attachment(vc filename, vc key, vc filename_dst)
{
    FileSource *fs = 0;
    try
    {
        GCM<AES>::Decryption e;
        fs = new FileSource((const char *)filename, false);
        SecByteBlock iv(12);
        fs->Pump(iv.SizeInBytes());
        if(fs->Get(iv, iv.SizeInBytes()) < iv.SizeInBytes())
        {
            delete fs;
            return 0;
        }
        e.SetKeyWithIV((const byte *)(const char *)key, key.len(), iv, iv.SizeInBytes());

        fs->Attach(new AuthenticatedDecryptionFilter(e, new FileSink((const char *)filename_dst),
                   AuthenticatedDecryptionFilter::DEFAULT_FLAGS, 12));
        fs->PumpAll();
        delete fs;

    }
    catch(...)
    {
        DeleteFile(filename_dst);
        delete fs;
        GRTLOG("decrypt att failed %s %s", (const char *)filename, (const char *)to_hex(key));
        return 0;
    }
    return 1;
}

#ifdef DWYCO_CRYPTO_PIPELINE
int
decrypt_attachment2(const DwString& filename, const DwString& key, const DwString& filename_dst)
{
    vc f(VC_BSTRING, filename.c_str(), filename.length());
    vc k(VC_BSTRING, key.c_str(), key.length());
    vc d(VC_BSTRING, filename_dst.c_str(), filename_dst.length());
    return decrypt_attachment(f, k, d);
}
#endif


vc
encrypt_msg_qqm(vc msg, vc sf, vc ectx, vc key)
{
    if(sf.is_nil() || ectx.is_nil())
        return msg;
    // encrypting: just re-encapsulate the msg in a new message
    vc nmsg;
    vc body = msg[QQM_MSG_VEC];

    vc emsg = vclh_encdec_xfer_enc_ctx(ectx, body);
    vc att = body[QQM_BODY_ATTACHMENT];
    if(!att.is_nil())
    {
        if(!is_attachment(att))
            return msg;
        DwString ext((const char *)att);
        ext.remove(0, ext.length() - 4);
        DwString efn = gen_random_filename();
        efn += ext;
        if(encrypt_attachment(newfn(att).c_str(), key, newfn(efn).c_str()) == 0)
            return msg;
        att = efn.c_str();
    }

    q_message2(msg[QQM_RECIP_VEC],
               att.is_nil() ? "" : (const char *)att,
               nmsg, vcnil, "update", vcnil, vcnil, vcnil,
               !body[QQM_BODY_NO_FORWARD].is_nil(), vcnil, !msg[QQM_SAVE_SENT].is_nil());
    nmsg[QQM_MSG_VEC][QQM_BODY_DHSF] = sf;
    nmsg[QQM_MSG_VEC][QQM_BODY_EMSG] = emsg;
    // this stuff is imported into the un-encrypted part of the messages
    // so the server has some meta-data for management
    nmsg[QQM_MSG_VEC][QQM_BODY_ATTACHMENT_LOCATION] = body[QQM_BODY_ATTACHMENT_LOCATION];
    nmsg[QQM_MSG_VEC][QQM_BODY_ESTIMATED_SIZE] = body[QQM_BODY_ESTIMATED_SIZE];
    nmsg[QQM_MSG_VEC][QQM_BODY_LOGICAL_CLOCK] = body[QQM_BODY_LOGICAL_CLOCK];
    nmsg[QQM_MSG_VEC][QQM_BODY_SPECIAL_TYPE] = body[QQM_BODY_SPECIAL_TYPE];
    // on the server, we won't have access to the size of the encrypted text, so
    // maybe we could just fudge it and say it is the size of the encrypted package by
    // adding it into the "estimated size". that is a tiny leak, and maybe not
    // worth it, since the size is only used as a gross indicator how long a message
    // will take to fetch, and text-only messages are fairly tiny

    GRTLOG("EMSG", 0, 0);
    GRTLOGVC(msg);
    GRTLOGVC(nmsg);

    return nmsg;
}


int
q_message(vc recip, const char *attachment, DwString& fn_out,
          vc body_to_forward, const char *new_text, vc att_hash, vc special_type, vc st_arg1, int no_forward, vc user_filename, int save_sent)
{
    vc msg_out;

    if(!q_message2(recip, attachment, msg_out, body_to_forward, new_text, att_hash, special_type, st_arg1, no_forward, user_filename, save_sent))
    {
        return 0;
    }

    DwString fn = gen_random_filename();
    fn += ".q";
    fn_out = fn;

    if(!save_info(msg_out, fn.c_str()))
        return 0;
    return 1;
}

static
vc
qd_body_to_saved_body(vc qb)
{
    vc v(VC_VECTOR);
    // the id is changed since we are saving a copy of a sent
    // message.
    v[QM_BODY_ID] = to_hex(gen_id());
    v[QM_BODY_FROM] = qb[QQM_BODY_FROM];
    v[QM_BODY_TEXT_do_not_use] = qb[QQM_BODY_TEXT_do_not_use];
    v[QM_BODY_ATTACHMENT] = qb[QQM_BODY_ATTACHMENT];
    v[QM_BODY_DATE] = qb[QQM_BODY_DATE];
    v[QM_BODY_AUTH_VEC] = qb[QQM_BODY_AUTH_VEC];
    v[QM_BODY_FORWARDED_BODY] = qb[QQM_BODY_FORWARDED_BODY];
    v[QM_BODY_NEW_TEXT] = qb[QQM_BODY_NEW_TEXT];
    v[QM_BODY_NO_FORWARD] = qb[QQM_BODY_NO_FORWARD];
    v[QM_BODY_FILE_ATTACHMENT] = qb[QQM_BODY_FILE_ATTACHMENT];
    v[QM_BODY_LOGICAL_CLOCK] = qb[QQM_BODY_LOGICAL_CLOCK];
    v[QM_BODY_FROM_GROUP] = qb[QQM_BODY_FROM_GROUP];
    return v;
}

// store messages we send to local area. if the speced_mid
// is non-nil, use it for the local message id, so the server
// can send us delivery report.
// note: used to be that server handled multi-recipient messages,
// but since we don't do that anymore, having the one speced_mid
// is ok (otherwise we would have to return a list of mids from the server.)
vc
do_local_store(vc filename, vc speced_mid)
{
    vc m;
    if(!load_info(m, filename))
        return vcnil;
    if(!valid_qd_message(m))
    {
        DeleteFile(newfn(filename).c_str());
        return vcnil;
    }
    vc recip = m[0];

    m[1] = qd_body_to_saved_body(m[1]);
    int copy_attachment = !m[1][QM_BODY_ATTACHMENT].is_nil();
    vc afile = m[1][QM_BODY_ATTACHMENT];
    for(int i = 0; i < recip.num_elems(); ++i)
    {
        DwString s;
        init_msg_folder(recip[i], &s);

        DwString nfn;
        if(speced_mid.is_nil())
            nfn = gen_random_filename();
        else
            nfn = (const char *)speced_mid;

        m[1][QM_BODY_ID] = vc(nfn.c_str());
        DwString s2 = s;
        nfn += ".snt";
        s2 += "" DIRSEPSTR "";
        s2 += nfn;
        m[1][QM_BODY_SENT] = vctrue;
        if(copy_attachment)
        {
            // generate new attachment name in case we
            // are sending it to ourselves
            DwString new_aname = gen_random_filename();
            if(m[1][QM_BODY_FILE_ATTACHMENT].is_nil())
                new_aname += ".dyc";
            else
                new_aname += ".fle";
            DwString dn = s;
            dn += "" DIRSEPSTR "";
            dn += new_aname;
            CopyFile(newfn(afile).c_str(), dn.c_str(), 0);
            m[1][QM_BODY_ATTACHMENT] = vc(new_aname.c_str());
        }
        if(!save_info(m[1], s2.c_str(), 1))
        {
            DeleteFile(s2.c_str());
            return vcnil;
        }
        else
        {
            // we saved the message, but the indexing failed, need
            // a fix for this. possibly we can just schedule a
            // re-index for this user.
            if(!update_msg_idx(recip[i], m[1], 0))
            {
                // FIGURE IT OUT
            }
        }
    }
    return m[1];
}

// send a q'd message

int QSend_inprogress;
int QSend_special_inprogress;

void
move_back_to_outbox(const DwString &fn)
{
    DwString s = newfn("inprogress");
    s += DIRSEPSTR;
    DwString d = newfn("outbox");
    d += DIRSEPSTR;
    s += fn;
    d += fn;
    move_replace(s, d);
}

void
move_to_outbox(const DwString& fn)
{
    DwString d = newfn("outbox");
    d += DIRSEPSTR;
    d += fn;
    move_replace(newfn(fn), d);
}

void
move_in_progress(const DwString& fn, const DwString& dfn)
{
    DwString s = newfn("inprogress");
    s += DIRSEPSTR;
    s += fn;

    move_replace(s, newfn(dfn));
}


void
remove_in_progress(const DwString& fn)
{
    DwString s = newfn("inprogress");
    s += DIRSEPSTR;
    s += fn;
    DeleteFile(s.c_str());
}
void
remove_from_outbox(const DwString& fn)
{
    DwString s = newfn("outbox");
    s += DIRSEPSTR;
    s += fn;
    DeleteFile(s.c_str());
}


void
recover_inprogress()
{
    int i;

    FindVec fv(newfn("inprogress" DIRSEPSTR "*.q"));
    int nn = fv.num_elems();
    for(i = 0; i < nn; ++i)
    {
        const WIN32_FIND_DATA& n = *fv[i];
        DwString s("inprogress" DIRSEPSTR "");
        DwString d("outbox" DIRSEPSTR "");
        s += n.cFileName;
        d += n.cFileName;
        move_replace(newfn(s), newfn(d));
    }
}


static
void
reset_qsend(enum dwyco_sys_event cmd, const DwString& qid, const vc& recip_uid)
{
    if(cmd == SE_MSG_SEND_START)
        return;
    QSend_inprogress = 0;
}

static
void
reset_qsend_special(enum dwyco_sys_event cmd, const DwString& qid, const vc& recip_uid)
{
    if(cmd == SE_MSG_SEND_START)
        return;
    QSend_special_inprogress = 0;
}

#if 0
static
DwVec<int>
perm(int n)
{
    DwVec<int> ret;
    for(int i = 0; i < n; ++i)
    {
        int j;
        if(ret.num_elems() == 0)
            j = 0;
        else
            j = dwyco_rand() % (ret.num_elems() + 1);
        if(j == ret.num_elems())
        {
            ret.append(i);
        }
        else
        {
            ret.append(ret[j]);
            ret[j] = i;
        }
    }
    return ret;
}
#endif

static
vc
load_qb(const DwString& dir, const DwString& pers_id)
{
    DwString p(dir);
    p += DIRSEPSTR;
    p += pers_id;
    vc m;
    if(load_info(m, p.c_str()))
        return m;
    return vcnil;
}

static
void
sort_on_time(vc vec, int field)
{
    int s = vec.num_elems();
    for(int i=0; i<s; i++)
    {
        for(int j=i+1; j<s; j++)
        {
            if(vec[i][field] < vec[j][field])
            {
                vc temp = vec[i];
                vec[i] = vec[j];
                vec[j] = temp;
            }
        }
    }
}

static
void
load_q_files(const DwString& dir, const vc& uid, int load_special, vc vec)
{
    DwString pat(dir);
    pat += DIRSEPSTR;
    pat += "*.q";

    FindVec fv(newfn(pat));
    int nn = fv.num_elems();
    int i;
    for(i = 0; i < nn; ++i)
    {
        const WIN32_FIND_DATA& n = *fv[i];
        vc v(VC_VECTOR);
        vc b = load_qb(dir, n.cFileName);

        if(uid.is_nil() || uid == b[QQM_RECIP_VEC][0])
        {
            if(b[QQM_MSG_VEC][QQM_BODY_SPECIAL_TYPE].is_nil() || load_special)
            {
                v[0] = b[QQM_RECIP_VEC][0];
                v[1] = n.cFileName;
                v[2] = b[QQM_MSG_VEC][QQM_BODY_LOGICAL_CLOCK];
                v[3] = b[QQM_MSG_VEC][QQM_BODY_ATTACHMENT];
                v[4] = b[QQM_MSG_VEC][QQM_BODY_SPECIAL_TYPE];
                vec.append(v);
            }
        }
    }

}

int
msg_outq_empty()
{
#ifdef LINUX
    DIR *d = opendir(newfn("outbox").c_str());
    if(!d)
        return 1;
    int outbox_empty;
    if(readdir(d) && readdir(d) && readdir(d))
        outbox_empty = 0;
    else
        outbox_empty = 1;
    closedir(d);
    if(!outbox_empty)
        return 0;

    d = opendir(newfn("inprogress").c_str());
    if(!d)
        return 1;
    int inpro_empty;
    if(readdir(d) && readdir(d) && readdir(d))
        inpro_empty = 0;
    else
        inpro_empty = 1;
    closedir(d);

    return outbox_empty && inpro_empty;
#endif
#ifdef _Windows
    WIN32_FIND_DATA n;

    HANDLE h = FindFirstFile(newfn("outbox\\*.q").c_str(), &n);
    if(h != INVALID_HANDLE_VALUE)
    {
        FindClose(h);
        return 0;
    }
    h = FindFirstFile(newfn("inprogress\\*.q").c_str(), &n);
    if(h != INVALID_HANDLE_VALUE)
    {
        FindClose(h);
        return 0;
    }
    return 1;
#endif
}

// returns 1 if "something is happening" or 0 if the q appears to be empty
// this will allow one special and one non-special to be going at the
// same time

int
qd_send_one()
{
    // note: we can send attachments without a connection
    // to the server, so don't prevent it here.
    if(QSend_inprogress && QSend_special_inprogress)
    {
        return 1;
    }
    vc qd(VC_VECTOR);
    load_q_files("outbox", vcnil, 0, qd);
    int is_special = 0;
    if(qd.num_elems() == 0)
    {
        // selected non-special messages previously and
        // nothing showed up, now try for special messages
        load_q_files("outbox", vcnil, 1, qd);
        if(qd.num_elems() == 0)
            return 0;
        is_special = 1;
    }
    if(!is_special && QSend_inprogress)
        return 1;
    if(is_special && QSend_special_inprogress)
        return 1;
    //sort_on_time(qd, 2);
    GRTLOG("QUEUE", 0, 0);
    GRTLOGVC(qd);

    // just pick whatever comes first
    vc tosend = qd[qd.num_elems() - 1];

    // find random one that has no attachment
    // the other method of sorting might cause one
    // that couldn't be delivered for some reason to
    // block everything else in the q
    DwVec<vc> na;
    for(int i = qd.num_elems() - 1; i >= 0; --i)
    {
        if(qd[i][3].is_nil())
        {
            //tosend = qd[i];
            na.append(qd[i]);
        }
    }
    if(na.num_elems() > 0)
    {
        int r = dwyco_rand() % na.num_elems();
        tosend = na[r];
    }
    DwString a = dwbasename(tosend[1]);
    DwQSend *qs = new DwQSend(a, 0);
    qs->se_sig.connect_ptrfun(se_emit_msg);
    qs->status_sig.connect_ptrfun(se_emit_msg_status);
    qs->se_sig.connect_ptrfun(is_special ? reset_qsend_special : reset_qsend);
    if(qs->send_message() == 1)
    {
        if(!is_special)
            QSend_inprogress = 1;
        else
            QSend_special_inprogress = 1;
    }
    else
        delete qs;
    return 1;
}



// find all outbox and inprogress message id's (*.q)
vc
load_qd_msgs(const vc& uid, int load_special)
{
    vc ret(VC_VECTOR);
    const vc uids = map_uid_to_uids(uid);
    int n = uids.num_elems();
    for(int i = 0; i < n; ++i)
    {
        const vc u = uids[i];
        load_q_files("outbox", u, load_special, ret);
        load_q_files("inprogress", u, load_special, ret);
    }
    sort_on_time(ret, 2);

    return ret;
}

vc
load_qd_to_body(vc qid)
{
    DwString a("outbox" DIRSEPSTR);
    a += (const char *)qid;
    vc qm;
    if(load_info(qm, a.c_str()))
    {
        return direct_to_body2(qm[QQM_MSG_VEC]);
    }
    DwString b("inprogress" DIRSEPSTR);
    b += (const char *)qid;
    if(load_info(qm, b.c_str()))
    {
        return direct_to_body2(qm[QQM_MSG_VEC]);
    }
    return vcnil;
}

void
qd_purge_outbox()
{
    int i;

    recover_inprogress();

    FindVec fv(newfn("outbox" DIRSEPSTR "*.q"));
    int nn = fv.num_elems();
    for(i = 0; i < nn; ++i)
    {
        const WIN32_FIND_DATA& n = *fv[i];
        DwString d("outbox" DIRSEPSTR "");
        d += n.cFileName;
        DeleteFile(newfn(d).c_str());
    }
}

#if 0
int
save_to_inbox(vc m)
{
    // note: the "local id" is set from the server if this msg came from the
    // server. it is set to something locally generated for peer-to-peer messages.
    // this isn't a problem, the main reason we are creating this tag is to filter out
    // stale notifications from google (ie, our app sees and notifies the user of
    // a message, then 20 minutes later google notifies about the same message.)
    // we only get sent google notifications for server messages...
    sql_add_tag(m[QQM_LOCAL_ID], "_inbox");
    return 1;
}
#endif


void
add_ignore(vc uid)
{
    if(Cur_ignore.is_nil())
        Cur_ignore = vc(VC_SET);
    sql_start_transaction();
    vc uids = map_uid_to_uids(uid);
    for(int i = 0; i < uids.num_elems(); ++i)
    {
        Cur_ignore.add(uids[i]);
        sql_add_tag(to_hex(uids[i]), "_ignore");
    }
    sql_commit_transaction();
}

void
del_ignore(vc uid)
{
    if(Cur_ignore.is_nil())
        Cur_ignore = vc(VC_SET);
    sql_start_transaction();
    vc uids = map_uid_to_uids(uid);
    for(int i = 0; i < uids.num_elems(); ++i)
    {
        Cur_ignore.del(uids[i]);
        sql_remove_mid_tag(to_hex(uids[i]), "_ignore");
    }
    sql_commit_transaction();
}

static
int
uid_has_god_power(vc uid)
{
    vc g;
    if(Current_gods.is_nil())
        return 0;
    if(!Current_gods.find(uid, g))
        return 0;
    if(!g[GT_GOD].is_nil())
        return 1;
    // note: demigods and subgods only have per-server god
    // powers. the god tracking stuff is global, so it will report
    // someone as having god powers if they are in another server
    // where they are a demigod or subgod.
    // problem is, we want the ignore to apply only when we are
    // outside the gods' sphere. in other words, if you ignore a
    // mod in a lobby, and you are in that lobby, the ignore will
    // not filter out messages from the god. when you go outside
    // their sphere of control, the ignore works.

    if(!g[GT_DEMIGOD].is_nil() || !g[GT_SUBGOD].is_nil())
    {
        if(g[GT_SERVER_ID] == Current_chat_server_id)
            return 1;
    }
    return 0;
}


// note: this is special case for situation where the dll API
// is querying. we don't want to show the user they have
// been ignored explicitly.
int
is_ignored_id_by_user(vc id)
{
    if(Cur_ignore.is_nil())
        Cur_ignore = vc(VC_SET);
    return Cur_ignore.contains(id);
}


int
uid_ignored(const vc& uid)
{
    if(uid.type() != VC_STRING)
        return 1;
    if(uid == TheMan)
        return 0;
    // if the uid currently has god powers, do not filter
    if(uid_has_god_power(uid))
        return 0;
    return Cur_ignore.contains(uid) ||
           (!uid_has_god_power(My_UID) && Mutual_ignore.contains(uid));
}

vc
get_local_ignore()
{
    const vc res = sql_get_tagged_mids2("_ignore");
    //vc res = map_uid_list_from_tag("_ignore");
    vc ret(VC_SET);
    for(int i = 0; i < res.num_elems(); ++i)
        ret.add(from_hex(res[i][0]));
    return ret;
}

vc
get_local_ignore_mapped()
{
    //vc res = sql_get_tagged_mids2("_ignore");
    vc res = map_uid_list_from_tag("_ignore");
    vc ret(VC_SET);
    for(int i = 0; i < res.num_elems(); ++i)
        ret.add(from_hex(res[i][0]));
    return ret;
}

#if 0
static void
pal_auth_results(vc m, void *t, vc u, ValidPtr)
{
    extern DwycoPalAuthCallback pal_auth_callback;
#ifdef CDC32
    void pal_req_dialog(vc);
#endif
    if(m[1].is_nil())
    {
        // can't tell if they need to be authorized
        // put up a message box or something
        if(pal_auth_callback)
            (*pal_auth_callback)((const char *)u, u.len(), DWYCO_PAL_AUTH_STATUS_FAILED);
        return;

    }
    if(m[3] == vc("auth"))
    {
        // pop up a modeless "you need to
        // request authorization" box
        vc igc = i_grant_cookie(u);
        vc tgc = they_grant_cookie(u);
        if(igc.is_nil() || tgc.is_nil())
        {
            // pop it up, definitely needs updating.
            if(pal_auth_callback)
                (*pal_auth_callback)((const char *)u, u.len(), DWYCO_PAL_AUTH_STATUS_REQUIRES_AUTHORIZATION);

        }
        else
        {
            // already has a cookie, maybe they don't
            // really want to update it right now
            // ask if they want to.
            if(pal_auth_callback)
                (*pal_auth_callback)((const char *)u, u.len(), DWYCO_PAL_AUTH_STATUS_ALREADY_AUTHORIZED);
        }

    }
    else
    {
        // pop up pal added
    }
}
#endif


// unfortunately, i think automatic cleaning like this isn't a good
// idea in the new crdt world. the problem is that this uses heuristics
// based on the locally available information, which we can't be sure
// reflects the entire cluster if it isn't in sync. it would probably be
// better to include some kind of user triggered cleanup, or filtering
// based on time or something like that.
void
power_clean_safe()
{
    return;

}

void
append_forwarded_text(DwString& s, const vc& body)
{
    if(body.type() != VC_VECTOR)
    {
        s += "<<can't display old old message>>\r\n";
        return;
    }
    int cant_resolve;
    const vc from = body[QM_BODY_FROM];
    if(body[QM_BODY_FORWARDED_BODY].is_nil())
    {

        s += "Created by ";
        vc handle;
        handle = uid_to_handle(from, &cant_resolve);
//        if(cant_resolve)
//            handle = " ";
        s += (const char *)handle;
        if(cant_resolve)
        {
            fetch_info(from);
            s += " (";
            s += (const char *)uid_to_short_text(from);
            s += ")";
        }
        s += "\r\n";
        s += (const char *)body[QM_BODY_NEW_TEXT];
        return;
    }
    s += "from ";
    vc handle;
    handle = uid_to_handle(from, &cant_resolve);
//    if(cant_resolve)
//        handle = " ";
    s += (const char *)handle;
    if(cant_resolve)
    {
        fetch_info(from);
        s += " (";
        s += (const char *)uid_to_short_text(from);
        s += ")";
    }
    s += "\r\n";
    s += (const char *)body[QM_BODY_NEW_TEXT];
    s += "\r\n---\r\n";
    append_forwarded_text(s, body[QM_BODY_FORWARDED_BODY]);
}

void
append_forwarded_bodies(vc v, vc body)
{
    // old msg check
    if(body.type() != VC_VECTOR)
        return;
    // end old msg check
    if(body[QM_BODY_FORWARDED_BODY].is_nil())
    {
        v.append(body);
        return;
    }
    v.append(body);
    append_forwarded_bodies(v, body[QM_BODY_FORWARDED_BODY]);
}

vc
get_body_text(vc body)
{
    return body[QM_BODY_NEW_TEXT];
}

vc
strip_port(vc ip)
{
    DwString a((const char *)ip);
    int b = a.find(":");
    if(b == DwString::npos)
        return ip;
    a.remove(b);
    vc ret(a.c_str());
    return ret;
}

static void
remove_all_but(const char *fn, vc nodel)
{
    FindVec fv(newfn(fn));
    int nn = fv.num_elems();
    for(int i = 0; i < nn; ++i)
    {
        const WIN32_FIND_DATA& n = *fv[i];
        if(!nodel.contains(n.cFileName))
        {
            DeleteFile(newfn(n.cFileName).c_str());
        }
    }
}

static
void
find_files_to_keep(DwString subdir, DwString pat, vc nodel)
{
    DwString match(newfn(subdir));

    match += DIRSEPSTR;
    match += pat;
    FindVec fv(match);
    int nn = fv.num_elems();
    for(int i = 0; i < nn; ++i)
    {
        const WIN32_FIND_DATA& n = *fv[i];

        DwString d(newfn(subdir));
        d += DIRSEPSTR;
        d += n.cFileName;
        vc m;
        if(load_info(m, d.c_str(), 1))
        {
            if(!valid_qd_message(m))
            {
                DeleteFile(d.c_str());
                Log_make_entry("deleting bogus item.");
            }
            else
            {
                nodel.add(m[QQM_MSG_VEC][QQM_BODY_ATTACHMENT]);
                // if there is a q-d encrypted package, keep that too
                DwString fn(n.cFileName);
                DwString fne = fn;
                fne += DwString(".enc");
                nodel.add(fne.c_str());

                // load the enc file and add the encrypted attachment too
                vc em;
                if(load_info(em, fne.c_str()))
                {
                    if(!valid_qd_message(em))
                    {
                        DeleteFile(newfn(fn).c_str());
                        Log_make_entry("deleteing bogus encrypted package");
                    }
                    else
                    {
                        nodel.add(em[QQM_MSG_VEC][QQM_BODY_ATTACHMENT]);
                    }
                }
                fne = fn_base_wo_extension(fn);
                fne += DwString(".aux");
                nodel.add(fne.c_str());

            }
        }
    }
}


void
clean_cruft()
{
// this is just a safety measure, because if the cwd
// has changed without us knowing it, we could end up
// causing some damage.
    if(access(newfn("auth").c_str(), 0) != 0)
        return;
// end safety check that won't work if the filename mapper
// is working

    vc nodel(VC_SET);
    find_files_to_keep("inbox", "*.urd", nodel);
    find_files_to_keep("inprogress", "*.q", nodel);
    find_files_to_keep("outbox", "*.q", nodel);

    // now we have a set of things we don't want to remove
    // and we can scan and remove the rest

// XXX NOTE: PUT THIS BACK IN FOR FINAL RELEASE
// useful to keep some of the temp files during testing
#ifndef DWYCO_NO_CLEANUP_ON_EXIT
    remove_all_but("*.dyc", nodel);
    remove_all_but("*.jpg", nodel);
    remove_all_but("*.q", nodel);
    //remove_all_but("*.out", nodel);
    remove_all_but("*.fle", nodel);
    remove_all_but("*.ppm", nodel);

    // if we want to restart sends of encrypted attachments, we have to
    // cache the attachment between runs and not re-encrypt it before
    // each send operation... we want the server and the client to
    // have the same contents. which means keeping both the *.dyc and
    // the *.dyc.enc file
    remove_all_but("*.enc", nodel);
    remove_all_but("*.tmp", nodel);
    remove_all_but("*.aux", nodel);
    DwString d1, user, tmp;
    get_fn_prefixes(d1, user, tmp);
// note: this is a good cleanup, but we can't do it as long as we
// are keeping the remote index databases in tmp.
#if 0
    // this is a bit dangerous, but for this one case where the
    // names are identical except for a "/tmp/" on the end of the
    // tmp pfx, we'll delete all the files in that path
    if(tmp.length() >= 5 && user.length() + 4 == tmp.length() &&
            user == DwString(tmp.c_str(), user.length()) &&
            tmp.rfind(DIRSEPSTR "tmp" DIRSEPSTR) == tmp.length() - 5)
    {
        DwString tp = tmp;
        tp += "*.*";
        FindVec fv(tp);
        int nn = fv.num_elems();
        for(int i = 0; i < nn; ++i)
        {
            const WIN32_FIND_DATA& n = *fv[i];
            DwString tfn = tmp;
            tfn += n.cFileName;
            DeleteFile(tfn.c_str());

        }
    }
#endif
#endif

}

void
weekly_trash_empty()
{
    // just empty the trash once a week, this is mainly for debugging
    // these days anyway, since we don't really offer a way for users
    // to untrash this atm.
    vc last_empty;
    if(!load_info(last_empty, "trs.dif") ||
            (time(0) - (time_t)last_empty) > ((time_t)7 * 24 * 3600))
    {
        empty_trash();
        last_empty = time(0);
        save_info(last_empty, "trs.dif");
    }
}

#if 0
void
they_grant_add(vc who, vc cookie)
{
    They_grant.add_kv(who, cookie);
    save_info(They_grant, "theygrnt");
    pal_relogin();
}

void
they_grant_del(vc who)
{
    They_grant.del(who);
    save_info(They_grant, "theygrnt");
    pal_relogin();
}

vc
they_grant_cookie(vc who)
{
    vc c;
    if(They_grant.find(who, c))
        return c;
    return vcnil;
}

void
i_grant_add(vc who, vc cookie)
{
    I_grant.add_kv(who, cookie);
    save_info(I_grant, "igrant");
    pal_relogin();
}

void
i_grant_del(vc who)
{
    I_grant.del(who);
    save_info(I_grant, "igrant");
    pal_relogin();
}

vc
i_grant_cookie(vc who)
{
    vc c;
    if(I_grant.find(who, c))
        return c;
    return vcnil;
}

void
reset_i_grant()
{
    vc v(VC_TREE);
    v.add_kv("me", My_UID);
    save_info(v, "igrant");
    I_grant = v;
    pal_relogin();
}

void
reset_they_grant()
{
    vc v(VC_TREE);
    v.add_kv("me", My_UID);
    save_info(v, "theygrnt");
    They_grant = v;
    pal_relogin();
}
#endif

int
pal_add(vc u)
{
    //dirth_send_get_pal_auth_state(My_UID, u, QckDone(pal_auth_results, 0, u));
    int ret = 0;
    sql_start_transaction();
    vc uids = map_uid_to_uids(u);
    for(int i = 0; i < uids.num_elems(); ++i)
    {
        vc uid = uids[i];
        if(!Pals.contains(uid))
        {
            Pals.add(uid);
            sql_add_tag(to_hex(uid), "_pal");
            ret = 1;
        }
    }
    sql_commit_transaction();
    if(ret)
        pal_relogin();
    return ret;
}

int
pal_del(vc u, int norelogin)
{
    sql_start_transaction();
    vc uids = map_uid_to_uids(u);
    int updated = 0;
    for(int i = 0; i < uids.num_elems(); ++i)
    {
        vc uid = uids[i];
        if(Pals.contains(uid))
        {
            Pals.del(uid);
            //i_grant_del(u);
            //they_grant_del(u);
            sql_remove_mid_tag(to_hex(uid), "_pal");
            updated = 1;
        }
    }
    sql_commit_transaction();
    if(!norelogin && updated)
    {
        pal_relogin();
    }
    return 1;
}

int
pal_user(vc u)
{
    return Pals.contains(u);
}

int
refile_attachment(vc filename, vc from_user)
{
    // move the attachment into the directory for that
    // remote user.
    if(from_user.len() == 0)
        return 0;
    DwString s((const char *)to_hex(from_user));
    s += ".usr";
    DwString s2 = s;
    s2 += DIRSEPSTR;
    s2 += (const char *)filename;
    // note: here we replace existing because we don't
    // want an error if the file comes via two different
    // paths (as can happen when a direct send works, but
    // success is not reported in time before the file
    // then sent via the server, resulting in two messages.
    //DeleteFile(newfn(s2).c_str());
    if(!move_replace(newfn((const char *)filename), newfn(s2)))
    {
        // hmmm, something is wrong, but make the dir anyway.
        init_msg_folder(from_user);
        // try again...
        if(!move_replace(newfn((const char *)filename), newfn(s2)))
            return 0;
    }

    return 1;
}

void
ignoring_you_update(vc m, void *, vc, ValidPtr)
{
    // msg is:
    // vector(vector(iy 0) vector(a|d uid))
    // where a = add uid to mutual ignore list,
    // and d = remove uid from mutual ignore list

    if(m[1].type() != VC_VECTOR)
        return;
    if(m[1][0] == vc("a"))
    {
        vc uids = map_uid_to_uids(m[1][1]);
        for(int i = 0; i < uids.num_elems(); ++i)
        {
            Mutual_ignore.add(uids[i]);
        }
    }
    else if(m[1][0] == vc("d"))
    {
        vc uids = map_uid_to_uids(m[1][1]);
        for(int i = 0; i < uids.num_elems(); ++i)
        {
            Mutual_ignore.del(uids[i]);
        }
    }
    // this is probably best left silent
    //Refresh_users = 1;
}





