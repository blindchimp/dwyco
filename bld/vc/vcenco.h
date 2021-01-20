
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCENCO_H
#define VCENCO_H
// $Header: g:/dwight/repo/vc/rcs/vcenco.h 1.45 1996/11/17 05:58:34 dwight Stable $

#define ENCODED_LEN_LEN 2
#define ENCODED_TYPE_LEN 2

typedef short VCXFERTYPE;

int pltoa(long l, char *buf, int buf_len, int field_wid);
int compat_ltoa(long l, char *buf, int buf_len);
int encode_long(char *, long);
int decode_len(char *);
long decode_long(char *, int);
long decode_long2(char *buf, int len, int& error);
VCXFERTYPE decode_type(char *);
int encode_type(char *, VCXFERTYPE);



#endif
