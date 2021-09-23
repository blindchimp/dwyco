
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
#include "dhsetup.h"
using namespace dwyco;

static vc DH_static;
vc Server_publics;
vc Current_session_key;
vc Current_authenticator;

static const char server_pub[] =
    "\x30\x39\x30\x31\x31\x30\x32\x30\x33\x32\x35\x36\x64\xa8\x82\xdd"
    "\xd9\xbc\xba\x9a\xa9\x39\x29\xd3\x99\x90\x13\x31\x41\x16\xbb\xb9"
    "\x08\x13\x37\x6a\xfa\x2d\x94\x0e\xa8\xca\xcc\xfc\xcb\xf8\x0d\x6c"
    "\x7f\x1e\xf1\xc4\x0f\x81\x52\x9c\xfe\x48\x94\x34\xb8\x48\x4b\x8b"
    "\xfd\xee\x93\x0f\x4e\x9b\x32\xf2\xcf\x05\x87\x68\x82\xe6\x17\x3a"
    "\xbc\x10\xc1\x86\xa5\x3a\x59\x27\x6a\xc8\x0b\x76\x6a\x69\x5a\x2f"
    "\x09\xc1\xad\x56\xa5\x99\xa4\x1e\x0f\xdf\x39\x54\x74\x05\x0a\x79"
    "\x9a\x45\x46\xf1\xa8\x5c\xd3\x8e\x83\x52\xcb\xe9\x46\x30\x3c\x7d"
    "\xd0\x97\xd4\x9a\xf2\xf8\x72\x38\x93\xc8\xa3\xe8\xbb\xf1\x4f\x49"
    "\x09\xdb\x57\x13\xf6\x1b\xf5\x68\x69\xa4\x82\xd6\x30\xce\xf6\xc5"
    "\xac\x2a\x10\x2b\x9e\xb3\x16\x2a\xba\x3d\x10\xb3\x3a\x71\x3c\xff"
    "\x4f\xed\xe6\xff\x29\x9e\xee\x6e\x5e\x72\xd5\x1e\xeb\x8b\x75\xbe"
    "\x6c\x67\xd5\x43\x23\xd7\x4a\xe4\x2f\x5f\x86\x84\x79\x8b\xe9\x00"
    "\x48\x1c\xc1\x7d\xc6\x4f\xa4\x5d\xb8\xbd\x02\x0c\x44\x48\x2e\x72"
    "\x97\x64\x69\xf8\xa6\xd4\xd8\x09\x5e\x1d\x24\x93\xb6\x9e\xa2\xc8"
    "\x62\xba\x4f\x29\x8a\xc6\x38\x8b\x95\x3c\x02\xe3\x43\x15\xbe\x38"
    "\x69\xe2\x21\x50\x97\xe8\x86\x90\x55\x97\x90\x4d"
    ;

void
dh_init()
{
    udh_init(get_entropy());
    // init server publics, might want to just compile it in
    if(!load_info(Server_publics, "dwyco_dh_pub.dif"))
    {
        vcxstream vcx(server_pub, sizeof(server_pub));
        if(!vcx.open(vcxstream::READABLE))
            oopanic("can't get server public keys");
        if(Server_publics.xfer_in(vcx) < 0)
            oopanic("can't get server public keys");
        vcx.close();
    }
}

static int
dh_new_account()
{
    vc entropy = get_entropy();
    DH_static = udh_new_static(entropy);

    if(!save_info(DH_static, "dh.dif"))
        return 0;
    return 1;
}

// return 2 means account was created, and the public static info will
// have to be refreshed in the profile, etc.
int
dh_load_account(const char *acct)
{
    if(!load_info(DH_static, acct))
    {
        if(!dh_new_account())
        {
            GRTLOG("cant create DH account", 0, 0);
            return 0;
        }
        GRTLOG("created new DH account", 0, 0);
        return 2;
    }
    GRTLOG("loaded existing DH account", 0, 0);
    return 1;
}

vc
dh_my_combined_publics()
{
    if(DH_static.is_nil())
        return vcnil;
    return udh_just_publics(DH_static);
}

vc
dh_static_material(vc combined_material)
{
    // warning: the indexing between combined and
    // separate key-pairs is messed up, don't rely on
    // defines or correspondence between the indexes.
    // ugh.
    vc ret(VC_VECTOR);
    ret[DH_STATIC_PRIVATE] = combined_material[2];
    ret[DH_STATIC_PUBLIC] = combined_material[3];
    return ret;
}

vc
dh_gen_combined_keys()
{
    vc entropy = get_entropy();
    return udh_gen_keys(DH_static, entropy);
}

vc
dh_my_static()
{
    return DH_static;
}


