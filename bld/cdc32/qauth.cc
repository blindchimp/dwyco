
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/qauth.cc 1.9 1999/01/10 16:09:50 dwight Checkpoint $

#include <time.h>
#include "vc.h"
#include "dwtimer.h"
#include "qauth.h"
#include "asshole.h"
#include "dwrtlog.h"
#include "xinfo.h"
#include "dwyco_rand.h"
#include "vcrng.h"

using namespace dwyco;

vc vclh_sha(vc);

namespace dwyco {

vc My_UID;
vc My_MID;
vc My_server_key;
extern vc Myhostname;
static CryptoPP::RandomPool *Entropy_rng;
int Send_auth;
int Create_new_account;
int Auth_remote;
void (*Entropy_display_callback)(int);

vc
to_hex(const vc& s)
{
    const char *a = (const char *)s;
    int len = s.len();
    char *d = new char[len * 2 + 1];
    for(int i = 0; i < len; ++i)
    {
        sprintf(&d[2 * i], "%02x", a[i] & 0xff);
    }
    vc r(VC_BSTRING, d, (long)len * 2);
    delete [] d;
    return r;
}

static int
hexd(char a)
{
    if(a >= '0' && a <= '9')
        return a - '0';
    if(a >= 'a' && a <= 'f')
        return a - 'a' + 10;
    if(a >= 'A' && a <= 'F')
        return a - 'A' + 10;
    return 0;
}

vc
from_hex(const vc& s)
{
    const char *a = (const char *)s;
    int len = s.len();
    char *d = new char[len / 2];
    for(int i = 0; i < len / 2; ++i)
    {
        d[i] = (hexd(a[i * 2]) << 4) | hexd(a[i * 2 + 1]);
    }
    vc r(VC_BSTRING, d, (long)len / 2);
    delete [] d;
    return r;
}

static vc
gen_salt()
{
    vc rn = get_entropy();
    vc id(VC_BSTRING, (const char *)rn, 8);
    return id;
}

vc
gen_pass(vc phrase, vc& salt)
{

// salt the password, hash it, and return
// the hash as the password

    if(salt.is_nil())
        salt = gen_salt();
    DwString a((const char *)phrase, 0, phrase.len());
    a += DwString((const char *)salt, 0, salt.len());
    vc v(VC_BSTRING, a.c_str(), a.length());
    vc hash = vclh_sha(v);
    return hash;
}

static void
mix_startup_noise(CryptoPP::RandomPool *rng)
{
    unsigned long t0 = DwTimer::time_now();
    time_t t1 = time(0);
    int s = dwyco_rand();
    rng->IncorporateEntropy((const CryptoPP::byte *)&t0, sizeof(t0));
    rng->IncorporateEntropy((const CryptoPP::byte *)&t1, sizeof(t1));
    rng->IncorporateEntropy((const CryptoPP::byte *)&s, sizeof(s));
    // note: we know hostname won't be more than 1024, see doinit.cc
    if(Myhostname.type() == VC_STRING && Myhostname.len() > 0)
        rng->IncorporateEntropy((const CryptoPP::byte *)(const char *)Myhostname,
                                Myhostname.len());
}

void
init_entropy()
{
    if(Entropy_rng)
        return;

    vc seed;
    int have_seed = load_info(seed, "entropy") && seed.type() == VC_STRING;

    if(have_seed)
        Entropy_rng = make_rng((const CryptoPP::byte *)(const char *)seed, seed.len());
    else
        Entropy_rng = make_rng();

    mix_startup_noise(Entropy_rng);
    save_entropy();
}

void
save_entropy()
{
    if(!Entropy_rng)
        init_entropy();

    CryptoPP::byte seed[32];
    Entropy_rng->GenerateBlock(seed, sizeof(seed));
    vc s(VC_BSTRING, (const char *)seed, sizeof(seed));
    memset(seed, 0, sizeof(seed));
    save_info(s, "entropy");
}

vc
get_entropy()
{
    if(!Entropy_rng)
        init_entropy();

    CryptoPP::byte buf[20];
    Entropy_rng->GenerateBlock(buf, sizeof(buf));
    vc ret(VC_BSTRING, (const char *)buf, sizeof(buf));
    memset(buf, 0, sizeof(buf));
    return ret;
}

void
add_entropy(const char *astr, int alen)
{
    if(alen <= 0 || !astr)
        return;
    if(!Entropy_rng)
        init_entropy();
    rng_add(Entropy_rng, astr, alen);
}

void
add_entropy_timer(const char *astr, int alen)
{
    static DwTimer timer("esample");
    static DwTimer save_timer("esave");
    static int been_here;
    if(!been_here)
    {
        // take some noise from the first event immediately, then
        // rate-limit further mixing so a busy event stream does not
        // spend all its time stirring the pool.
        been_here = 1;
        add_entropy(astr, alen);
        timer.start(DwTimer::REPEATING, 60 * 1000, 60 * 1000);

        // used to be 15 seconds, but this caused too much
        // thrashing on laptops and things like dropbox
        save_timer.start(DwTimer::REPEATING, 2 * 3600 * 1000, 2 * 3600 * 1000);
        return;
    }
    if(timer.is_expired())
    {
        timer.ack_expire();
        add_entropy(astr, alen);
    }
    if(save_timer.is_expired())
    {
        save_timer.ack_expire();
        save_entropy();
    }
}

vc
gen_id()
{
    vc rn = get_entropy();
#if 0
// start test hack, 0 out middle of id to test to make sure
// functions are always using length and not 0 termination on ids.
    char a[10];
    memcpy(a, (const char *)rn, 10);
    a[5] = 0;
    rn = vc(VC_BSTRING, a, 10);
// end hack
#endif
    vc id(VC_BSTRING, (const char *)rn, 10);
    return id;
}

int
gen_random_int()
{
    vc rn = get_entropy();
    int ret;
    memcpy((void *)&ret, (const char *)rn, sizeof(ret));
    return ret;
}

long long
gen_random_long_long()
{
    vc rn = get_entropy();
    long long ret;
    memcpy((void *)&ret, (const char *)rn, sizeof(ret));
    return ret;
}

DwString
gen_random_filename()
{
    vc fn = gen_id();
    fn = to_hex(fn);
    DwString ret((const char *)fn, 0, fn.len());
    return ret;
}

static vc
gen_ck(vc id)
{
    DwString s("\xf5\xa3\xc9\x95\x80\xd1\x66\x7f\x26\xa7", 0, 10);
    DwString s2((const char *)id, 0, id.len());
    s += s2;
    vc c = vclh_sha(vc(VC_BSTRING, s.c_str(), s.length()));
    s = DwString((const char *)c, 0, 12);
    return vc(VC_BSTRING, s.c_str(), s.length());
}

static void
add_ck(vc v, vc id)
{
    vc ck = gen_ck(id);
    v.add_kv("ck", ck);
}

static int
ck_ck(vc id, vc ck)
{
    if(gen_ck(id) != ck)
        return 0;
    return 1;
}


void
save_auth_info(vc id, vc server_key, const char *filename)
{
    vc v(VC_MAP, "", 8);
    v.add_kv("pw", "");
    v.add_kv("salt", "");
    v.add_kv("id", id);
    v.add_kv("local_pw", "");
    v.add_kv("local_salt", "");
    v.add_kv("sk", server_key);
    add_ck(v, id);
    save_info(v, filename);
}

int
load_auth_info(vc& id, vc& server_key, const char *filename)
{
    id = vcnil;
    server_key = vcnil;

    vc v;
    vc ck;

    int r = load_info(v, filename);
    if(!r)
        goto err;

    if(!v.find("id", id))
        goto err;

// don't add ck anymore if we don't find it... virtually everyone should
// be upgraded now.
    if(!v.find("ck", ck))
    {
        goto err;
    }
    else
    {
        if(!ck_ck(id, ck))
            goto err;
    }
    if(!v.find("sk", server_key))
    {
        // upgrade from old pre 2.9 auth file
        server_key = gen_id();
        v.add_kv("sk", server_key);
        save_info(v, filename);
    }
    goto ok;
err:
    id = vcnil;
    server_key = vcnil;
    return 0;

ok:
    return 1;
}



void
init_qauth()
{
    vc id;
    vc server_key;

    My_UID = vcnil;
    My_server_key = vcnil;
    Auth_remote = 0;
    Send_auth = 0;

    if(!load_auth_info(id, server_key, "auth"))
    {
        Create_new_account = 1;
        return;
    }
    else
    {
        My_UID = id;
        My_server_key = server_key;
    }
    Send_auth = 1;
    GRTLOG("old account", 0, 0);
}

// call this to check if there is an auth file (ie, an existing
// account). this can be called without doing an "init" on the entire
// system.
int
qauth_check_account_exists()
{
    vc dum1;
    vc dum2;
    return(load_auth_info(dum1, dum2, "auth"));
}

}


