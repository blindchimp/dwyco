
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <time.h>
#ifdef LINUX
#include <unistd.h>
#endif

#include "vc.h"
#include "dwrtlog.h"
#include "qauth.h"
#include "vcudh.h"
#include "dhgsetup.h"
#include "simplesql.h"
//#include "sha3.h"
#include "aes.h"
#include "modes.h"
#include "keccak.h"
#include "simple_property.h"
#include "ezset.h"
#include "se.h"
#include "qmsgsql.h"
#include "synccalls.h"
#include "grpmsg.h"
#include "pulls.h"


using namespace CryptoPP;
using namespace dwyco;

namespace dwyco {
using namespace dwyco::dhg;

sigprop<DH_alternate *> Current_alternate;
vc DH_alternate::Group_join_password;
sigprop<vc> Group_uids;

struct DHG_sql : public SimpleSql
{
    DHG_sql() : SimpleSql("dhg.sql") {}

    void init_schema(const DwString&) {
        sql_simple("create table if not exists keys ("
                   "uid text collate nocase not null, "
                   "alt_name text collate nocase  not null, "
                   "pubkey blob not null,"
                   "privkey blob not null,"
                   "time integer,"
                   "unique(alt_name, pubkey, privkey) on conflict ignore"
                   ")");
        sql_simple("create index if not exists keys_uid on keys(uid)");
        sql_simple("create index if not exists keys_alt_name on keys(alt_name)");
        sql_simple("create table if not exists group_uids ("
                   "uid text collate nocase unique on conflict ignore not null on conflict fail"
                   ")");
        sql_simple("create table if not exists sigs (alt_name text primary key collate nocase not null, sig blob not null)");


    }


};

// WARNING: this sha3 produces pre-5.6.4 values (which was used in old software.)
// for compat, we changed it to Keccak, since it produced the old values.
// WARNING WARNING: this is used for "group id", which were put into messages
// starting several years ago, in order to aid in grouping messages. if you
// change this function, the group id's will look different, and it will be
// displayed differently in the UI.
// unfortunately, if we change the hash function, it will be collosal pain in the
// butt to re-write the group info all over the place. it might be a good idea to
// come up with a better way of doing it that can avoid extra steps that might change
// or make it more flexible in some way.
//
static
vc
sha3(vc s)
{
    if(s.type() != VC_STRING)
        return vcnil;

    Keccak_256 md;
    SecByteBlock b(md.DigestSize());
    md.Update((const byte *)(const char *)s, s.len());
    md.Final(b);
    vc ret(VC_BSTRING, (const char *)b.data(), (long)md.DigestSize());
    return ret;
}

static
vc
to_lower(vc an)
{
    DwString a(an);
    a.to_lower();
    return a.c_str();
}

static DHG_sql *DHG_db;
#define sql DHG_db->sql_simple

namespace dhg {

void
sql_start_transaction()
{
    DHG_db->start_transaction();
}
void
sql_commit_transaction()
{
    DHG_db->commit_transaction();
}
void
sql_rollback_transaction()
{
    DHG_db->rollback_transaction();
}

}

static
void
change_current_group(vc, vc )
{
    // least surprise: when we change groups we discard our
    // sync history state. this is mainly so we don't
    // propagate tombstones that might delete things on
    // other clients when we join a group we may have been in before.
    // note: it is probably better not to generate the tombstones
    // in the first place if we are not in "group mode", but
    // that will need some schema changes
    remove_sync_state();
    se_emit_group_status_change();
}

static
void
change_join_key(vc, vc new_key)
{
    se_emit_group_status_change();
    DH_alternate::Group_join_password = new_key;
}

static
void
eager_changed(vc, vc)
{
    drop_all_sync_calls(0);
    se_emit_group_status_change();
    pulls::clear_all_asserts();
}

static
void
successful_join(vc alt_name)
{
    set_settings_value("group/alt_name", alt_name);
}

void
init_dhgdb()
{
    if(!DHG_db)
    {
        DHG_db = new DHG_sql;
        DHG_db->init();
    }
}

void
exit_dhgdb()
{
    if(DHG_db)
    {
        DHG_db->exit();
        delete DHG_db;
        DHG_db = 0;
    }
}

static int Init;
// note: there is no "exit" for this, which should probably
// be revisited. for now, we just ignore multiple init's, as
// we don't really need to redo everything even when switching
// threads.
void
init_dhg()
{
    // emit this message to prompt clients to re-read the state
    // if necessary. but this is a kluge, should just have clients
    // initialize themselves properly.
    se_emit_group_status_change();
    if(Init)
        return;
    Init = 1;

    bind_sql_setting("group/alt_name", change_current_group);
    bind_sql_setting("group/join_key", change_join_key);
    Join_signal.connect_ptrfun(successful_join, ssns::UNIQUE);
    vc alt_name;
    vc pw;

    pw = get_settings_value("group/join_key");
    DH_alternate::Group_join_password = pw;
    alt_name = get_settings_value("group/alt_name");
    if(alt_name.is_nil() || alt_name.len() == 0)
        return;


    bind_sql_setting("sync/eager", eager_changed);
    DH_alternate *dha = new DH_alternate;
    dha->init(My_UID, alt_name);
    if(!dha->load_account(alt_name, false))
    {
        // can't find the given name, something may have happened
        // to our key, maybe restoring from a backup or whatever.
        // revert back to "no group" mode
        delete dha;
        set_settings_value("group/alt_name", "");
        set_settings_value("group/join_key", "");
        return;
    }
    vc v = DHG_db->sql_simple("select * from group_uids");
    vc v2(VC_VECTOR);
    for(int i = 0; i < v.num_elems(); ++i)
        v2[i] = from_hex(v[i][0]);
    Group_uids = v2;

    //dha->password = pw;
    Current_alternate = dha;
    Group_uids.value_changed.connect_ptrfun(&DH_alternate::update_group, ssns::UNIQUE);
    Current_alternate.value_changed.connect_ptrfun(drop_all_sync_calls, ssns::UNIQUE);
}



// wip: this class can be used to encrypt messages with
// an alternate DH key (for example, if a message needs to be
// sent to a group.) the "alternate_name" might be a token or
// email address or something that represents the group.
//
// this doesn't handle all the stuff related to
// getting the keys sent around, but i think it will be necessary
// to have multiple uid, alternate_name, key tuples to handle
// cases where people change email addresses, or try to create
// a group from multiple devices using different uids, but the same
// alternate_name, etc.
//
void
DH_alternate::init(vc uid, vc alternate_name)
{
    this->uid = uid;
    this->alternate_name = to_lower(alternate_name);
    if(!DHG_db)
    {
        DHG_db = new DHG_sql;
        DHG_db->init();
    }
}

void
DH_alternate::leave()
{
    try
    {
        sql_start_transaction();
        sql("delete from group_uids");
        sql("delete from keys where alt_name = ?1", alternate_name);
        sql("delete from sigs where alt_name = ?1", alternate_name);
        sql_commit_transaction();
        DH_static = vcnil;
        alternate_name = vcnil;
    }
    catch(...)
    {
        sql_rollback_transaction();
        throw -1;
    }
}

void
DH_alternate::update_group(vc uids)
{
    try
    {
        sql_start_transaction();
        sql("delete from group_uids");
        for(int i = 0; i < uids.num_elems(); ++i)
        {
            sql("insert into group_uids values(?1)", to_hex(uids[i]));
        }
        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }
}

int
DH_alternate::insert_record(vc uid, vc alt_name, vc dh_static)
{
    try {
        sql_start_transaction();
        VCArglist a;
        a.append("insert into keys values(?1, ?2, ?3, ?4, ?5)");
        a.append(to_hex(uid));
        a.append(alt_name);
        vc v(VC_VECTOR);
        v[0] = "blob";
        v[1] = dh_static[DH_STATIC_PUBLIC];
        a.append(v);
        vc v2(VC_VECTOR);
        v2[0] = "blob";
        v2[1] = dh_static[DH_STATIC_PRIVATE];
        a.append(v2);
        a.append(time(0));
        DHG_db->query(&a);
        sql_commit_transaction();

    } catch (...) {
        sql_rollback_transaction();
        return 0;
    }
    return 1;

}

int
DH_alternate::new_account()
{
    vc entropy = get_entropy();
    DH_static = udh_new_static(entropy);
    if(!insert_record(uid, alternate_name, DH_static))
        return 0;
    return 1;
}

// return 2 means account was created, and the public static info will
// have to be refreshed in the profile, etc.
int
DH_alternate::load_account(vc alternate_name, bool create_if_not_exists)
{
    vc res;
    try {
        res = sql("select pubkey, privkey from keys where uid = ?1 and alt_name = ?2 order by time desc limit 1",
                                 to_hex(uid), alternate_name);
    } catch (...) {
        GRTLOG("cant create DH account", 0, 0);
        return 0;
    }
    if(res.num_elems() == 0)
    {
        if(!create_if_not_exists || !new_account())
        {
            GRTLOG("cant create DH account", 0, 0);
            return 0;
        }
        GRTLOG("created new DH account", 0, 0);
        return 2;
    }
    vc priv = res[0][1];
    if(priv.is_nil() || priv.len() == 0)
        return 0;
    DH_static = vc(VC_VECTOR);
    DH_static[DH_STATIC_PRIVATE] = priv;
    DH_static[DH_STATIC_PUBLIC] = res[0][0];
    GRTLOG("loaded existing DH account", 0, 0);
    return 1;
}

static
vc
blob(vc b)
{
    vc v(VC_VECTOR);
    v[0] = "blob";
    v[1] = b;
    return v;
}

// replaces existing pubkey and remove private key
// used in the case we will be finding the private key from
// another group member
int
DH_alternate::insert_public_key(vc alt_name, vc grp_key, vc sig)
{
    if(!DHG_db)
        return 0;
    try
    {
        DwString a(alt_name);
        alt_name = to_lower(alt_name);
        sql_start_transaction();
        remove_key(alt_name);
        sql("insert or replace into keys (alt_name, pubkey, privkey, uid, time) values(?1, ?2, ?3, ?4, strftime('%s', 'now'))",
            alt_name, blob(grp_key), blob(""), to_hex(My_UID));
        sql("insert or replace into sigs(alt_name, sig) values(?1, ?2)",
            alt_name, blob(sig));
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
DH_alternate::insert_sig(vc alt_name, vc sig)
{
    if(!DHG_db)
        return 0;
    alt_name = to_lower(alt_name);
    sql("insert or ignore into sigs (alt_name, sig) values(?1, ?2)",
                       alt_name,
                       blob(sig)
                       );
    return 1;
}

int
DH_alternate::insert_private_key(vc alt_name, vc grp_key)
{
    if(!DHG_db)
        return 0;
    try
    {
        sql_start_transaction();
        alt_name = to_lower(alt_name);
        vc res = sql("select 1 from keys where alt_name = ?1 and pubkey = ?2",
                                    alt_name,
                                    blob(grp_key[DH_STATIC_PUBLIC]));
        if(res.num_elems() != 1)
        {
            // this might happen if someone manages to confuse the server
            // and is now giving us a different public key to go with the
            // private key.
            // note: there probably needs to be some validation in here at some
            // point, like checking the signature etc.etc.
            throw -1;
        }

        sql("update keys set privkey = ?2 where alt_name = ?1 and pubkey = ?3",
                           alt_name,
                           blob(grp_key[DH_STATIC_PRIVATE]),
                           blob(grp_key[DH_STATIC_PUBLIC])
                           );
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
DH_alternate::remove_key(vc alt_name)
{
    if(!DHG_db)
        return 0;
    sql_start_transaction();
    alt_name = to_lower(alt_name);
    sql("delete from keys where alt_name = ?1", alt_name);
    sql("delete from sigs where alt_name = ?1", alt_name);
    sql_commit_transaction();
    return 1;
}

int
DH_alternate::has_private_key(vc alt_name)
{
    if(!DHG_db)
        return 0;
    sql_start_transaction();
    vc res = sql("select privkey from keys where alt_name = ?1", alt_name);
    sql_commit_transaction();
    if(res.num_elems() == 0)
        return 0;
    if(res[0][0].is_nil() || res[0][0].len() == 0)
        return 0;
    return 1;
}

vc
DH_alternate::my_static()
{
    if(DH_static.is_nil())
        return vcnil;
    vc ret(VC_VECTOR);
    ret[DH_STATIC_PUBLIC] = DH_static[DH_STATIC_PUBLIC];
    ret[DH_STATIC_PRIVATE] = DH_static[DH_STATIC_PRIVATE];
    return ret;
}


vc
DH_alternate::my_static_public()
{
    if(DH_static.is_nil())
        return vcnil;
    vc ret(VC_VECTOR);
    ret[DH_STATIC_PUBLIC] = DH_static[DH_STATIC_PUBLIC];

    return ret;
}

vc
DH_alternate::get_gid(vc pubkey)
{
    Keccak_256 md;
    SecByteBlock b(md.DigestSize());
    md.Update((const byte *)(const char *)pubkey, pubkey.len());
    md.Final(b);
    DwString gid((const char *)b.BytePtr(), b.SizeInBytes());
    gid.remove(10);
    return vc(VC_BSTRING, gid.c_str(), gid.length());
}

vc
DH_alternate::get_gid()
{
    if(DH_static.is_nil())
        return vcnil;
    return get_gid(DH_static[DH_STATIC_PUBLIC]);
}

vc
DH_alternate::hash_key_material()
{
    Keccak_256 md;
    SecByteBlock b(md.DigestSize());
    vc s = DH_static[DH_STATIC_PUBLIC];
    md.Update((const byte *)(const char *)s, s.len());
    s = DH_static[DH_STATIC_PRIVATE];
    md.Update((const byte *)(const char *)s, s.len());
    md.Final(b);
    vc ret(VC_BSTRING, (const char *)b.data(), (long)md.DigestSize());
    return ret;
}

vc
DH_alternate::get_all_keys()
{
    if(!DHG_db)
        return vcnil;
    vc res = sql("select pubkey, privkey from keys order by time desc");
    vc ret(VC_VECTOR);
    int n = res.num_elems();
    for(int i = 0; i < n; ++i)
    {
        vc v(VC_VECTOR);
        v[DH_STATIC_PUBLIC] = res[i][0];
        v[DH_STATIC_PRIVATE] = res[i][1];
        ret.append(v);
    }
    return ret;
}

vc
DH_alternate::gen_challenge_msg(vc& nonce_out)
{
    vc nonce = get_entropy();
    vc key;
    vc pub(VC_VECTOR);
    pub[0] = my_static_public();
    vc material = dh_store_and_forward_material2(pub, key);

    ECB_Mode<AES>::Encryption kc;
    kc.SetKey((const byte *)(const char *)key, key.len());
    byte buf[16];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, (const char *)nonce, dwmin(sizeof(buf), nonce.len()));
    byte checkstr[sizeof(buf)];
    kc.ProcessData(checkstr, buf, sizeof(checkstr));

    vc msg(VC_VECTOR);
    msg[0] = material;
    msg[1] = vc(VC_BSTRING, (const char *)checkstr, sizeof(checkstr));
    nonce_out = sha3(vc(VC_BSTRING, (const char *)buf, sizeof(buf)));
    return msg;
}

int
DH_alternate::challenge_recv(vc m, vc& resp)
{
    vc priv(VC_VECTOR);
    priv[0] = my_static();
    vc key = dh_store_and_forward_get_key2(m[0], priv);
    if(key.is_nil())
        return 0;

    vc enonce = m[1];

    ECB_Mode<AES>::Decryption kc;
    kc.SetKey((const byte *)(const char *)key, key.len());
    byte buf[16];
    memset(buf, 0, sizeof(buf));
    memcpy(buf, (const char *)enonce, dwmin(sizeof(buf), enonce.len()));
    byte checkstr[sizeof(buf)];
    kc.ProcessData(checkstr, buf, sizeof(checkstr));

    resp = sha3(vc(VC_BSTRING, (const char *)checkstr, sizeof(checkstr)));
    return 1;
}

void
update_profiles_for_new_membership()
{
    if(!DHG_db)
        return;
    if(!Current_alternate)
        return;
    DHG_db->attach("prfdb.sql", "prf");

    try
    {
        sql_start_transaction();
        // find all uid's that proport to have our public alt_key
        // if they aren't in the current set of group uid's, get rid of them
        // so they are refetched.
        sql(
            "with foo(uid) as (select uid from pubkeys where alt_static_public = (select pubkey from keys where alt_name = ?1))"
             "delete from prf.pubkeys where uid in (select uid from foo except select uid from group_uids)",
                    Current_alternate->alt_name()
        );

        // if there are new members in the group, but their pub keys in the profile cache
        // don't match, delete them
        sql(
            "with foo(uid) as (select uid from pubkeys,group_uids using(uid) where pubkeys.alt_static_public != (select pubkey from keys where alt_name = ?1))"
             "delete from prf.pubkeys where uid in (select * from foo)",
                    Current_alternate->alt_name()
                    );


        sql_commit_transaction();
    }
    catch(...)
    {
        sql_rollback_transaction();
    }

    //DHG_db->detach("prf");
}

}



