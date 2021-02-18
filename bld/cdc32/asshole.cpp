#ifdef DWYCO_ASSHAT

/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "qauth.h"
#include "asshole.h"
#include "xinfo.h"

#include <math.h>

static vc Asshole_factor(0.0);
static vc Asshole_threshold;
// 2 asshole sets because one is dynamically updated
// more frequently, and if there is an asshole factor
// there, that is the one that should be used.
// otherwise, use the one from the directory.
static vc Assholes;
// this should come down from the server
#define AssHalfLifeDefault (-1.0/(7 * 24 * 3600))
//#define AssHalfLifeDefault (-1.0/20)
static double AssHalfLife;
int Asshole_filtering = ASSHOLE_FILTERING_NONE;

void
init_assholes()
{
    Assholes = vc(VC_TREE);
    set_asshole_thresh(0.0);
    AssHalfLife = AssHalfLifeDefault;
}

// only call this if you know the time the
// asshole was sampled is close to the current time.
void
new_asshole(vc uid, vc ah)
{
    if(Assholes.is_nil())
        return;
    vc v(VC_VECTOR);
    v[0] = ah;
    v[1] = (long)time(0);
    Assholes.add_kv(uid, v);
}

void
new_asshole2(vc uid, vc ah, vc tm)
{
    vc v(VC_VECTOR);
    v[0] = ah;
    v[1] = tm;
    Assholes.add_kv(uid, v);
}

void
set_asshole_param(vc p)
{
    if(p.is_nil())
        AssHalfLife = AssHalfLifeDefault;
    else
        AssHalfLife = p;
}

int
is_asshole(vc uid)
{
    vc ah;
    if(!Assholes.find(uid, ah))
        return -1;
    if(get_decayed_asshole(uid) >= Asshole_threshold)
        return 1;
    return 0;
}

vc
get_asshole(vc uid, time_t& tm)
{
    vc ah;
    if(!Assholes.find(uid, ah))
    {
        return vcnil;
    }
    tm = (long)ah[1];
    return ah[0];
}


void
update_asshole_factor(vc ah)
{
    save_info(ah, "ah.dif");
}

void
load_asshole_factor()
{
    if(!load_info(Asshole_factor, "ah.dif"))
        update_asshole_factor(0.0);
}

vc
get_asshole_factor()
{
    return Asshole_factor;
}

vc
get_asshole_thresh()
{
    return Asshole_threshold;
}

void
set_asshole_thresh(vc ah)
{
    Asshole_threshold = ah;
}

vc
get_decayed_asshole(vc uid)
{
    time_t tm0;
    vc ah = get_asshole(uid, tm0);
    if(ah.is_nil())
        return vcnil;
    time_t now = time(0);
    double e = AssHalfLife * (now - tm0);
    // keep exp from underflowing, etc.
    if(e > 0 || e < -20)
        return 0;
    double d = (double)ah * exp(e);
    return d;

}

// this function *must* return a 2 character string
DwString
display_asshole(vc uid)
{
    vc ah1;
    double ah;
    ah1 = get_decayed_asshole(uid);
    if(ah1.is_nil())
        return "??";
    ah = ah1;
    if(ah >= 10.0)
        ah = 9.999;
    ah = 9.999 - ah;
    ah *= 10.0;
    // subtract .5 to counteract rounding done in
    // sprintf
    ah -= 0.5;
    char a[10];
    sprintf(a, "%02.0f", ah);
    return DwString(a);
}
#endif
