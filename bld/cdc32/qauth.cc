
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/qauth.cc 1.9 1999/01/10 16:09:50 dwight Checkpoint $

#include <time.h>
#ifdef _Windows
#include <io.h>
#include <fcntl.h>
#endif
#include "vc.h"
#include "vcxstrm.h"
#include "vccomp.h"
#include "qdirth.h"
#include "gvchild.h"
#include "dwtimer.h"
#include "netvid.h"
#include "qauth.h"
#include "dirth.h"
#include "pval.h"
#include "qmsg.h"
#include "asshole.h"
#include "dlli.h"
#ifdef LINUX
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "miscemu.h"
#endif
#include "dwrtlog.h"
#include "xinfo.h"
#include "dwyco_rand.h"

using namespace dwyco;

vc vclh_sha(vc);

vc My_UID;
vc My_MID;
vc My_server_key;
static vc Entropy;
int Send_auth;
int Create_new_account;
int Auth_remote;
int Entropy_charge;
void (*Entropy_display_callback)(int);

vc
to_hex(vc s)
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
from_hex(vc s)
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

void
init_entropy()
{
    if(!load_info(Entropy, "entropy") || Entropy.type() != VC_STRING)
    {
        Entropy = vclh_sha("hmmm");
        Entropy_charge = ECHARGE;
    }
    else
    {
        // give it a little more in case we crashed last
        // time, don't want to get the same numbers by accident.
        // we know it is a string, so just normalize and keep going
        Entropy = vclh_sha(Entropy);
        DWORD t = timeGetTime();
        add_entropy((char *)&t, sizeof(t));
    }
    // first time init of entropy pool...
    extern vc Myhostname;

    unsigned long t0 = DwTimer::time_now();
    time_t t1 = time(0);
    int s = dwyco_rand();
    DwString a;
    a += DwString((const char *)&s, 0, sizeof(s));
    a += DwString((const char *)&t0, 0, sizeof(t0));
    a += DwString((const char *)&t1, 0, sizeof(t1));
    // note: we know hostname won't be more than 1024, see doinit.cc
    a += DwString((const char *)Myhostname, 0, Myhostname.len());
    // try to get an IP address, but it may not work...
    // oh well, if not, then at least we get some rubbish from
    // the stack.
    struct hostent *h = gethostbyname((const char *)Myhostname);
    if(h)
    {
        a += DwString((const char *)h, 0, sizeof(*h));
        for(int i = 0; h->h_addr_list[i]; ++i)
        {
            a += DwString((const char *)h->h_addr_list[i], 0, h->h_length);
        }
    }
#ifdef _Windows
    DWORD w = GetCurrentProcessId();
    a += DwString((const char *)&w, 0, sizeof(w));
    w = GetCurrentThreadId();
    a += DwString((const char *)&w, 0, sizeof(w));
    HW_PROFILE_INFO hwp;
    GetCurrentHwProfile(&hwp);
    // if it fails, no biggie
    a += DwString((const char *)&hwp, 0, sizeof(hwp));

#endif
    // add some from /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY);
    char p[8];
    if(fd != -1)
    {
        read(fd, p, 8);
        close(fd);
    }
    // if open fails, just append whatever trash is on stack
    a += DwString(p, 0, sizeof(p));
    a += DwString((const char *)Entropy, 0, Entropy.len());
    vc s2(VC_BSTRING, a.c_str(), a.length());
    Entropy = vclh_sha(s2);
    save_info(vclh_sha(Entropy), "entropy");
}

void
save_entropy()
{
    if(Entropy.is_nil())
        init_entropy();
    save_info(vclh_sha(Entropy), "entropy");
}

vc
get_entropy()
{
    if(Entropy.is_nil())
        init_entropy();
    DWORD t = timeGetTime();
    add_entropy((char *)&t, sizeof(t));
    vc ret = vclh_sha(Entropy);
    return ret;
}

void
add_entropy(const char *astr, int alen)
{
    if(Entropy.is_nil())
        init_entropy();
    int len = Entropy.len();
    char *a = new char[len + alen];
    memcpy(a, (const char *)Entropy, len);
    memcpy(a + len, astr, alen);
    vc r(VC_BSTRING, a, len + alen);
    Entropy = vclh_sha(r);
    delete [] a;
}

void
add_entropy_timer(const char *astr, int alen)
{
    static DwTimer timer("esample");
    static DwTimer save_timer("esave");
    static int been_here;
    if(!been_here)
    {
        timer.set_autoreload(1);
        timer.set_interval(60 * 1000);
        timer.reset();
        timer.start();

        save_timer.set_autoreload(1);
        // used to be 15 seconds, but this caused too much
        // thrashing on laptops and things like dropbox
        save_timer.set_interval(2 * 3600 * 1000);
        save_timer.reset();
        save_timer.start();

        been_here = 1;
    }
    if(Entropy_charge)
    {
        --Entropy_charge;
        add_entropy(astr, alen);
        if(!Entropy_charge)
            save_info(vclh_sha(Entropy), "entropy");
        if(Entropy_display_callback)
            (*Entropy_display_callback)((int)(100 - ((double)Entropy_charge / ECHARGE) * 100));
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
        save_info(vclh_sha(Entropy), "entropy");
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


