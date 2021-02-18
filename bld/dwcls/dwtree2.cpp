
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/

#define DICT_IMPLEMENTATION
#include "dict.h"

dnode_t *
dwtreekaz_alloc_node(void *)
{
    return new dnode_t;
}

