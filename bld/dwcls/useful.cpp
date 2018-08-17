
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
 * $Header: g:/dwight/repo/dwcls/rcs/useful.cpp 1.1 1997/07/05 02:50:23 dwight Stable095 $
 */

#include "useful.h"
class DwAllocator;
DwAllocator *Default_alloc;

void
vpnull(void*& v)
{
    v = 0;
}
