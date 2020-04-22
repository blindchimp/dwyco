
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/

/*
 * $Header: g:/dwight/repo/dwcls/rcs/gcd.cpp 1.10 1997/06/01 04:40:07 dwight Stable095 $
 */
#include <stdlib.h>
#include "gcd.h"

long
gcd(long x, long y)        // euclid's algorithm
{
    long a = labs(x);
    long b = labs(y);

    long tmp;

    if (b > a)
    {
        tmp = a;
        a = b;
        b = tmp;
    }
    for(;;)
    {
        if (b == 0)
            return a;
        else if (b == 1)
            return b;
        else
        {
            tmp = b;
            b = a % b;
            a = tmp;
        }
    }
}
