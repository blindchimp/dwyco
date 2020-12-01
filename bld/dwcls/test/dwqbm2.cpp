
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#include "dwqbm2.h"


#define TEST
#ifdef TEST
using namespace dwyco;
// g++ dwqbm.cpp dbg/dwcls.a
#include <stdio.h>
#include "dwstr.h"
struct foo
{
    DwString baz;
    int bar;

    int operator==(const foo& k) {
        return baz == k.baz && bar == k.bar;
    }
    int qfun() {
        return 1;
    }

};

DwQueryByMember2<foo, DwString> qbm;

void
oopanic(const char *s)
{
    ::abort();
}

int
main(int, char **)
{
    foo a;
    foo b;
    a.baz = "a";
    a.bar = 4;
    b.baz = "b";
    b.bar = 5;



    qbm.add(&a);
    qbm.add(&b);

    {
        DwVecP<foo> ret;
        ret = qbm.query_by_member(DwString("a"), &foo::baz);

        printf("%d\n", ret.num_elems());

        ret = qbm.query_by_member(5, &foo::bar);

        printf("%d\n", ret.num_elems());

        ret = qbm.query_by_member(3, &foo::bar);

        printf("%d\n", ret.num_elems());

        //printf("%d\n", qbm.exists_by_fun(&foo::qfun, 1));
        //printf("%d\n", qbm.exists_by_fun(&foo::qfun, 0));
    }

#if 0
    {

        QList<foo *> ret;
        ret = qbm.query_by_member(DwString("a"), &foo::baz);

        printf("%d\n", ret.num_elems());

        ret = qbm.query_by_member(5, &foo::bar);

        printf("%d\n", ret.num_elems());

        ret = qbm.query_by_member(3, &foo::bar);

        printf("%d\n", ret.num_elems());
    }
#endif

}

#endif
