
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
// $Header: g:/dwight/repo/cdc32/rcs/pval.cc 1.10 1999/01/10 16:09:48 dwight Checkpoint $
#include "dwtree2.h"
#include "dwvp.h"

// note: static objects with these things in
// them could cause you problems.
DwTreeKaz<void *, long> *DwVP::Ptr_listp;

void
DwVP::init_dvp()
{
    Ptr_listp = new DwTreeKaz<void *, long>(0);
}
#define Ptr_list (*Ptr_listp)


long DwVP::CookieGen = 2000;
void
DwVP::add_ptr(long p, void *v)
{
    if(Ptr_list.exists(p))
        oopanic("bogus ptr add");
    Ptr_list.add(p, v);
//GRTLOG("vpsize %d", Ptr_list.num_elems(), 0);
}


int
DwVP::valid_ptr(long p)
{
    return Ptr_list.exists(p);
}

void
DwVP::del_ptr(long p)
{
    Ptr_list.del(p);
}

DwVP
DwVP::cookie_to_ptr(long cookie)
{
    DwVP p;
    p.cookie = cookie;
    void *v;
    if(!Ptr_list.find(cookie, v))
        return DwVP();
    p.ptr = v;
    return p;
}
