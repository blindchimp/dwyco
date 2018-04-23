
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "dwquerybymember.h"



#undef TEST
#ifdef TEST
// g++ -I/usr/include/qt4/QtCore -I/usr/include/qt4 -I/home/dwight/depot/dwycore2/ins/dbg/inc dwquerybymember.cpp -lQtCore /home/dwight/depot/dwycore2/ins/dbg/lib/dwcls.a
#include <stdio.h>
#include "dwstr.h"
struct foo
{
    DwOString baz;
    int bar;

    int operator==(const foo& k) {return baz == k.baz && bar == k.bar;}

};
DwQueryByMember<foo> qbm;

void
cdcxpanic(const char *s)
{
    ::abort();
}

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
    QList<foo *> ret;
    ret = qbm.query_by_member(DwOString("a"), &foo::baz);

    printf("%d\n", ret.count());

    ret = qbm.query_by_member(5, &foo::bar);

    printf("%d\n", ret.count());

    ret = qbm.query_by_member(3, &foo::bar);

    printf("%d\n", ret.count());
    }

#if 0
    {

    QList<foo *> ret;
    ret = qbm.query_by_member(DwOString("a"), &foo::baz);

    printf("%d\n", ret.count());

    ret = qbm.query_by_member(5, &foo::bar);

    printf("%d\n", ret.count());

    ret = qbm.query_by_member(3, &foo::bar);

    printf("%d\n", ret.count());
    }
#endif

}

#endif
