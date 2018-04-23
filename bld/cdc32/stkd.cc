
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cppleak/rcs/bcci86.cc 1.5 1998/06/14 07:33:54 dwight Exp $
/* hack to output a binary stack trace */
#pragma option -rd
#pragma option -Od
#ifdef _Windows
#ifdef __BORLANDC__
#include <mem.h>
#endif
#ifdef _MSC_VER
#include <memory.h>
#endif
#else
#include <string.h>
#endif

void
stackdump(void *vec, int len)
{
    void *fp = &fp;
    void *p = (void *)&fp;
    memcpy(vec, p, len);

}

#undef TEST
#ifdef TEST
#include <stdio.h>
int vec[20];
int
bar(int)
{
    stackdump((void*)vec, sizeof(vec));
}

int
foo(int)
{
    bar(2);
}

main()
{
    int i;
    foo(1);
    for(i = 0; i < 20; ++i)
        printf("%x\n", vec[i]);
    bar(2);
    printf("\n");
    for(i = 0; i < 20; ++i)
        printf("%x\n", vec[i]);
    //stacktrace(vec);

}
#endif
