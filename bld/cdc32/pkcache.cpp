
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vc.h"
#include "vccrypt2.h"
#include "doinit.h"
#include "qauth.h"
#include "pkcache.h"
#include "dwstr.h"
#ifdef _Windows
#include <io.h>
#ifdef _MSC_VER
#include <direct.h>
#include <sys/utime.h>
#endif
#include <sys\stat.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#endif
#include "qmsg.h"
#include "dwlog.h"
#ifdef LINUX
#include "miscemu.h"
#endif
#include "sepstr.h"
#include "se.h"
#include "ser.h"
#include "qdirth.h"
#include "fnmod.h"
#include "xinfo.h"
#include "vcudh.h"
#include "dwrtlog.h"
#include "dwyco_rand.h"

#define DWYCO_NO_PRF_ENC

#define PROFILE_KEY "\x66\x43\xe6\x63\xe8\x4e\x8f\x3d\x08\xf5\xbe\x65\xd0\x42\xf7\x51"
#define PRF_DIR "pk"

static char Key[] = PROFILE_KEY;
static vc enc_ctx;
// session cache is used in order to avoid querying the server
// over and over for keys that may not exist.
static vc Prf_session_cache;
static vc Prf_memory_cache;

static
void
update_mtime(const char *fn)
{
    utime(fn, 0);
}

void
init_pk_cache()
{
    enc_ctx = vclh_encdec_open();
    vclh_encdec_init_key_ctx(enc_ctx, vc(VC_BSTRING, Key, sizeof(Key) - 1), 0);
    if(access(newfn(PRF_DIR).c_str(), 0) == -1)
        if(mkdir(newfn(PRF_DIR).c_str()) == -1)
            Log->make_entry("can't create prf dir");
    Prf_session_cache = vc(VC_SET);
    Prf_memory_cache = vc(VC_TREE);
}

void
exit_pk_cache()
{
    if(!enc_ctx.is_nil())
        vclh_encdec_close(enc_ctx);
    enc_ctx = vcnil;
    Prf_session_cache = vcnil;
    Prf_memory_cache = vcnil;
}

static DwString
prf_cache_name(vc uid)
{
    DwString ret = PRF_DIR;
    ret = newfn(ret);
    ret += DIRSEPSTR;
    ret += (const char *)to_hex(uid);

    return ret;
}


void
clean_pk_cache(int days_old, int maxleft)
{
    DwString pat(newfn(PRF_DIR));
    pat += DIRSEPSTR "????????????????????";

    FindVec& f = *find_to_vec(pat.c_str());
    time_t now = time(0);
    long old = days_old * 3600L * 24;
    int n = f.num_elems();
    int i;
    int deleted = 0;
    // we do it once for mtime too, since atime isn't
    // always reliable.
    int left = n - deleted;
    DwString pfn(newfn(PRF_DIR));
    pfn += DIRSEPSTR;
    if(left > maxleft)
    {
        for(i = 0; i < n; ++i)
        {
            WIN32_FIND_DATA& d = *f[i];

            DwString fn = pfn;
            fn += d.cFileName;
            struct stat s;
            if(stat(fn.c_str(), &s) == -1)
                continue; // might want to delete it
            if(now - s.st_mtime >= old)
            {
                if(DeleteFile(fn.c_str()))
                    ++deleted;
                delete f[i];
                f[i] = 0;
            }
        }
    }

    left = n - deleted;
    if(left > maxleft)
    {
        // just delete the first so many until we
        // have only maxleft.
        // this is done because the atime and mtime
        // may not work right on various ms filesystems
        for(i = 0; i < n && left > maxleft; ++i)
        {
            if(!f[i])
                continue;
            WIN32_FIND_DATA& d = *f[i];

            DwString fn = pfn;
            fn += d.cFileName;
            // the rand is so we don't always end up trimming off
            // the last few files if the findvec comes back the
            // same order all the time
            if((dwyco_rand() & 1) && DeleteFile(fn.c_str()))
                --left;
        }

    }
    delete_findvec(&f);
}

static int
check_profile(vc prf)
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

static int
load_pk(vc uid, vc& prf_out)
{
    if(uid.type() != VC_STRING)
        return 0;

    if(Prf_memory_cache.find(uid, prf_out))
        return 1;

    DwString fn = prf_cache_name(uid);

    if(access(fn.c_str(), 0) == -1)
        return 0;

    // note: "remove" changed in linux from basically
    // an alias for "unlink" to "remove files *and* directories"
    // which messed us up when bogus empty uid's were sent in here.
    // on windows, this change didn't happen.
    vc c;
    // note: prf_cache_name added any prefix already
    if(!load_info(c, fn.c_str(), 1))
    {
        pk_force_check(uid);
        unlink(fn.c_str());
        return 0;
    }

    vc prf;
#ifdef DWYCO_NO_PRF_ENC
    prf = c;
#else
    if(encdec_xfer_dec_ctx(enc_ctx, c, prf).is_nil())
    {
        pk_force_check(uid);
        unlink(fn.c_str());
        return 0;
    }
#endif

    if(!check_profile(prf) || !verify_sig(prf))
    {
        pk_force_check(uid);
        unlink(fn.c_str());
        return 0;
    }

    prf_out = prf;
    Prf_memory_cache.add_kv(uid, prf);
    update_mtime(fn.c_str());
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

static int
save_pk(vc uid, vc prf)
{

    Prf_memory_cache.del(uid);

    DwString fn = prf_cache_name(uid);
    // note: really need some defines for the format of the
    // key vector returned by vcudh stuff...
    // do this to avoid saving out anything other than the public
    // static key

    GRTLOG("SAVE PK", 0, 0);
    GRTLOGVC(prf);
    if(!check_profile(prf))
        return 0;
    vc prfe;

#ifdef DWYCO_NO_PRF_ENC
    prfe = prf;
#else

    prfe = vclh_encdec_xfer_enc_ctx(enc_ctx, prf);
#endif
    if(prfe.is_nil())
        return 0;
    // note: prf_cache_name added any prefix already
    if(!save_info(prfe, fn.c_str(), 1))
        return 0;
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
    extern vc My_UID;
    if(uid == My_UID)
    {
        // always get your current profile from the
        // server.
        return 0;
    }
    return Prf_session_cache.contains(uid);
}

void
pk_set_session_cache(vc uid)
{
    Prf_session_cache.add(uid);
}

void
pk_force_check(vc uid)
{
    Prf_session_cache.del(uid);
    Prf_memory_cache.del(uid);
}

void
pk_invalidate(vc uid)
{
    DwString fn = prf_cache_name(uid);
    DeleteFile(fn.c_str());
    pk_force_check(uid);
}
