
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
#include "xinfo.h"
#include "dwrtlog.h"
#include "qauth.h"
#include "vcudh.h"
#include "dhgsetup.h"
#include "simplesql.h"
#include "sha3.h"
#include "rng.h"
#include "aes.h"
#include "modes.h"
#include "simple_property.h"
#include "ezset.h"
#include "se.h"


using namespace CryptoPP;
using namespace dwyco;

extern sigprop<vc> Group_uids;
void drop_all_sync_calls(DH_alternate *);

namespace dwyco {
sigprop<DH_alternate *> Current_alternate;
vc DH_alternate::Group_join_password;

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

static vc
sha3(vc s)
{
    if(s.type() != VC_STRING)
        return vcnil;

    SHA3_256 md;
    SecByteBlock b(md.DigestSize());
    md.Update((const byte *)(const char *)s, s.len());
    md.Final(b);
    vc ret(VC_BSTRING, (const char *)b.data(), (long)md.DigestSize());
    return ret;
}

static DHG_sql *DHG_db;
#define sql DHG_db->sql_simple

static
void
change_current_group(vc, vc new_name)
{
    if(new_name.len() == 0)
    {
        auto odha = Current_alternate;
        Current_alternate = 0;
        delete odha;
        return;
    }
    DH_alternate *dha = new DH_alternate;
    dha->init(My_UID, new_name);
    dha->load_account(new_name);

    auto odha = Current_alternate;
    Current_alternate = dha;
    delete odha;
}

static
void
change_join_key(vc, vc new_key)
{
    DH_alternate::Group_join_password = new_key;
    if(!Current_alternate)
        return;
    Current_alternate->password = new_key;
}

static
void
eager_changed(vc, vc)
{
    drop_all_sync_calls(0);
}

void
init_dhg()
{
    if(Current_alternate)
        return;

    se_emit_group_status_change();

    vc alt_name;
    vc pw;

    {
        const char *grp_name;
        grp_name = getenv("DWYCO_GROUP");

        if(!grp_name)
        {
            alt_name = get_settings_value("group/alt_name");
            if(alt_name.is_nil() || alt_name.len() == 0)
                return;
            //bind_sql_setting("group/alt_name", change_current_group);
        }
        else
            alt_name = grp_name;

        const char *grp_pw;
        grp_pw = getenv("DWYCO_GROUP_PW");

        if(!grp_pw)
        {
            pw = get_settings_value("group/join_key");
            //if(pw.is_nil() || pw.len() == 0)
            //    return;
            bind_sql_setting("group/join_key", change_join_key);
        }
        else
            pw = grp_pw;
    }
    bind_sql_setting("sync/eager", eager_changed);
    DH_alternate *dha = new DH_alternate;
    dha->init(My_UID, alt_name);
    dha->load_account(alt_name);
    vc v = DHG_db->sql_simple("select * from group_uids");
    vc v2(VC_VECTOR);
    for(int i = 0; i < v.num_elems(); ++i)
        v2[i] = from_hex(v[i][0]);
    Group_uids = v2;

    dha->password = pw;
    Current_alternate = dha;
    Group_uids.value_changed.connect_ptrfun(&DH_alternate::update_group);
    Current_alternate.value_changed.connect_ptrfun(drop_all_sync_calls);
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
    this->alternate_name = alternate_name;
    if(!DHG_db)
    {
        DHG_db = new DHG_sql;
        DHG_db->init();
    }
}

void
DH_alternate::leave()
{
    DHG_db->start_transaction();
    DHG_db->sql_simple("delete from group_uids");
    DHG_db->sql_simple("delete from keys where alt_name = ?1", alternate_name);
    DHG_db->sql_simple("delete from sigs where alt_name = ?1", alternate_name);
    DHG_db->commit_transaction();
}

void
DH_alternate::update_group(vc uids)
{
    try
    {
        DHG_db->start_transaction();
        DHG_db->sql_simple("delete from group_uids");
        for(int i = 0; i < uids.num_elems(); ++i)
        {
            DHG_db->sql_simple("insert into group_uids values(?1)", to_hex(uids[i]));
        }
        DHG_db->commit_transaction();
    }
    catch(...)
    {
        DHG_db->rollback_transaction();
    }
}

int
DH_alternate::insert_record(vc uid, vc alt_name, vc dh_static)
{
    try {
        DHG_db->start_transaction();
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
        DHG_db->commit_transaction();

    } catch (...) {
        DHG_db->rollback_transaction();
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
DH_alternate::load_account(vc alternate_name)
{
    vc res;
    try {
        res = DHG_db->sql_simple("select pubkey, privkey from keys where uid = ?1 and alt_name = ?2 order by time desc limit 1",
                                 to_hex(uid), alternate_name);
    } catch (...) {
        GRTLOG("cant create DH account", 0, 0);
        return 0;
    }
    if(res.num_elems() == 0)
    {
        if(!new_account())
        {
            GRTLOG("cant create DH account", 0, 0);
            return 0;
        }
        GRTLOG("created new DH account", 0, 0);
        return 2;
    }
    DH_static = vc(VC_VECTOR);
    DH_static[DH_STATIC_PRIVATE] = res[0][1];
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
        DHG_db->start_transaction();
        remove_key(alt_name);
        sql("insert or replace into keys (alt_name, pubkey, privkey, uid, time) values(?1, ?2, ?3, ?4, strftime('%s', 'now'))",
            alt_name, blob(grp_key), blob(""), to_hex(My_UID));
        sql("insert or replace into sigs(alt_name, sig) values(?1, ?2)",
            alt_name, blob(sig));
        DHG_db->commit_transaction();
    }
    catch(...)
    {
        DHG_db->rollback_transaction();
        return 0;
    }
    return 1;
}



int
DH_alternate::insert_sig(vc alt_name, vc sig)
{
    if(!DHG_db)
        return 0;

    DHG_db->sql_simple("insert or ignore into sigs (alt_name, sig) values(?1, ?2)",
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
    DHG_db->start_transaction();
    DHG_db->sql_simple("update keys set privkey = ?2 where alt_name = ?1 and pubkey = ?3",
                       alt_name,
                       blob(grp_key[DH_STATIC_PRIVATE]),
                       blob(grp_key[DH_STATIC_PUBLIC])
                       );
    DHG_db->commit_transaction();
    return 1;
}

int
DH_alternate::remove_key(vc alt_name)
{
    if(!DHG_db)
        return 0;
    DHG_db->start_transaction();
    DHG_db->sql_simple("delete from keys where alt_name = ?1", alt_name);
    DHG_db->sql_simple("delete from sigs where alt_name = ?1", alt_name);
    DHG_db->commit_transaction();
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
DH_alternate::hash_key_material()
{
    SHA3_256 md;
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
    vc res = DHG_db->sql_simple("select pubkey, privkey from keys order by time desc");
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

}



