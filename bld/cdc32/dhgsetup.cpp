
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
#include "simple_property.h"


using namespace CryptoPP;
using namespace dwyco;

extern sigprop<vc> Group_uids;

namespace dwyco {

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


    }


};
static DHG_sql *DHG_db;

void
init_dhg()
{
    const char *grpname;
    grpname = getenv("DWYCO_GROUP");
    if(!grpname)
        grpname = "foo@bar.com";

    DH_alternate *dha = new DH_alternate;
    dha->init(My_UID, grpname);
    dha->load_account(grpname);
    vc v = DHG_db->sql_simple("select * from group_uids");
    vc v2(VC_VECTOR);
    for(int i = 0; i <v.num_elems(); ++i)
        v2[i] = from_hex(v[i][0]);
    Group_uids = v2;
    Current_alternate = dha;
    Group_uids.value_changed.connect_memfun(dha, &DH_alternate::update_group);
}



// wip: this class can be used to encrypt messages with
// an alternate DH key (for example, if a message needs to be
// send to a group.) the "alternate_name" might be a token or
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
        a.append((long)time(0));
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

int
DH_alternate::insert_new_key(vc alt_name, vc grp_key)
{
    if(!insert_record(My_UID, alt_name, grp_key))
        return 0;
    return 1;
}

int
DH_alternate::remove_key(vc alt_name)
{
    DHG_db->sql_simple("delete from keys where alt_name = ?1", alt_name);
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

}



