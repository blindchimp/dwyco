
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/* $Header: g:/dwight/repo/cdc32/rcs/sqrs.h 1.10 1996/06/28 10:33:00 dwight Stable095 $ */
#ifndef SQRS_H
#define SQRS_H
#define NSQRS (1 << 11)
extern long sqrs[];
extern long sqrs2[];
extern long *sqrs_abs;
extern int mo_abs[];
extern int mo_abs2[];
extern int *mo_abs_ptr;

void gen_sqr();
void gen_abs();

#endif

