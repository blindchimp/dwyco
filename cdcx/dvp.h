
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DVP_H
#define DVP_H

// $Header: g:/dwight/repo/cdc32/rcs/pval.h 1.10 1999/01/10 16:10:59 dwight Checkpoint $

#include <stdlib.h>
#include <QHash>

// this class provides pointer validation.
// this is used to store pointers that might be
// retrieved and used in a different context, so you want to
// be sure the interface elements associated with the
// pointer have not changed (ie, been deleted and then
// recreated with possibly the same pointer.)
//
// there might be ways to make this more type-safe... but it is more
// complicated too. once we cast to a void * to store the ptr, c++
// refuses to let us dynamic cast it back to another class type.
// no biggie, oughtta fix it, since it *is* possible now to send
// a dvp to the wrong place and have it be interpreted wrong, just like
// regular c++.

void cdcxpanic(const char *);

// pick a type that won't lose info when cast back and forth to void *
#ifdef _WIN64
typedef long long DVP_COOKIE;
#else
typedef long DVP_COOKIE;
#endif

typedef QHash<DVP_COOKIE, void *> DVP_MAP;

class DVP
{
public:
    DVP() {
        ptr = (void *)0xaaaaaaa5;
        cookie = 0x55555555;
    }
    DVP(void *p) {
        ptr = p;
        cookie = ++CookieGen;
        add_ptr(cookie, ptr);
    }


    int operator==(const DVP& v) const {
        if(ptr == v.ptr && cookie == v.cookie)
            return 1;
        return 0;
    }
    int operator!=(const DVP& v) const {
        return ! ((*this) == v);
    }
    int operator<(const DVP& ) {
        cdcxpanic("pval bad op");
        return 0;
    }
    int operator<=(const DVP& ) {
        cdcxpanic("pval bad op");
        return 0;
    }
    int operator>(const DVP& ) {
        cdcxpanic("pval bad op");
        return 0;
    }
    int operator>=(const DVP& ) {
        cdcxpanic("pval bad op");
        return 0;
    }

    operator void *() {
        if(!is_valid())
            cdcxpanic("bad ptr");
        return ptr;
    }


    int is_valid() {
        return valid_ptr(cookie);
    }
    void invalidate() {
        del_ptr(cookie);
    }

    //unsigned long hashValue() const {return ::hash(cookie);}


public:

    DVP_COOKIE cookie;
private:
    static DVP_COOKIE CookieGen;
    static void add_ptr(DVP_COOKIE p, void *);
    static int valid_ptr(DVP_COOKIE p);
    static void del_ptr(DVP_COOKIE p);
    static DVP_MAP *Ptr_listp;
    void *ptr;
public:
    static DVP cookie_to_ptr(DVP_COOKIE cookie);
    static void init_dvp();
};


#endif
