
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
#include "vccomp.h"
#include "dwstr.h"
#include "qauth.h"
#include "dirth.h"
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
#include "zapadv.h"
#include "ta.h"
#include "usercnfg.h"
#include "cdcver.h"
#include "files.h"
#include "sha.h"
#include "aes.h"
#include "gcm.h"

using namespace CryptoPP;
#include "filters.h"

#include "netvid.h"
#include "dwrtlog.h"
#include "dwstr.h"
#include "prfcache.h"
#include "dumbass.h"
#include "fnmod.h"
#ifdef _Windows
#define strcasecmp strcmpi
#endif
#include "dwtree2.h"
#include "se.h"
#include "chatq.h"
#include "pgdll.h"
#include "sepstr.h"
#include "lanmap.h"
#include "ser.h"
#include "xinfo.h"
#include "vcudh.h"
#include "pkcache.h"
#include "dhsetup.h"
#include "qsend.h"
#include "ssns.h"
#include "dwyco_rand.h"
#include "qmsgsql.h"
using namespace dwyco;

vc MsgFolders;

int Rescan_msgs;

// list of message summaries both from server and direct
static vc Cur_msgs;

vc Cur_ignore;
vc Session_ignore;
// mutual is separated out because we want to have a
// part of the ignore list that cannot be maniplated
// via the api
vc Mutual_ignore;


vc Online;
vc Client_types;
vc Online_noise;
//vc Never_visible;
//vc Always_visible;
//vc I_grant;
//vc They_grant;
vc No_direct_msgs;
vc No_direct_att;
//int Pal_auth_warn;
vc Pals;
vc Client_ports;
vc LANmap;
int LANmap_inhibit;
vc Chat_ips;
vc Chat_ports;
vc Session_infos;
static vc In_progress;
static long Logical_clock;
// this is used in order to assign clock values to
// messages when we first see them from the server.
static vc Mid_to_logical_clock;
extern vc Current_chat_server_id;

void pal_relogin();
void new_pipeline();
int save_msg(vc m, vc msg_id);

#include "qmsgsql.h"

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

static int
add_msg_folder(vc uid)
{
    if(MsgFolders.contains(uid))
        return 0;
    MsgFolders.add_kv(uid, vcnil);
    se_emit(SE_USER_ADD, uid);
    return 1;
}

// note: this function is also used to transmit pal list to
// chat server, so don't update it unless you have to.
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

int
uid_online_display(vc uid)
{
    if(Online.contains(uid))
        return 1;
    return 0;
}

int
uid_online(vc uid)
{
    if(Online.contains(uid))
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
    // prefer the LAN ip and ports over the remote ones
    if(!LANmap_inhibit)
    {
        if(LANmap.find(uid, u))
        {
            can_do_direct = 1;
            DwString a((const char *)u[LANMAP_LOCAL_IP]);
            int c = a.find(":");
            if(c != DwString::npos)
                a.erase(c);
            if(u.type() == VC_VECTOR)
            {
                prim = (int)u[LANMAP_LOCAL_PORTS][0];
                sec = (int)u[LANMAP_LOCAL_PORTS][1];
                pal = (int)u[LANMAP_LOCAL_PORTS][2];
            }
            GRTLOGA("uid to ip LANMAP %s %d %d %d", a.c_str(), prim, sec, pal, 0);
            return inet_addr(a.c_str());
        }
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
        GRTLOGA("uid to ip ONLINE %s %d %d %d", a.c_str(), prim, sec, pal, 0);
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
        GRTLOGA("uid to ip ONLINE/CHAT %s %d %d %d", a.c_str(), prim, sec, pal, 0);
        return inet_addr(a.c_str());
    }

    return 0;
}

static int Invis;
int
is_invisible()
{
#if 0
    //no more invis
    return 0;
#endif
    return Invis;
}

void
set_invisible(int i)
{
#if 0
    //no more invis
    return 0;
#endif
    Invis = !!i;
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
    Cur_ignore = get_local_ignore();
    Session_ignore = vc(VC_SET);
    Mutual_ignore = vc(VC_SET);
    Online = vc(VC_TREE);
    Client_types = vc(VC_TREE);
    No_direct_msgs = vc(VC_SET);
    No_direct_att = vc(VC_SET);
    MsgFolders = vc(VC_TREE);
    In_progress = vc(VC_TREE);
    // not perfect, but better than scanning for a max value
    // somewhere.
    Logical_clock = time(0);

    if(!load_info(Online_noise, "noise"))
    {
        Online_noise = vc(VC_SET);
        save_info(Online_noise, "noise");
    }
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
    Client_ports = vc(VC_TREE);
    LANmap = vc(VC_TREE);
    if(!load_info(Session_infos, "sinfo") || Session_infos.type() != VC_MAP)
    {
        Session_infos = vc(VC_MAP);
        save_info(Session_infos, "sinfo");
    }
    Chat_ips = vc(VC_TREE);
    Chat_ports = vc(VC_TREE);

    init_qmsg_sql();
    long tmplc = sql_get_max_logical_clock();
    if(tmplc > Logical_clock)
        Logical_clock = tmplc + 1;

    new_pipeline();

    Mid_to_logical_clock = vc(VC_TREE);
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
    Cur_msgs = vc(VC_VECTOR);
    Cur_ignore = get_local_ignore();
    Session_ignore = vc(VC_SET);
    Mutual_ignore = vc(VC_SET);
    Online = vc(VC_TREE);
    Client_types = vc(VC_TREE);
    No_direct_msgs = vc(VC_SET);
    No_direct_att = vc(VC_SET);
    MsgFolders = vc(VC_TREE);
    In_progress = vc(VC_TREE);
    
    // not perfect, but better than scanning for a max value
    // somewhere.
    Logical_clock = time(0);

    if(!load_info(Online_noise, "noise"))
    {
        Online_noise = vc(VC_SET);
        save_info(Online_noise, "noise");
    }
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
    Client_ports = vc(VC_TREE);
    LANmap = vc(VC_TREE);
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
    //init_fav_sql();

    //new_pipeline();

    //Mid_to_logical_clock = vc(VC_TREE);
}


#if 0
static int
should_auto_reply(vc msg)
{
    if(Session_auto_replies.contains(msg[QM_FROM]))
        return 0;
    return 1;
}

static int
gen_auto_reply(vc msg)
{
    if(!ZapAdvData.get_send_auto_reply() || !should_auto_reply(msg))
        return 1;
    int perform_auto_reply(vc recip_uid);
    int ret = perform_auto_reply(msg[QM_FROM]);
    Session_auto_replies.add(msg[QM_FROM]);
    return ret;
}
#endif

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
valid_qd_message(vc v)
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
    SHA sha;
    HashFilter *hf = new HashFilter(sha);
    FileSource fs(filename.c_str(), TRUE, hf);
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

void
gen_authentication(vc qmsg, vc att_hash)
{
    // put the auth vector into a q'd message
    vc av(VC_VECTOR);
    vc mvec = qmsg[QQM_MSG_VEC];

    SHA sha;
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
verify_authentication(vc text, vc uid, vc att_hash, vc datevec, vc no_forward, vc mac)
{
    SHA sha;
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
verify_chain(vc body, int top, vc att_hash, vc attachment_dir)
{
    GRTLOG("verify chain %d", top, 0);
    GRTLOGVC(body);
    GRTLOGVC(att_hash);
    GRTLOGVC(attachment_dir);
    vc from = body[QM_BODY_FROM];
    vc text = body[QM_BODY_NEW_TEXT];
    vc attachment = body[QM_BODY_ATTACHMENT];
    vc authvec = body[QM_BODY_AUTH_VEC];
    vc new_text = body[QM_BODY_NEW_TEXT];
    vc forwarded_body = body[QM_BODY_FORWARDED_BODY];
    vc datevec = body[QM_BODY_DATE];
    vc no_forward = body[QM_BODY_NO_FORWARD];

    int res = 0;
    if(authvec.is_nil())
    {
        res = VERF_AUTH_NO_INFO;
        return res;
    }
    vc av_no_forward = authvec[QMBA_NO_FORWARD];
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

typedef DwVecP<WIN32_FIND_DATA> FindVec;

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
uid_to_dir(vc uid)
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
    int do_fetch = 0;
    if(mkdir(s.c_str()) == 0)
    {
        do_fetch = 1;
        add_msg_folder(uid);
    }
    if(do_fetch || !Session_infos.contains(uid))
    {
        fetch_info(uid);
    }
    return 1;
}

int
init_msg_folder(vc uid)
{
    return init_msg_folder(uid, 0);
}

FindVec *
find_to_vec(const char *pat)
{
    int i = 1;

    FindVec& ret = *new FindVec;
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
        return &ret;
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

    return &ret;
}

void
delete_findvec(FindVec *fv)
{
    int n = fv->num_elems();
    for(int i = 0; i < n; ++i)
        delete (*fv)[i];
    delete fv;
}

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
fetch_attachment(vc id, DestroyCallback dc, vc dcb_arg1, void *dcb_arg2, ValidPtr dcb_arg3,
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
        return 0;
    }
    mc->remote_filename = id;
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
fetch_pk_done(vc m, void *, vc uid, ValidPtr)
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
    put_pk(uid, static_public, sig);
    pk_set_session_cache(uid);
    TRACK_ADD(QM_fetch_pk_ok, 1);

}



static void
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

    static vc invalid("invalid");

    vc v;
    // special case: if issuing this command results in a local
    // error like a timeout, we want to allow the client to attempt
    // the fetch again. this is a little dangerous because we could
    // end up fetching forever, but since it is a local error, the
    // server probably is never seeing it anyway.
    // in the past, we provided "alternate info" we could use if the
    // server failed, and this was derived from secondary like the
    // directory. but if those sources are not available that
    // bogus info would get in and never cleaned out.
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
        vc ai = make_best_local_info(uid, 0);
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
            vc ai = make_best_local_info(uid, 0);
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
            vc ai = make_best_local_info(uid, 0);
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
            vc ai = make_best_local_info(uid, 0);
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
        vc user_id = v[0];

        Session_infos.add_kv(user_id, v);
    }
}

static
vc
make_alt_info(vc name, vc desc, vc location)
{
    vc r(VC_VECTOR);
    r.append(name);
    r.append(desc);
    r.append(location);
    return r;
}

vc
make_best_local_info(vc uid, int *cant_resolve_now)
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
            return make_alt_info(pack[vc("handle")], pack[vc("desc")], pack[vc("loc")]);
        }
    }
    // this is an indicator that the profile isn't available in the main cache, and should
    // be fetched from the server
    if(cant_resolve_now)
        *cant_resolve_now = 1;
    if(uid == My_UID)
    {
        return make_alt_info(UserConfigData.get_username(), UserConfigData.get_description(), UserConfigData.get_location());

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
fetch_info(vc uid)
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
          vc forwarded_body, vc new_text, vc no_forward, vc user_filename, vc logical_clock)
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

    return v;
}

vc
direct_to_body(vc msgid)
{
    vc huid = sql_get_uid_from_mid(msgid);
    if(huid.is_nil())
        return vcnil;
    vc uid = from_hex(huid);
    return load_body_by_id(uid, msgid);

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
    // effect, the list of unsaved messages will be presented to
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

void
del_msg(vc vec, vc msg_id)
{
    int n = vec.num_elems();
    int i;
    for(i = 0; i < n; ++i)
    {
        if(vec[i][QM_ID] == msg_id)
        {
            vec.remove(i, 1);
            //Refresh_users = 1;
            return;
        }
    }
}

static vc
find_msg(vc vec, vc msg_id)
{
    int n = vec.num_elems();
    int i;
    for(i = 0; i < n; ++i)
    {
        if(vec[i][QM_ID] == msg_id)
        {
            return vec[i];
        }
    }
    return vcnil;
}

vc
find_cur_msg(vc msg_id)
{
    return find_msg(Cur_msgs, msg_id);
}

static void
query_done(vc m, void *, vc, ValidPtr)
{
    if(m[1].is_nil())
        return;

    vc v2 = m[1];

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
            v2.remove(i, 1);
            --i;
            continue;
        }
        init_msg_folder(from);

        // this logical clock stuff corresponds to "receiving" the message
        // the first time we see the message summary from the server.
        // this is to avoid a situation where we are assigning logical clocks
        // when the message is finished fetching, which isn't quite right (could
        // happen in random order, which looks goofy when we re-order by lc)
        // this is a hack, for sure... really need to have some consistent way of
        // assiging lc's for each path thru the system. maybe the server even
        // needs to have its own lc
        vc mid = v[QM_ID];
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
        add_msg(Cur_msgs, v);
        sql_add_tag(mid, "_remote");
        DwString a("_");
        a += (const char *)to_hex(from);
        sql_add_tag(mid, a.c_str());
    }
    sql_commit_transaction();
    }
    catch(...)
    {
        Cur_msgs = vc(VC_VECTOR);
        sql_rollback_transaction();
    }

    Rescan_msgs = 1;
}

void
query_messages()
{
    dirth_send_query(My_UID, QckDone(query_done, 0));
}

//static int Hack_first_load = 0;

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
        No_direct_msgs.del(from);
    }

    Rescan_msgs = 1;
    // save_msg indexes the message too, so after store_direct
    // you can map mid to uid and so on.
    try
    {
        sql_start_transaction();
        if(save_msg(msg, id))
        {

            sql_add_tag(id, "_inbox");
            sql_add_tag(id, "_local");
            sql_remove_mid_tag(id, "_remote");
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


void
load_users(int only_recent, int *total_out)
{
    DwString s;

    MsgFolders = vc(VC_TREE);

    if(only_recent)
    {
        vc ret = sql_get_recent_users(total_out);
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
            return load_users(0, total_out);
        }
        int n = ret.num_elems();
        TRACK_MAX(QM_UL_recent_count, n);
        for(int i = 0; i < n; ++i)
        {
            vc uid = from_hex(ret[i]);
            MsgFolders.add_kv(uid, vcnil);
        }
    }
    else
    {

        FindVec &fv = *find_to_vec(newfn("*.usr").c_str());
        int n = fv.num_elems();
        TRACK_MAX(QM_UL_count, n);
        for(int i = 0; i < n; ++i)
        {
            WIN32_FIND_DATA &d = *fv[i];
            s = d.cFileName;
            vc uid = dir_to_uid(s);
            if(uid.len() != 10)
                continue;
            MsgFolders.add_kv(uid, vcnil);
        }

        delete_findvec(&fv);
        if(total_out)
            *total_out = MsgFolders.num_elems();
    }
}


vc
uid_to_short_text(vc id)
{
    vc u = to_hex(id);
    DwString chop((const char *)u);
    chop.remove(8);
    DwString ret;
    ret += "#";
    ret += chop;
    return ret.c_str();
}

vc
uid_to_handle(vc id, int *cant_resolve_now)
{
    vc ai = make_best_local_info(id, cant_resolve_now);
    return ai[0];
}

vc
uid_to_handle2(vc id)
{
    vc ai = make_best_local_info(id, 0);
    return ai[0];
}

vc
uid_to_location(vc id, int *cant_resolve_now)
{
    vc ai = make_best_local_info(id, cant_resolve_now);
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
        delete_msg2(ackset[i]);
        sql_remove_mid_tag(ackset[i], "_remote");
    }
    DwString tag("_");
    tag += (const char *)to_hex(uid);
    vc mids = sql_get_tagged_mids2(tag.c_str());
    for(int i = 0; i < mids.num_elems(); ++i)
    {
        sql_fav_remove_mid(mids[i][0]);
    }
    sql_commit_transaction();

}

int
remove_user_files(vc id, const char *pfx, int keep_folder)
{
    int i;
    if(id.len() == 0)
        return 0;
    DwString s((const char *)id);

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
    FindVec& fv = *find_to_vec(s.c_str());
    int n = fv.num_elems();

    for(i = 0; i < n; ++i)
    {
        WIN32_FIND_DATA& d = *fv[i];
        if(strcmp(d.cFileName, ".") == 0 ||
                strcmp(d.cFileName, "..") == 0)
            continue;
        if(god && strcmp(d.cFileName, "info") == 0)
            continue;
        DwString s2((const char *)id);
        DwString p2 = pfx;
        p2 += s2;
        p2 = newfn(p2);
        p2 += "" DIRSEPSTR "";
        p2 += d.cFileName;
        if(!DeleteFile(p2.c_str()))
            retval = 0;
    }
    p = pfx;
    p += (const char *)id;
    if(!god &&  (!keep_folder && !RemoveDirectory(newfn(p).c_str())))
        retval = 0;
    delete_findvec(&fv);

    // note: don't remove never vis attr, as it is
    // possible someone wants to remain invisible to
    // this person even after removing them, likewise
    // with the ignore attribute.
    // NOTE: the CALLER must save the infos file, since this
    // is called to empty the trash can, and we don't want that
    // to modify the infos files
    return 1; // tired of hearing tech support about not being able to
    // remove things, not sure why this happens (maybe they
    // have a video open when they try to remove it or something.)
    //return retval;
}

int
remove_user(vc id, const char *pfx)
{
    remove_user_files(id, pfx, 0);
    // note: this removes pal auth stuff too
    vc uid = dir_to_uid((const char *)id);
    //always_vis_del(uid);
    // remove indexs so the msgs don't magically reappear
    sql_start_transaction();
    sql_fav_remove_uid(uid);
    remove_msg_idx_uid(uid);
    DwString tag("_");
    tag += (const char *)to_hex(uid);
    vc mids = sql_get_tagged_mids2(tag.c_str());
    for(int i = 0; i < mids.num_elems(); ++i)
    {
        sql_fav_remove_mid(mids[i][0]);
    }
    sql_commit_transaction();

    MsgFolders.del(uid);
    se_emit(SE_USER_REMOVE, uid);
    return 1;
}

int
clear_user(vc id, const char *pfx)
{
    remove_user_files(id, pfx, 1);
    vc uid = dir_to_uid((const char *)id);
    sql_start_transaction();
    sql_fav_remove_uid(uid);
    clear_msg_idx_uid(uid);
    DwString tag("_");
    tag += (const char *)to_hex(uid);
    vc mids = sql_get_tagged_mids2(tag.c_str());
    for(int i = 0; i < mids.num_elems(); ++i)
    {
        sql_fav_remove_mid(mids[i][0]);
    }
    sql_commit_transaction();
    Rescan_msgs = 1;
    return 1;
}

int
trash_user(vc id)
{
    if(id.len() == 0)
        return 0;
    int i;
    DwString s((const char *)id);

    if(s.length() == 0)
        return 0;
    if(s.rfind(".usr") != s.length() - 4)
        return 0;
    DwString s2 = s;
    s2.remove(s2.length() - 4);
    if(s2.find_first_not_of("0123456789abcdefABCDEF") != DwString::npos)
        return 0;
    vc uid = from_hex(s2.c_str());

    int god = (uid == TheMan);
    if(god)
        return 0;

    DwString trashdir = newfn("trash");
    trashdir += DIRSEPSTR;
    trashdir += s;
    mkdir(trashdir.c_str());
    s = newfn(s);
    s += "" DIRSEPSTR "*.*";

    int retval = 1;
    FindVec& fv = *find_to_vec(s.c_str());
    int n = fv.num_elems();

    for(i = 0; i < n; ++i)
    {
        WIN32_FIND_DATA& d = *fv[i];
        if(strcmp(d.cFileName, ".") == 0 ||
                strcmp(d.cFileName, "..") == 0)
            continue;
        DwString s2((const char *)id);
        s2 = newfn(s2);
        s2 += "" DIRSEPSTR "";
        s2 += d.cFileName;
        DwString trashfile = trashdir;
        trashfile += "" DIRSEPSTR "";
        trashfile += d.cFileName;
        //DeleteFile(newfn(trashfile).c_str());
        if(!move_replace(s2, trashfile))
            retval = 0;
    }
    if(!RemoveDirectory(newfn((const char *)id).c_str()))
        retval = 0;
    delete_findvec(&fv);
    // note: when you trash a user, it was nice to leave the
    // aatributes alone, then an untrash would retrieve the
    // attributes as well. but that led to problems where
    // trashed users attributes would be sent to the pal server
    // and stuff. what we really need is a new trash with the
    // attributes in it. but for now, we just kill the attributes
    // so when you untrash, you get the messages, but not the
    // attributes back. there are other problems with users
    // being in the trash, then reappearing on the pal list
    // thru normal operation. fix those later.
    // note: this removes pal auth stuff too
    //always_vis_del(uid);
    // note: don't remove never vis attr, as it is
    // possible someone wants to remain invisible to
    // this person even after removing them, likewise
    // with the ignore attribute.
    //infos_del(uid);
    MsgFolders.del(uid);
    remove_msg_idx_uid(uid);
    se_emit(SE_USER_REMOVE, uid);
    //Refresh_users = 1;
    return retval;
}

void
untrash_users()
{
    int i;

    DwString trashdir = newfn("trash");
    trashdir += DIRSEPSTR;
    DwString s = trashdir;
    s += "*.usr";

    FindVec& fv = *find_to_vec(s.c_str());
    int n = fv.num_elems();

    for(i = 0; i < n; ++i)
    {
        WIN32_FIND_DATA& d = *fv[i];
        if(strcmp(d.cFileName, ".") == 0 ||
                strcmp(d.cFileName, "..") == 0)
            continue;
        // make target dir
        //mkdir(newfn(d.cFileName).c_str());
        vc uid = dir_to_uid(d.cFileName);
        init_msg_folder(uid);

        // now go thru all the files in the
        // trash dir and move them back
        DwString s2 = trashdir;
        s2 += d.cFileName;
        DwString dir = s2;

        s2 += "" DIRSEPSTR "*.*";
        FindVec& fv2 = *find_to_vec(s2.c_str());
        int n2 = fv2.num_elems();
        for(int j = 0; j < n2; ++j)
        {
            WIN32_FIND_DATA& d2 = *fv2[j];
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
        delete_findvec(&fv2);
        clear_indexed_flag(uid);
        load_msg_index(uid, 1);
    }
    delete_findvec(&fv);
}

vc
load_msgs(vc uid)
{
    vc ret(VC_VECTOR);
    if(!uid.is_nil())
    {
        for(int i = 0; i < Cur_msgs.num_elems(); ++i)
        {
            if(Cur_msgs[i][QM_FROM] == uid)
                ret.append(Cur_msgs[i]);
        }
    }
    else
    {
        for(int i = 0; i < Cur_msgs.num_elems(); ++i)
        {
            ret.append(Cur_msgs[i]);
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
load_bodies(vc id, int load_sent)
{
    int i = 1;
    DwString s((const char *)id);
    vc ret(VC_VECTOR);

    DwTreeKaz<int, GroovyItem> sorter(0);

    DwString ss = s;
    s = newfn(s);
    DwString dpref = s;
    s += "" DIRSEPSTR "*.bod";
    int t = 0;

    FindVec& fv = *find_to_vec(s.c_str());
    int n = fv.num_elems();
    for(i = 0; i < n; ++i)
    {
        WIN32_FIND_DATA &d = *fv[i];
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
    delete_findvec(&fv);

    if(load_sent)
    {
        ss = newfn(ss);
        ss += "" DIRSEPSTR "*.snt";
        s = ss;
        FindVec& fv2 = *find_to_vec(s.c_str());
        n = fv2.num_elems();
        for(i = 0; i < n; ++i)
        {
            WIN32_FIND_DATA &d = *fv2[i];
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
        delete_findvec(&fv2);
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


vc
load_body_by_id(vc user_id, vc msg_id)
{
    if(user_id.len() == 0)
        return vcnil;
    DwString s = (const char *)to_hex(user_id);
    s += ".usr";
    return load_body(s.c_str(), msg_id);
}

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
void
delete_msg2(vc msg_id)
{
    del_msg(Cur_msgs, msg_id);
    Rescan_msgs = 1;
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
delete_body3(vc user_id, vc msg_id, int inhibit_indexing)
{
    if(user_id.len() == 0)
        return;
    DwString s((const char *)to_hex(user_id));
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
        if(!msg[QM_BODY_ATTACHMENT].is_nil())
            delete_attachment2(user_id, msg[QM_BODY_ATTACHMENT]);
        DeleteFile(s.c_str());
        if(!inhibit_indexing)
        {
            remove_msg_idx(user_id, msg_id);
            sql_fav_remove_mid(msg_id);
        }
        return;
    }
    s2 += ".snt";
    if(load_info(msg, s2.c_str(), 1))
    {
        if(!msg[QM_BODY_ATTACHMENT].is_nil())
            delete_attachment2(user_id, msg[QM_BODY_ATTACHMENT]);
        DeleteFile(s2.c_str());
        if(!inhibit_indexing)
        {
            remove_msg_idx(user_id, msg_id);
            sql_fav_remove_mid(msg_id);
        }
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
    m[QQM_BODY_NO_FORWARD] = no_forward;
    // inhibit server-based delivery reporting for now
    m[QQM_BODY_NO_DELIVERY_REPORT] = vctrue;
    gen_authentication(qmsg, att_hash);
    // note: get rid of no-forward encoding hackery.
    // means old software will not be able to view
    // no-forward msgs sent from new software.
    msg_out = qmsg;

    return 1;
}

// msg should be in QQM format
vc
decrypt_msg_qqm(vc emsg)
{
    GRTLOG("dec msg qqm", 0, 0);
    GRTLOGVC(emsg);

    vc body = emsg[QQM_MSG_VEC];

    vc key = dh_store_and_forward_get_key(body[QQM_BODY_DHSF], dh_my_static());
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

vc
decrypt_msg_body(vc body)
{
    GRTLOG("dec msg body", 0, 0);
    GRTLOGVC(body);

    vc key = dh_store_and_forward_get_key(body[QQM_BODY_DHSF], dh_my_static());
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

// decrypt everything but the attachment
// this is used to send large attachment decryption to another thread
// so we don't block on it here
vc
decrypt_msg_body2(vc body, DwString& src, DwString& dst, DwString& key_out)
{
    GRTLOG("dec msg body", 0, 0);
    GRTLOGVC(body);

    vc key = dh_store_and_forward_get_key(body[QQM_BODY_DHSF], dh_my_static());
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

int
decrypt_attachment2(const DwString& filename, const DwString& key, const DwString& filename_dst)
{
    vc f(VC_BSTRING, filename.c_str(), filename.length());
    vc k(VC_BSTRING, key.c_str(), key.length());
    vc d(VC_BSTRING, filename_dst.c_str(), filename_dst.length());
    return decrypt_attachment(f, k, d);
}


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
            update_msg_idx(recip[i], m[1]);
            sql_add_tag(m[1][QM_BODY_ID], "_local");
            sql_add_tag(m[1][QM_BODY_ID], "_sent");
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

    FindVec& fv = *find_to_vec(newfn("inprogress" DIRSEPSTR "*.q").c_str());
    int nn = fv.num_elems();
    for(i = 0; i < nn; ++i)
    {
        WIN32_FIND_DATA& n = *fv[i];
        DwString s("inprogress" DIRSEPSTR "");
        DwString d("outbox" DIRSEPSTR "");
        s += n.cFileName;
        d += n.cFileName;
        move_replace(newfn(s), newfn(d));
    }
    delete_findvec(&fv);
}


static
void
reset_qsend(enum dwyco_sys_event cmd, DwString qid, vc recip_uid)
{
    if(cmd == SE_MSG_SEND_START)
        return;
    QSend_inprogress = 0;
}

static
void
reset_qsend_special(enum dwyco_sys_event cmd, DwString qid, vc recip_uid)
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
load_q_files(const DwString& dir, vc uid, int load_special, vc vec)
{
    DwString pat(dir);
    pat += DIRSEPSTR;
    pat += "*.q";

    FindVec& fv = *find_to_vec(newfn(pat).c_str());
    int nn = fv.num_elems();
    int i;
    for(i = 0; i < nn; ++i)
    {
        WIN32_FIND_DATA& n = *fv[i];
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
    delete_findvec(&fv);

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
load_qd_msgs(vc uid, int load_special)
{
    vc ret(VC_VECTOR);

    load_q_files("outbox", uid, load_special, ret);
    load_q_files("inprogress", uid, load_special, ret);
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

    FindVec& fv = *find_to_vec(newfn("outbox" DIRSEPSTR "*.q").c_str());
    int nn = fv.num_elems();
    for(i = 0; i < nn; ++i)
    {
        WIN32_FIND_DATA& n = *fv[i];
        DwString d("outbox" DIRSEPSTR "");
        d += n.cFileName;
        DeleteFile(newfn(d).c_str());
    }
    delete_findvec(&fv);
}


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
    sql_add_tag(m[QQM_LOCAL_ID], "_local");
    return 1;
}





void
got_ignore(vc m, void *, vc, ValidPtr)
{
    if(m[1].is_nil())
    {
        Cur_ignore = vc(VC_SET);
    }
    else
        Cur_ignore = m[1];
    //Refresh_users = 1;
}

void
add_ignore(vc id)
{
    if(Cur_ignore.is_nil())
        Cur_ignore = vc(VC_SET);
    Cur_ignore.add(id);
    Session_ignore.add(id);
}

void
del_ignore(vc id)
{
    if(Cur_ignore.is_nil())
        Cur_ignore = vc(VC_SET);
    Cur_ignore.del(id);
    Session_ignore.del(id);
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
    return Cur_ignore.contains(id) || Session_ignore.contains(id);
}


int
uid_ignored(vc uid)
{
    if(uid.type() != VC_STRING)
        return 1;
    if(uid == TheMan)
        return 0;
    // if the uid currently has god powers, do not filter
    if(uid_has_god_power(uid))
        return 0;
    return Cur_ignore.contains(uid) || Session_ignore.contains(uid) ||
           (!uid_has_god_power(My_UID) && Mutual_ignore.contains(uid));
}

void
clear_local_ignore()
{
    DeleteFile(newfn("ignore").c_str());
}
void
add_local_ignore(vc uid)
{
    vc ign;
    if(!load_info(ign, "ignore"))
        ign = vc(VC_SET);
    ign.add(uid);
    save_info(ign, "ignore");
}
void
del_local_ignore(vc uid)
{
    vc ign;
    if(!load_info(ign, "ignore"))
        ign = vc(VC_SET);
    ign.del(uid);
    save_info(ign, "ignore");
}

vc
get_local_ignore()
{
    vc ign;
    if(!load_info(ign, "ignore"))
        ign = vc(VC_SET);
    return ign;
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


void
power_clean_safe()
{
    sql_index_all();
    int n;
    vc uids = sql_get_empty_users();

    if(!uids.is_nil())
    {
        n = uids.num_elems();
        for(int i = 0; i < n; ++i)
        {
            vc uid = from_hex(uids[i]);
            if(uid == My_UID)
                continue;
            if(pal_user(uid))
                continue;
            trash_user(uid_to_dir(uid));
        }
    }

    uids = sql_get_no_response_users();

    if(!uids.is_nil())
    {
        n = uids.num_elems();
        for(int i = 0; i < n; ++i)
        {
            vc uid = from_hex(uids[i]);
            if(uid == My_UID)
                continue;
            if(pal_user(uid))
                continue;
            if(sql_fav_has_fav(uid))
                continue;
            trash_user(uid_to_dir(uid));
        }
    }

    uids = sql_get_old_ignored_users();
    if(uids.is_nil())
        return;
    n = uids.num_elems();
    for(int i = 0; i < n; ++i)
    {
        vc uid = from_hex(uids[i]);
        if(uid == My_UID)
            continue;
        if(pal_user(uid))
            continue;
        if(sql_fav_has_fav(uid))
            continue;
        trash_user(uid_to_dir(uid));
    }

}

void
online_noise_add(vc u)
{
    Online_noise.add(u);
    save_info(Online_noise, "noise");
}
void
online_noise_del(vc u)
{
    Online_noise.del(u);
    save_info(Online_noise, "noise");
}

int
online_noise(vc u)
{
    return Online_noise.contains(u);
}

int
count_trashed_users()
{
    FindVec &fv = *find_to_vec(newfn("trash" DIRSEPSTR "*.usr").c_str());
    int num = fv.num_elems();
    delete_findvec(&fv);
    return num;
}

int
empty_trash()
{
    FindVec &fv = *find_to_vec(newfn("trash" DIRSEPSTR "*.usr").c_str());
    int num = fv.num_elems();
    for(int i = 0; i < num; ++i)
    {
        WIN32_FIND_DATA& d = *fv[i];
        if(strcmp(d.cFileName, ".") == 0 ||
                strcmp(d.cFileName, "..") == 0)
            continue;
        remove_user_files(d.cFileName, "trash" DIRSEPSTR "", 0);
    }
    delete_findvec(&fv);
    return 1;
}

void
append_forwarded_text(DwString& s, vc body)
{
    if(body.type() != VC_VECTOR)
    {
        s += "<<can't display old old message>>\r\n";
        return;
    }
    int cant_resolve;
    if(body[QM_BODY_FORWARDED_BODY].is_nil())
    {

        s += "Created by ";
        vc handle;
        handle = uid_to_handle(body[QM_BODY_FROM], &cant_resolve);
        if(cant_resolve)
            handle = " ";
        s += (const char *)handle;
        if(cant_resolve)
        {
            s += " (";
            s += (const char *)uid_to_short_text(body[QM_BODY_FROM]);
            s += ")";
        }
        s += "\r\n";
        s += (const char *)body[QM_BODY_NEW_TEXT];
        return;
    }
    s += "from ";
    vc handle;
    handle = uid_to_handle(body[QM_BODY_FROM], &cant_resolve);
    if(cant_resolve)
        handle = " ";
    s += (const char *)handle;
    if(cant_resolve)
    {
        s += " (";
        s += (const char *)uid_to_short_text(body[QM_BODY_FROM]);
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
    FindVec& fv = *find_to_vec(newfn(fn).c_str());
    int nn = fv.num_elems();
    for(int i = 0; i < nn; ++i)
    {
        WIN32_FIND_DATA& n = *fv[i];
        if(!nodel.contains(n.cFileName))
        {
            DeleteFile(newfn(n.cFileName).c_str());
        }
    }
    delete_findvec(&fv);
}

static
void
find_files_to_keep(DwString subdir, DwString pat, vc nodel)
{
    DwString match(newfn(subdir));

    match += DIRSEPSTR;
    match += pat;
    FindVec& fv = *find_to_vec(match.c_str());
    int nn = fv.num_elems();
    for(int i = 0; i < nn; ++i)
    {
        WIN32_FIND_DATA& n = *fv[i];

        DwString d(newfn(subdir));
        d += DIRSEPSTR;
        d += n.cFileName;
        vc m;
        if(load_info(m, d.c_str(), 1))
        {
            if(!valid_qd_message(m))
            {
                DeleteFile(d.c_str());
                Log->make_entry("deleting bogus item.");
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
                        Log->make_entry("deleteing bogus encrypted package");
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
    delete_findvec(&fv);
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
    // this is a bit dangerous, but for this one case where the
    // names are identical except for a "/tmp/" on the end of the
    // tmp pfx, we'll delete all the files in that path
    if(tmp.length() >= 5 && user.length() + 4 == tmp.length() &&
            user == DwString(tmp.c_str(), user.length()) &&
            tmp.rfind(DIRSEPSTR "tmp" DIRSEPSTR) == tmp.length() - 5)
    {
        DwString tp = tmp;
        tp += "*.*";
        FindVec& fv = *find_to_vec(tp.c_str());
        int nn = fv.num_elems();
        for(int i = 0; i < nn; ++i)
        {
            WIN32_FIND_DATA& n = *fv[i];
            DwString tfn = tmp;
            tfn += n.cFileName;
            DeleteFile(tfn.c_str());

        }
        delete_findvec(&fv);
    }
#endif

}

#if 0
int
is_always_vis(vc uid)
{
    return Always_visible.contains(uid);
}

int
is_never_vis(vc uid)
{
    return Never_visible.contains(uid);
}

void
always_vis_add(vc uid)
{
    Never_visible.del(uid);
    Always_visible.add(uid);
    save_info(Always_visible, "always");
    save_info(Never_visible, "never");
    pal_relogin();
    se_emit(SE_STATUS_CHANGE, uid);
}

void
always_vis_del(vc uid)
{
    // note: this can be called in cases
    // were we just want to clear the always
    // vis attr if it is there, and in that
    // case, the never vis may be set, and we
    // don't want to clear it.
    if(!is_always_vis(uid))
        return;
    Never_visible.del(uid);
    //Never_visible.del(uid);
    Always_visible.del(uid);
    save_info(Always_visible, "always");
    save_info(Never_visible, "never");
    pal_relogin();
    se_emit(SE_STATUS_CHANGE, uid);
}

void
never_vis_add(vc uid)
{
    Never_visible.add(uid);
    Always_visible.del(uid);
    save_info(Always_visible, "always");
    save_info(Never_visible, "never");
    pal_relogin();
    se_emit(SE_STATUS_CHANGE, uid);
}

void
never_vis_del(vc uid)
{
    // note: this can be called in cases
    // were we just want to clear the never
    // vis attr if it is there, and in that
    // case, the always vis may be set, and we
    // don't want to clear it.
    if(!is_never_vis(uid))
        return;
    Never_visible.del(uid);
    //Always_visible.del(uid);
    save_info(Always_visible, "always");
    save_info(Never_visible, "never");
    pal_relogin();
    se_emit(SE_STATUS_CHANGE, uid);
}

void
reset_always_vis()
{
    Always_visible = vc(VC_SET);
    save_info(Always_visible, "always");
}

void
reset_never_vis()
{
    Never_visible = vc(VC_SET);
    save_info(Never_visible, "never");
}


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

#if 0

DwString Diag_res;

static void
check_type(vc v)
{
    if(v[0].type() != VC_STRING)
        Diag_res += "BOGUS infos file key (wrong type.)\r\n";
}

DwString
simple_diagnostics()
{
#ifdef __WIN32__
    // run thru the set of folders in the install folder
    // and see if there is any problems with the naming of the
    // folders, or the consistency of the info associated with the
    // folders.
    // notes: check to make sure everything is writable, including "."

    FindVec &fv = *find_to_vec(newfn("*.usr").c_str());
    DwString &res = Diag_res;
    res = "";

    int n = fv.num_elems();
    for(int i = 0; i < n; ++i)
    {
        DWORD attr;
        WIN32_FIND_DATA& n = *fv[i];
        // check to see that the filename is the right length, case, etc.
        DwString a = n.cFileName;
        //attr = GetFileAttributes(a.c_str());
        if(!(n.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
        {
            res += "Error: ";
            res += a;
            res += " is not a folder.\r\n";
            continue;
        }
#if 0
        if(n.dwFileAttributes|FILE_ATTRIBUTE_READONLY)
        {
            res += "Warning: folder ";
            res += a;
            res += " is read-only.\r\n";
        }
#endif
        if(a.length() != 24)
        {
            res += "Error: folder ";
            res += a;
            res += " is not the right length.\r\n" ;
            continue;
        }
        DwString b = a;
        b.remove(20);
        if(b.find_first_not_of("0123456789abcdefABCDEF") != DwString::npos)
        {
            res += "Error: folder ";
            res += a;
            res += " is not named properly.\r\n";
            continue;
        }
        if(b.find_first_not_of("0123456789abcdef") != DwString::npos)
        {
            res += "Warning: folder name ";
            res += a;
            res += " contains upper-case letters.\r\n";
        }
        // ok, the name is ok at this point, now check to see if the
        // info file is consistent.
        DwString info = a;
        info += DIRSEPSTR "info";
        vc infoc;

        if(access(info.c_str(), 0) != 0)
        {
            res += "Warning: no info file ";
            res += info;
            res += "\r\n";
            goto infos_check;
        }
        attr = GetFileAttributes(info.c_str());
        if(attr&FILE_ATTRIBUTE_DIRECTORY)
        {
            res += "Error: info file ";
            res += info;
            res += " is a folder???\r\n";
        }
        if(attr&FILE_ATTRIBUTE_READONLY)
        {
            res += "Error: info file ";
            res += info;
            res += " is read-only\r\n";
        }

        if(!load_info(infoc, info.c_str()))
        {
            res += "Error: info file ";
            res += info;
            res += " was corrupt (deleted.)\r\n";
        }
        if(!valid_info(infoc))
        {
            res += "Error: info file ";
            res += info;
            res += " not valid.\r\n";
            goto infos_check;
        }
        if(dir_to_uid(a.c_str()) != infoc[QIR_FROM])
        {
            res += "Error: info file ";
            res += info;
            res += " UID doesn't match folder name.\r\n";
        }
infos_check:
        UID_infos.foreach(vcnil, check_type);
        // check the folder uid against what is in the infos file
        vc fuid = dir_to_uid(a.c_str());
        vc iv;
        if(!UID_infos.find(fuid, iv))
        {
            res += "Error: folder ";
            res += a;
            res += " not in infos file.\r\n";
            continue;
        }
        if(!valid_info(iv))
        {
            res += "Error: info for ";
            res += a;
            res += " in infos file ";
            res += "not valid.\r\n";
            continue;
        }
        if(fuid != iv[QIR_FROM])
        {
            res += "Error: infos file key for ";
            res += a;
            res += " doesn't match info UID\r\n";
        }

    }
    return res;
#else
    return DwString("");
#endif
}
#endif

#if 0
int
infos_add(vc uid, vc info)
{
    int new_user = 0;
    // note: this isn't exactly right, since we may have
    // different info for an existing user... we only
    // write it out when a new user comes in... this is an
    // optimization, since the infos file should not be used
    // all that much with the new profile caching.
    UID_infos.add_kv(uid, info);
    if(!UID_infos.contains(uid))
    {
        new_user = 1;
        save_info(UID_infos, "infos");
    }
    return new_user;
}


void
infos_del(vc uid)
{
    if(UID_infos.contains(uid))
    {
        UID_infos.del(uid);
        save_info(UID_infos, "infos");
    }
}
#endif


int
pal_add(vc u)
{
    //dirth_send_get_pal_auth_state(My_UID, u, QckDone(pal_auth_results, 0, u));
    if(!Pals.contains(u))
    {
        Pals.add(u);
        pal_relogin();

        chatq_send_update_pals(pal_to_vector(0));

        return save_info(Pals, "pals");
    }
    return 1;
}

int
pal_del(vc u, int norelogin)
{
    if(Pals.contains(u))
    {
        Pals.del(u);
        //i_grant_del(u);
        //they_grant_del(u);
        if(!norelogin)
        {
            pal_relogin();

            chatq_send_update_pals(pal_to_vector(0));
        }
        return save_info(Pals, "pals");
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
    // one of the copies succeeded, so
    // get rid of the original
    //DeleteFile(newfn((const char *)filename).c_str());
    return 1;
}

#if 0

static
DwString
pfx(const DwString& p, const char *str)
{
    DwString a(p);
    a += DIRSEPSTR;
    a += str;
    return a;
}

static
int
copy_dir(const DwString& d1, const DwString& dest_dir)
{
    // d1 is of the form c:\...\...\foo
    // dest_dir is of similar form, except foo will be copied
    // into the dest dir as "foo"
    // get last component of d1
    DwString dirname(d1);
    int i = dirname.rfind(DIRSEPSTR);
    if(i == DwString::npos)
        return 0;
    dirname.erase(0, i + 1);
    DwString ndest_dir(dest_dir);
    ndest_dir += DIRSEPSTR;
    ndest_dir += dirname;
    mkdir(ndest_dir.c_str());

    DwString pat(d1);
    pat += DIRSEPSTR "*.*";
    FindVec& fv = *find_to_vec(pat.c_str());
    int n = fv.num_elems();
    for(int j = 0; j < n; ++j)
    {
        WIN32_FIND_DATA& d = *fv[j];
        if(strcmp(d.cFileName, ".") == 0 ||
                strcmp(d.cFileName, "..") == 0)
            continue;
        DwString srcfile(d1);
        srcfile += DIRSEPSTR;
        srcfile += d.cFileName;
        DwString ndest_file(ndest_dir);
        ndest_file += DIRSEPSTR;
        ndest_file += d.cFileName;
        CopyFile(srcfile.c_str(), ndest_file.c_str(), 1);
    }
    delete_findvec(&fv);
    return 0;
}

static int
find_ini_key(const char *filename, const char *key, DwString& val_out)
{
    FILE *f = fopen(filename, "r");
    if(!f)
        return 0;
    char a[100];
    while(fgets(a, sizeof(a) - 1, f))
    {
        int m = (strlen(a) < strlen(key) ? strlen(a) : strlen(key));
        if(strncmp(a, key, m) == 0)
        {
            char *i = strchr(a, '=');
            if(i == 0)
                continue;
            DwString ret(i + 1, 0, strlen(a) - (i - a) - 1);
            // strip off trailing \r\n mumbo jumbo
            while(ret.length() >= 1 && (ret[ret.length() - 1] == '\n' || ret[ret.length() - 1] == '\r'))
            {
                ret.erase(ret.length() - 1);
            }
            val_out = ret;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

static vc cvt_pals;
static void
append_pals(vc item)
{
    if(!cvt_pals.contains(item))
        cvt_pals.append(item);
}

static void
add_session_infos(vc item)
{
    if(!Session_infos.contains(item[0]))
    {
        Session_infos.add_kv(item[0], item[1]);
    }
}

// two years on, no need for this anymore... plus it probably will screw up
// a new install anyways.
int
upgrade_cdc32_account(const DwString &cdc32_dir, const DwString& cdcx_dir)
{
#define copy_there_here(f1) \
	CopyFile(pfx(cdc32_dir, f1).c_str(), pfx(cdcx_dir, f1).c_str(), 0);

    //copy_there_here("accept")
    //copy_there_here("always")
    //copy_there_here("igrant")
    copy_there_here("infos")
    //copy_there_here("never")
    //copy_there_here("noise")
    //copy_there_here("theygrnt")
    //copy_there_here("timing")

    int is_icu2 = 0;
    if(access(pfx(cdc32_dir, "icuii.ini").c_str(), 0) == 0)
        is_icu2 = 1;

    // here is where we need to convert the "accept" list
    // into the "pal" list
    // note: the auth file might be problematic...
    // maybe just instead of copying it, we just insert the new
    // uid into it. have to force a restart as well to get the new
    // uid. also, the password related stuff needs to be updated.
    // ie, the settings.qds file is out of whack for the new uid.
    // i'm guessing what we really need to do is make them pick
    // a new password, and have the server accept that once when
    // they create their new account.
    // dwyco.ini will have some regcode infomation that needs to be
    // imported. look for the "reg3" key
    //
    // for importing just msgs, if someone imports new msgs multiple times
    // from cdc32, remove the index files, otherwise the msgs won't be seen.
    vc always_accept;
    // merge other pal list, in case they have been doing stuff
    cvt_pals = vc(VC_VECTOR);
    for(int i = 0; i < Pals.num_elems(); ++i)
        cvt_pals.append(Pals[i]);
    DwString palfile;
    if(is_icu2)
        palfile = pfx(cdc32_dir, "pals");
    else
        palfile = pfx(cdc32_dir, "accept");
    if(load_info(always_accept, palfile.c_str()))
    {
        always_accept.foreach(vcnil, append_pals);
        MoveFile(pfx(cdcx_dir, "pals").c_str(), pfx(cdcx_dir, "pals.cdcx").c_str());
        save_info(cvt_pals, pfx(cdcx_dir, "pals").c_str());
    }
    DwString sifile = pfx(cdc32_dir, "sinfo");
    vc si;
    if(load_info(si, sifile.c_str()))
    {
        si.foreach(vcnil, add_session_infos);
        MoveFile(pfx(cdcx_dir, "sinfo").c_str(), pfx(cdcx_dir, "sinfo.cdcx").c_str());
        save_info(Session_infos, pfx(cdcx_dir, "sinfo").c_str());
    }
    MoveFile(pfx(cdcx_dir, "auth").c_str(), pfx(cdcx_dir, "auth.cdcx").c_str());
    MoveFile(pfx(cdcx_dir, "settings.qds").c_str(), pfx(cdcx_dir, "settings.qds.cdcx").c_str());
    copy_there_here("auth")
    MoveFile(pfx(cdcx_dir, "auth").c_str(), pfx(cdcx_dir, "auth.upg").c_str());

    if(!is_icu2)
    {
        DwString reg;
        if(find_ini_key(pfx(cdc32_dir, "dwyco.ini").c_str(), "reg3", reg))
        {
            dwyco_set_regcode(reg.c_str());
        }


#define xfer_from_cdc32(cdc_setting, cdcx_setting) \
{ \
DwString tmp; \
	if(find_ini_key(pfx(cdc32_dir, "dwyco.ini").c_str(), cdc_setting, tmp)) \
	{ \
		dwyco_set_setting(cdcx_setting, tmp.c_str()); \
	} \
}

        xfer_from_cdc32("username", "user/username")
        xfer_from_cdc32("description", "user/description")
        xfer_from_cdc32("email", "user/email")
    }
    else
    {
#define xfer_from_icu2(cdc_setting, cdcx_setting) \
{ \
DwString tmp; \
	if(find_ini_key(pfx(cdc32_dir, "icuii.ini").c_str(), cdc_setting, tmp)) \
	{ \
		dwyco_set_setting(cdcx_setting, tmp.c_str()); \
	} \
}

        xfer_from_cdc32("NickName", "user/username")
        xfer_from_cdc32("Comment", "user/description")
        xfer_from_cdc32("Email", "user/email")
    }

    return 0;
}

int
copy_cdc32_zaps(const DwString &cdc32_dir, const DwString& cdcx_dir, DwycoStatusCallback cb)
{
    //copy_dir(pfx(cdc32_dir, "trash"), cdcx_dir);
    // copy all the *.usr files in the trash
#if 0
    {
        DwString target_trash_dir(pfx(cdcx_dir, "trash"));
        mkdir(target_trash_dir.c_str());
        DwString pat(cdc32_dir);
        pat += DIRSEPSTR "trash" DIRSEPSTR "*.usr";
        FindVec& fv = *find_to_vec(pat.c_str());
        int n = fv.num_elems();
        for(int j = 0; j < n; ++j)
        {
            WIN32_FIND_DATA& d = *fv[j];
            if(strcmp(d.cFileName, ".") == 0 ||
                    strcmp(d.cFileName, "..") == 0)
                continue;
            DwString srcdir(cdc32_dir);
            srcdir += DIRSEPSTR "trash" DIRSEPSTR "";
            srcdir += d.cFileName;
            copy_dir(srcdir.c_str(), target_trash_dir);
            if(cb)
            {
                DwString msg("Copied trash ");
                msg += d.cFileName;
                int percent = (j * 100) / n;
                (*cb)(0, msg.c_str(), percent, 0);
            }
        }
        delete_findvec(&fv);
    }
#endif

    {
        // copy all the *.usr files
        DwString pat(cdc32_dir);
        pat += DIRSEPSTR "*.usr";
        FindVec& fv = *find_to_vec(pat.c_str());
        int n = fv.num_elems();
        for(int j = 0; j < n; ++j)
        {
            WIN32_FIND_DATA& d = *fv[j];
            if(strcmp(d.cFileName, ".") == 0 ||
                    strcmp(d.cFileName, "..") == 0)
                continue;
            DwString srcdir(cdc32_dir);
            srcdir += DIRSEPSTR "";
            srcdir += d.cFileName;
            copy_dir(srcdir.c_str(), cdcx_dir);
            // delete whatever *.idx file might be in the dst
            // in case this is a merge from a previous import
            DwString idx(cdcx_dir);
            idx += DIRSEPSTR "";
            idx += d.cFileName;
            idx += DIRSEPSTR "date.idx";
            DeleteFile(idx.c_str());
            if(cb)
            {
                DwString msg("Copied user ");
                msg += d.cFileName;
                int percent = (j * 100) / n;
                (*cb)(0, msg.c_str(), percent, 0);
            }
        }
        delete_findvec(&fv);
    }

    return 0;
}
#endif

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
        Mutual_ignore.add(m[1][1]);
    }
    else if(m[1][0] == vc("d"))
    {
        Mutual_ignore.del(m[1][1]);
    }
    // this is probably best left silent
    //Refresh_users = 1;
}





