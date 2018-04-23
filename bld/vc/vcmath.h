
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCMATH_H
#define VCMATH_H
// $Header: g:/dwight/repo/vc/rcs/vcmath.h 1.38 1998/06/17 17:21:57 dwight Exp $

vc vcsqrt(vc v);
vc vcsin(vc v);
vc vccos(vc v);
vc vctan(vc v);
vc vcabs(vc v);
vc vcexp(vc v);
vc vclog(vc v);
vc vclog10(vc v);
vc vcpow(vc v, vc p);
vc vcpow10(vc v);
vc vcacos(vc v);
vc vcasin(vc v);
vc vcatan(vc v);
vc vcatan2(vc v1, vc v2);
vc vcfloor(vc v);
vc vcatof(vc v);
vc vchypot(vc x, vc y);
vc vcfindmin(vc, vc);
vc vcfindmax(vc, vc);
vc vcfindeq(vc, vc, vc);
vc vcfindne(vc, vc, vc);
vc vclhsum(vc);
vc vclhsum2(vc);
vc vcrand();
vc vcsrand(vc);

#endif
