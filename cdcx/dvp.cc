
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/pval.cc 1.10 1999/01/10 16:09:48 dwight Checkpoint $
#include "dvp.h"
#include <stdio.h>

// note: static objects with these things in
// them could cause you problems.
DVP_MAP *DVP::Ptr_listp;

void
DVP::init_dvp()
{
    Ptr_listp = new DVP_MAP;
}
#define Ptr_list (*Ptr_listp)


DVP_COOKIE DVP::CookieGen;
void
DVP::add_ptr(DVP_COOKIE p, void *v)
{
    if(Ptr_list.contains(p))
        cdcxpanic("bogus ptr add");
    Ptr_list.insert(p, v);
    //printf("a dvpsize %d %ld\n", Ptr_list.count(), p);
}


int
DVP::valid_ptr(DVP_COOKIE p)
{
    return Ptr_list.contains(p);
}

void
DVP::del_ptr(DVP_COOKIE p)
{
    Ptr_list.remove(p);
    //printf("d dvpsize %d %ld\n", Ptr_list.count(), p);
}

DVP
DVP::cookie_to_ptr(DVP_COOKIE cookie)
{
    if(!Ptr_list.contains(cookie))
        return DVP();

    DVP p;
    p.cookie = cookie;
    void *v = Ptr_list.value(cookie);
    p.ptr = v;
    return p;
}
