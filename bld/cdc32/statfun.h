
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/* $Header: g:/dwight/repo/cdc32/rcs/statfun.h 1.7 1995/10/08 05:55:49 dwight Stable095 $ */
#ifndef STATFUN_H
#define STATFUN_H

double stdev(double N, double sumx, double sumx_squared);
double variance(double N, double sumx, double sumx_squared);
int log2(int);

#endif
