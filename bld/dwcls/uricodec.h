
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef URICODEC_H
#define URICODEC_H
#include "dwstr.h"

DwString UriDecode(const DwString &sSrc);
DwString UriEncode(const DwString &sSrc);
#endif
