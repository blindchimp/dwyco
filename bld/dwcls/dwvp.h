
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWVP_H
#define DWVP_H

// $Header: g:/dwight/repo/cdc32/rcs/pval.h 1.10 1999/01/10 16:10:59 dwight Checkpoint $

#include <stdlib.h>
#include "dwhash.h"
#include "dwtree2.h"

// this class provides pointer validation.
// this is used to store pointers that might be
// retrieved and used at a later time, so you want to
// be sure the interface elements associated with the
// pointer have not changed (ie, been deleted and then
// recreated with possibly the same pointer.
//
[[noreturn]] void oopanic(const char *);


class DwVP
{
public:
    DwVP() {
        ptr = (void *)0xaaaaaaa5;
        cookie = 0x55555555;
    }
    DwVP(void *p) {
        ptr = p;
        cookie = ++CookieGen;
        add_ptr(cookie, ptr);
    }

    int operator==(const DwVP& v) const {
        if(ptr == v.ptr && cookie == v.cookie)
            return 1;
        return 0;
    }
    int operator!=(const DwVP& v) const {
        return ! ((*this) == v);
    }
    int operator<(const DwVP& ) {
        oopanic("pval bad op");
        return 0;
    }
    int operator<=(const DwVP& ) {
        oopanic("pval bad op");
        return 0;
    }
    int operator>(const DwVP& ) {
        oopanic("pval bad op");
        return 0;
    }
    int operator>=(const DwVP& ) {
        oopanic("pval bad op");
        return 0;
    }

    operator void *() {
        if(!is_valid())
            oopanic("bad ptr");
        return ptr;
    }

    void *get_ptr() {
        if(!is_valid())
            oopanic("bad ptr");
        return ptr;
    }
#if 0
    operator long() {
        if(!is_valid())
            oopanic("bad cookie");
        return cookie;
    }
#endif

    int is_valid() {
        return valid_ptr(cookie);
    }
    void invalidate() {
        del_ptr(cookie);
    }

    unsigned long hashValue() const {
        return ::hash(cookie);
    }


public:
    long cookie;
private:
    void *ptr;
    static long CookieGen;
    static void add_ptr(long p, void *);
    static int valid_ptr(long p);
    static void del_ptr(long p);
    static DwTreeKaz<void *, long> *Ptr_listp;
public:
    static DwVP cookie_to_ptr(long cookie);
    static void init_dvp();
};


#endif
