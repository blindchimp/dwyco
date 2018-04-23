
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <stdlib.h>
#include "sqrs.h"

long sqrs[NSQRS];
long sqrs2[2*NSQRS];
long *sqrs_abs;


void
gen_sqr()
{
    long i;
    for(i = 0; i < NSQRS; ++i)
        sqrs[i] = i * i;
    for(i = 0; i < 2 * NSQRS; ++i)
        sqrs2[i] = (i - NSQRS) * (i - NSQRS);
    sqrs_abs = &sqrs2[NSQRS];
}

int mo_abs[256];
int mo_abs2[512];
int *mo_abs_ptr;
void
gen_abs()
{
    int i;
    int *moa;
    for(i = -128; i < 128; ++i)
    {
        mo_abs[i & 0xff] = (abs(i) & 0x7f);
    }
    moa = &mo_abs2[256];
    for(i = -256; i < 256; ++i)
    {
        moa[i] = abs(i);
    }
    mo_abs_ptr = moa;

}

#ifdef TEST_SQRS
#include <stdio.h>
main()
{
    gen_abs();
    for(int i = 0; i < 256; ++i)
        printf("abs(%d) = %d\n", i, mo_abs[i]);
}
#endif
