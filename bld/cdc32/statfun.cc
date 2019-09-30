
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
static char Rcsid[] = "$Header: g:/dwight/repo/cdc32/rcs/statfun.cc 1.12 1997/05/18 05:16:59 dwight Stable095 $";
#include <math.h>
#include "statfun.h"

double
variance(double N, double sumx, double sumx_squared)
{
    double t = sumx / N;
    double d = sumx_squared / N - (t * t);
    if(d < 0.)
        return 0;
    return d;
}

double
stdev(double N, double sumx, double sumx_squared)
{
    double d = sumx_squared / N - ((sumx / N) * (sumx / N));
    if(d < 0.)
        return 0.00001;
    return sqrt(d);
}

#if 0
int
_matherr(struct _exception *e)
{
    return 0;
}
#endif

#if defined(MACOSX) || defined(_MSC_VER) || defined(ANDROID)
int
ffs(int i)
{
    int j = 1;
    do
    {
        if(i & 1)
            return j;
        i >>= 1;
        ++j;
    } while(i != 0);
    return -1;
}
#else
#include <strings.h>
#endif

int
log2(int i)
{
    return ffs(i) - 1;
}

