
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#include "dwhash.h"

unsigned long
hash(int a)
{
    return a;
}

unsigned long
hash(long a)
{
    return a;
}

unsigned long
hash(double d)
{
    return (unsigned long)d;
}

unsigned long
hash(void *d)
{
    return (unsigned long)d;
}
