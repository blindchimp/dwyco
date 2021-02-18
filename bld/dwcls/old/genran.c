
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
 * $Header: g:/dwight/repo/dwcls/rcs/genran.c 1.9 1997/06/01 04:41:30 dwight Stable095 $
 */
#include <stdio.h>

main()
{
    int d = 1;
    int i;
    int nb;
    unsigned long set= 0;
    for(i = 0; i < 16; ++i)
    {
        printf("%d\n", d);
        set |= 1L << (d - 1);
        nb = (d & 1) ^ ((d >> 1) & 1);
        d >>= 1;
        d |= nb << 3;
    }
    if(set != 0xffffffff)
        printf("nope %lx\n", set);
}


