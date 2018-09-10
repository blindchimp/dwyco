
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

using namespace dwyco;

struct DHG_sql : public SimpleSql
{
    DHG_sql() : SimpleSql("dhg.sql") {}

    void init_schema() {
        sql_simple("create table if not exists keys ("
                   "uid text collate nocase, "
                   "alt_name text collate nocase, "
                   "pubkey blob,"
                   "privkey blob,"
                   "time integer"
                   ")");
        sql_simple("create index if not exists keys_uid on keys(uid)");
        sql_simple("create index if not exists keys_alt_name on keys(alt_name)");


    }


};

static DHG_sql *DHG_db;

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

int
DH_alternate::new_account()
{
    vc entropy = get_entropy();
    DH_static = udh_new_static(entropy);
    try {
        DHG_db->start_transaction();
        VCArglist a;
        a.append("insert into keys values($1, $2, $3, $4, $5)");
        a.append(to_hex(uid));
        a.append(alternate_name);
        vc v(VC_VECTOR);
        v[0] = "blob";
        v[1] = DH_static[DH_STATIC_PUBLIC];
        a.append(v);
        vc v2(VC_VECTOR);
        v2[0] = "blob";
        v2[1] = DH_static[DH_STATIC_PRIVATE];
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

// return 2 means account was created, and the public static info will
// have to be refreshed in the profile, etc.
int
DH_alternate::load_account(vc uid, vc alternate_name)
{
    vc res;
    try {
        VCArglist a;
        a.append("select pubkey, privkey from keys where alt_name = $1 order by time desc limit 1");
        //a.append(to_hex(uid));
        a.append(alternate_name);
        res = DHG_db->query(&a);
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

//vc
//DH_alternate::my_combined_publics()
//{
//    if(DH_static.is_nil())
//        return vcnil;
//    return udh_just_publics(DH_static);
//}

//vc
//DH_alternate::static_material(vc combined_material)
//{
//    // warning: the indexing between combined and
//    // separate key-pairs is messed up, don't rely on
//    // defines or correspondence between the indexes.
//    // ugh.
//    vc ret(VC_VECTOR);
//    ret[DH_STATIC_PRIVATE] = combined_material[2];
//    ret[DH_STATIC_PUBLIC] = combined_material[3];
//    return ret;
//}

//vc
//DH_alternate::gen_combined_keys()
//{
//    vc entropy = get_entropy();
//    return udh_gen_keys(DH_static, entropy);
//}

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



