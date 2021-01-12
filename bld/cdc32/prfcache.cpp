
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// simple profile caching
// profile is a vector(text video auth)
//
#include "vc.h"
#include "vccrypt2.h"
#include "doinit.h"
#include "qauth.h"
#include "prfcache.h"
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
#include "dwyco_rand.h"
#include "ezset.h"

#define PROFILE_KEY "\x98\x83\xae\xf3\x60\x23\x33\xf9\x14\xd4"
#define PRF_DIR "prf2"

static char Key[] = PROFILE_KEY;
static vc enc_ctx;
static vc Prf_session_cache;
static vc Prf_memory_cache;

static
void
update_mtime(const char *fn)
{
    utime(fn, 0);
}

void
init_prf_cache()
{
    enc_ctx = vclh_bf_open();
    vclh_bf_init_key_cbc_ctx(enc_ctx, vc(VC_BSTRING, Key, sizeof(Key) - 1), 0);
    if(access(newfn(PRF_DIR).c_str(), 0) == -1)
        if(mkdir(newfn(PRF_DIR).c_str()) == -1)
            Log->make_entry("can't create prf dir");
    Prf_session_cache = vc(VC_SET);
    Prf_memory_cache = vc(VC_TREE);
}

void
exit_prf_cache()
{
    if(!enc_ctx.is_nil())
        vclh_bf_close(enc_ctx);
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
clean_profile_cache(int days_old, int maxleft)
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
        prf_force_check(uid);
        unlink(fn.c_str());
        return 0;
    }

    vc prf;
    if(bf_xfer_dec_ctx(enc_ctx, c, prf).is_nil())
    {
        prf_force_check(uid);
        unlink(fn.c_str());
        return 0;
    }
    if(!check_profile(prf))
    {
        prf_force_check(uid);
        unlink(fn.c_str());
        return 0;
    }

    prf_out = prf;
    Prf_memory_cache.add_kv(uid, prf);
    update_mtime(fn.c_str());
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

int
save_profile(vc uid, vc prf)
{

    Prf_memory_cache.del(uid);
    if(!check_profile(prf))
        return 0;

    DwString fn = prf_cache_name(uid);
    vc prfe;

    prfe = vclh_bf_xfer_enc_ctx(enc_ctx, prf);
    if(prfe.is_nil())
        return 0;
    // note: prf_cache_name added any prefix already
    if(!save_info(prfe, fn.c_str(), 1))
        return 0;
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
    }
    return 1;
}

int
prf_already_cached(vc uid)
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
    DwString fn = prf_cache_name(uid);
    DeleteFile(fn.c_str());
    prf_force_check(uid);
}
