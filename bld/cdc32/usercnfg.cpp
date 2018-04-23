
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// note: this is largely defunct, keeping it for compat...
// the info is loaded from the "profile" at each program start
// and on each profile update so the info is the same.
#include "usercnfg.h"
#include "ser.h"
#include "prfcache.h"
#include "xinfo.h"
#include "fnmod.h"

extern vc My_UID;

UserConfigXfer UserConfigData;

#define USER_SECTION "user"
#define USERNAME "username"
#define DESCRIPTION "description"
#define EMAIL "email"
#define LOCATION "location"
#define DEFAULT_USERNAME ""
#define DEFAULT_DESCRIPTION ""
#define DEFAULT_EMAIL ""
#define DEFAULT_LOCATION ""

int User_info_sync;

UserConfigXfer::UserConfigXfer()
DWUIINIT_CTOR_BEGIN,
DWUIINIT_CTOR_VAL(description),
DWUIINIT_CTOR_VAL(username),
DWUIINIT_CTOR_VAL(email),
DWUIINIT_CTOR_VAL(location)
DWUIINIT_CTOR_END
{
    set_username(DEFAULT_USERNAME);
    set_description(DEFAULT_DESCRIPTION);
    set_email(DEFAULT_EMAIL);
    set_location(DEFAULT_LOCATION);
}

void
UserConfigXfer::save()
{
}

void
UserConfigXfer::load()
{
    vc prf;
    if(!load_profile(My_UID, prf))
    {
        // maybe someone deleted the cache, get it from
        // the bootstrap profile if it is there
        if(!load_info(prf, newfn("boot.dif").c_str()))
        {
            // fall back to the old user.dif, but only if
            // it hasn't already been loaded
            load_syncmap(syncmap, USER_SECTION ".dif");
            int internal_boot_file(const char *handle, int len_handle, const char *desc, int len_desc, const char *loc, int len_loc, const char *email, int len_email);
            internal_boot_file(
                get_username(), strlen(get_username()),
                get_description(), strlen(get_description()),
                get_location(), strlen(get_location()),
                get_email(), strlen(get_email())
            );

            if(!load_info(prf, newfn("boot.dif").c_str()))
                return;
        }
    }
    if(prf.type() != VC_VECTOR || prf[0].type() != VC_STRING)
        return;
    vc pack;
    if(deserialize(prf[0], pack))
    {
        set_username(pack[vc("handle")]);
        set_email(pack[vc("email")]);
        set_description(pack[vc("desc")]);
        set_location(pack[vc("loc")]);
    }
}

int
UserConfigXfer::is_saved()
{
    return 1;
}

