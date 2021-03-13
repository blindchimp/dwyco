#include "profiledb.h"
#include "simplesql.h"
#include "qauth.h"
#include "ezset.h"
#include "ser.h"
#include "qdirth.h"
#include "se.h"
#include "dhsetup.h"
#include "dhgsetup.h"
#include "vcudh.h"
#include "dwrtlog.h"
#include "vccrypt2.h"
#include "ssns.h"
#include "fnmod.h"

extern vc Session_infos;
extern vc My_UID;

namespace dwyco {
ssns::signal2<vc, int> Profile_updated;
ssns::signal2<vc, int> Keys_updated;

struct Sql : public SimpleSql
{
    Sql() : SimpleSql("prfdb.sql") {}

    void init_schema(const DwString&) {
        sql_simple("create table if not exists prf ("
                   "uid text collate nocase primary key not null, "
                   "pack blob not null, "
                   "media blob, "
                   "chksum text collate nocase not null, "
                   "reviewed not null, "
                   "regular not null, "
                   "image blob, "
                   "time integer"
                   ")");

        sql_simple("create table if not exists pubkeys ("
                   "uid text collate nocase primary key not null, "
                   "static_public blob, "
                   "dwyco_sig blob, "
                   "alt_static_public blob, "
                   "alt_server_sig blob, "
                   "alt_gname, "
                   "time integer"
                   ")");
        sql_simple("create index if not exists altidx on pubkeys(alt_static_public)");

    }


};

static Sql *sDb;
static vc Prf_memory_cache;
static vc Prf_session_cache;

static vc Pk_memory_cache;
static vc Pk_session_cache;

static
vc
sql_simple(const char *sql, const vc& a0 = vcnil, const vc& a1 = vcnil, const vc& a2 = vcnil, const vc& a3 = vcnil, const vc& a4 = vcnil)
{
    vc res = sDb->sql_simple(sql, a0, a1, a2, a3, a4);
    return res;
}

static
vc
sql_bulk_query(const VCArglist *a)
{
    return sDb->query(a);
}

static
void
sql_start_transaction()
{
    sDb->start_transaction();
}


static
void
sql_commit_transaction()
{
    sDb->commit_transaction();
}

static
void
sql_sync_off()
{
    sDb->sync_off();
}

static
void
sql_sync_on()
{
    sDb->sync_on();

}


static
void
sql_rollback_transaction()
{
    sDb->rollback_transaction();
}

void
init_prfdb()
{
    if(sDb)
        return;
    sDb = new Sql;
    sDb->init();
    Prf_session_cache = vc(VC_SET);
    Prf_memory_cache = vc(VC_TREE);
    Pk_session_cache = vc(VC_SET);
    Pk_memory_cache = vc(VC_TREE);
    vc prf;
    if(load_profile(My_UID, prf))
    {
        vc pack;
        if(deserialize(prf[PRF_PACK], pack))
        {
            set_settings_value("user/username", pack[vc("handle")]);
            set_settings_value("user/description", pack[vc("desc")]);
            set_settings_value("user/location", pack[vc("loc")]);
            set_settings_value("user/email", pack[vc("email")]);
        }
    }

}

void
exit_prfdb()
{
    if(!sDb)
        return;
    // clean up old stuff if needed
    sDb->exit();
    delete sDb;
    sDb = 0;
    Prf_session_cache = vcnil;
    Prf_memory_cache = vcnil;
    Pk_session_cache = vcnil;
    Pk_memory_cache = vcnil;
}

static int
check_profile(vc prf)
{
    // basic sanity checking on decrypted profile
    // note: assumes the thing that is doing the
    // decrypting does sanity checks on its own encapsulation
    if(prf.type() != VC_VECTOR)
        return 0;
    if(prf[PRF_PACK].type() != VC_STRING)
        return 0;
    vc v = prf[PRF_MEDIA];
    if(!(v.is_nil() || v.type() == VC_STRING))
        return 0;
    if(prf[PRF_CHKSUM].type() != VC_STRING)
        return 0;
    if(prf[PRF_REVIEWED].type() != VC_INT)
        return 0;
    if(prf[PRF_REGULAR].type() != VC_INT)
        return 0;
    v = prf[PRF_IMAGE];
    if(!(v.is_nil() || v.type() == VC_STRING))
        return 0;
    return 1;
}

int
load_profile(vc uid, vc& prf_out)
{
    if(uid.type() != VC_STRING)
        return 0;

    if(Prf_memory_cache.find(uid, prf_out))
        return 1;

    vc huid = to_hex(uid);
    vc res = sql_simple("select pack from prf where uid = ?1", huid);
    if(res.num_elems() == 0)
        return 0;

    vc prf;
    if(!deserialize(res[0][0], prf) || !check_profile(prf))
    {
        prf_force_check(uid);
        sql_simple("delete from prf where uid = ?1", huid);
        return 0;
    }
    prf_out = prf;
    Prf_memory_cache.add_kv(uid, prf);
    if(!Session_infos.is_nil() && !Session_infos.contains(uid))
    {
        // capture the basic infos so we have some offline
        // capability even if the profile cache gets removed
        vc pack;
        if(deserialize(prf[PRF_PACK], pack))
        {
            vc v;
            v = vc(VC_VECTOR);
            v[QIR_FROM] = uid;
            v[QIR_HANDLE] = pack[vc("handle")];
            v[QIR_EMAIL] = "";
            v[QIR_USER_SPECED_ID] = "";
            v[QIR_FIRST] = "";
            v[QIR_LAST] = "";
            v[QIR_DESCRIPTION] = pack[vc("desc")];
            v[QIR_LOCATION] = pack[vc("loc")];

            Session_infos.add_kv(uid, v);
        }

    }
    return 1;
}

static
vc
blob(vc v)
{
    vc ret(VC_VECTOR);
    ret[0] = "blob";
    ret[1] = v;
    return ret;
}

int
save_profile(vc uid, vc prf)
{
    Prf_memory_cache.del(uid);
    if(!check_profile(prf))
        return 0;
    VCArglist a;
    vc huid = to_hex(uid);
    a.append("insert or replace into prf ("
             "uid,"
             "pack,"
             "media,"
             "reviewed,"
             "regular,"
             "chksum,"
             "image, "
             "time"
             ")"
             "values(?1, ?2, ?3, ?4, ?5, ?6, ?7, strftime('%s', 'now'))"
             );
    a.append(huid);
    a.append(blob(serialize(prf)));
    a.append(blob(prf[PRF_MEDIA]));
    a.append(prf[PRF_REVIEWED]);
    a.append(prf[PRF_REGULAR]);
    a.append(prf[PRF_CHKSUM]);
    a.append(blob(prf[PRF_IMAGE]));
    sql_bulk_query(&a);

    // should propagate the update out to the places where
    // other parts of the system are expecting it:
    // the *.usr/info file
    // the infos file
    // the sinfos file
    // UserConfigData
    vc pack;
    if(deserialize(prf[PRF_PACK], pack))
    {
        vc v;
        v = vc(VC_VECTOR);
        v[QIR_FROM] = uid;
        v[QIR_HANDLE] = pack[vc("handle")];
        v[QIR_EMAIL] = "";
        v[QIR_USER_SPECED_ID] = "";
        v[QIR_FIRST] = "";
        v[QIR_LAST] = "";
        v[QIR_DESCRIPTION] = pack[vc("desc")];
        v[QIR_LOCATION] = pack[vc("loc")];

        Session_infos.add_kv(uid, v);

        if(uid == My_UID)
        {
            set_settings_value("user/username", v[QIR_HANDLE]);
            set_settings_value("user/description", v[QIR_DESCRIPTION]);
            set_settings_value("user/location", v[QIR_LOCATION]);
            set_settings_value("user/email", pack[vc("email")]);
        }
        se_emit(SE_USER_UID_RESOLVED, uid);
        Profile_updated.emit(uid, 1);
    }
    return 1;
}

int
prf_already_cached(vc uid)
{
    if(uid == My_UID)
    {
        // always get your current profile from the
        // server.
        return 0;
    }
    return Prf_session_cache.contains(uid);
}

void
prf_set_cached(vc uid)
{
    Prf_session_cache.add(uid);
}

void
prf_force_check(vc uid)
{
    Prf_session_cache.del(uid);
    Prf_memory_cache.del(uid);
}

void
prf_invalidate(vc uid)
{
    prf_force_check(uid);
    sql_simple("delete from prf where uid = ?1", to_hex(uid));
    Profile_updated.emit(uid, 0);
}

// pk related stuff

static int
check_pk(vc prf)
{
    // basic sanity checking on decrypted profile
    // note: assumes the thing that is doing the
    // decrypting does sanity checks on its own encapsulation.
    // note: don't check for signature, some keys will not
    // be signed.
    if(prf.type() != VC_VECTOR)
        return 0;
    if(prf[PKC_STATIC_PUBLIC].type() != VC_STRING)
        return 0;
    if(!prf[PKC_DWYCO_SIGNATURE].is_nil() && prf[PKC_DWYCO_SIGNATURE].type() != VC_STRING)
        return 0;
    return 1;
}

static int
verify_sig(vc prf)
{
    return 1;
    // WARNING: server computes the SHA of the SERIALIZED public
    // key, not sure what i was thinking, except that maybe if the
    // public key was a composite other than a string, it might
    // still work without mods
    vc hash = vclh_sha(prf[PKC_STATIC_PUBLIC]);
    vclh_dsa_pub_init("dsadwyco.pub");
    vc ret = vclh_dsa_verify(hash, prf[PKC_DWYCO_SIGNATURE]);
    return !ret.is_nil();
}

static
int
load_pk(vc uid, vc& prf_out)
{
    if(uid.type() != VC_STRING)
        return 0;

    if(Pk_memory_cache.find(uid, prf_out))
        return 1;
    vc huid = to_hex(uid);
    // WARNING: the columns of this select are ordered by the PKC_* constants
     vc res = sql_simple("select static_public, dwyco_sig, alt_static_public, alt_server_sig, alt_gname "
                         "from pubkeys where uid = ?1", huid);
     if(res.num_elems() == 0)
         return 0;

     // note: for compat, rest of system expects "nils" instead of zero len strings.

    vc prf(VC_VECTOR);
    vc d = res[0];
    int n = d.num_elems();

    for(int i = 0; i < n; ++i)
    {
        prf.append(d[i].len() == 0 ? vcnil : d[i]);
    }

    if(!check_pk(prf) || !verify_sig(prf))
    {
        pk_force_check(uid);
        sql_simple("delete from pubkeys where uid = ?1", huid);
        return 0;
    }

    prf_out = prf;
    Pk_memory_cache.add_kv(uid, prf);

    return 1;

}

int
get_pk(vc uid, vc& sfpk_out)
{
    vc prf_out;
    if(!load_pk(uid, prf_out))
        return 0;

    vc ret(VC_VECTOR);
    ret[DH_STATIC_PUBLIC] = prf_out[PKC_STATIC_PUBLIC];

    sfpk_out = ret;
    return 1;
}

int
get_pk2(vc uid, vc& sfpk_out, vc& alt_sfpk_out, vc& alt_name)
{
    vc prf_out;
    if(!load_pk(uid, prf_out))
        return 0;

    vc ret(VC_VECTOR);
    ret[DH_STATIC_PUBLIC] = prf_out[PKC_STATIC_PUBLIC];

    sfpk_out = ret;

    vc ret2(VC_VECTOR);
    if(prf_out[PKC_ALT_STATIC_PUBLIC].is_nil())
    {
        alt_sfpk_out = vcnil;
        return 1;
    }
    ret2[DH_STATIC_PUBLIC] = prf_out[PKC_ALT_STATIC_PUBLIC];
    alt_sfpk_out = ret2;
    alt_name = prf_out[PKC_ALT_GNAME];

    return 1;
}

static
vc
blobnil(vc v)
{
    return v.is_nil() ? blob("") : blob(v);
}

static
int
save_pk(vc uid, vc prf)
{
    Pk_memory_cache.del(uid);
    vc huid = to_hex(uid);
    GRTLOG("SAVE PK", 0, 0);
    GRTLOGVC(prf);
    if(!check_pk(prf))
        return 0;

    VCArglist a;
    a.append("insert or replace into pubkeys("
             "uid, "
             "static_public, "
             "dwyco_sig, "
             "alt_static_public, "
             "alt_server_sig, "
             "alt_gname, "
             "time"
             ")"
             "values(?1, ?2, ?3, ?4, ?5, ?6, strftime('%s', 'now'))"
             );
    a.append(huid);
    a.append(blobnil(prf[PKC_STATIC_PUBLIC]));
    a.append(blobnil(prf[PKC_DWYCO_SIGNATURE]));
    a.append(blobnil(prf[PKC_ALT_STATIC_PUBLIC]));
    a.append(blobnil(prf[PKC_ALT_SERVER_SIG]));
    a.append(blobnil(prf[PKC_ALT_GNAME]));
    sql_bulk_query(&a);

    Keys_updated.emit(uid, 1);
    return 1;
}

int
put_pk(vc uid, vc sfpk, vc sig)
{
    vc pk(VC_VECTOR);
    pk[PKC_STATIC_PUBLIC] = sfpk[DH_STATIC_PUBLIC];
    pk[PKC_DWYCO_SIGNATURE] = sig;
    return save_pk(uid, pk);
}

int
put_pk2(vc uid, vc sfpk, vc sig, vc alt_pk, vc server_sig, vc gname)
{
    vc pk(VC_VECTOR);
    pk[PKC_STATIC_PUBLIC] = sfpk[DH_STATIC_PUBLIC];
    pk[PKC_DWYCO_SIGNATURE] = sig;
    pk[PKC_ALT_STATIC_PUBLIC] = alt_pk[DH_STATIC_PUBLIC];
    pk[PKC_ALT_SERVER_SIG] = server_sig;
    pk[PKC_ALT_GNAME] = gname;
    return save_pk(uid, pk);
}

int
pk_session_cached(vc uid)
{
    if(uid == My_UID)
    {
        // always get your current profile from the
        // server.
        return 0;
    }
    return Pk_session_cache.contains(uid);
}

void
pk_set_session_cache(vc uid)
{
    Pk_session_cache.add(uid);
}

void
pk_force_check(vc uid)
{
    Pk_session_cache.del(uid);
    Pk_memory_cache.del(uid);
}

void
pk_invalidate(vc uid)
{
    sql_simple("delete from pubkeys where uid = ?1", to_hex(uid));
    pk_force_check(uid);
    Keys_updated.emit(uid, 0);
}

// this is probably only useful as a fall-back, since the info could be
// stale, and we could end up asking a particular uid for a private key
// they don't have anymore. it might be useful in situations where
// you can't access the server, and the uid is on the local network.
vc
find_alt_pubkey(vc alt_name, vc& uid_out)
{
    vc res = sql_simple("select uid, alt_static_public, alt_server_sig from pubkeys where alt_gname = ?1 order by time desc");
    vclh_dsa_pub_init(newfn("dsadwyco.pub").c_str());
    for(int i = 0; i < res.num_elems(); ++i)
    {
        vc r = res[i];
        vc spk = serialize(r[1]);
        DwString a(alt_name, alt_name.len());
        a += DwString(spk, spk.len());
        vc h = vclh_sha(vc(VC_BSTRING, a.c_str(), a.length()));
        if(vclh_dsa_verify(h, r[2]).is_nil())
            continue;
        uid_out = from_hex(r[0]);
        return r[1];
    }
    return vcnil;
}



}
