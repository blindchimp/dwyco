
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
#ifndef NO_VCEVAL
vc vcsqrt(const vc& v);
vc vcsin(const vc& v);
vc vccos(const vc& v);
vc vctan(const vc& v);
vc vcabs(const vc& v);
vc vcexp(const vc& v);
vc vclog(const vc& v);
vc vclog10(const vc& v);
vc vcpow(const vc& v, const vc& p);
vc vcacos(const vc& v);
vc vcasin(const vc& v);
vc vcatan(const vc& v);
vc vcatan2(const vc& v1, const vc& v2);
vc vcfloor(const vc& v);
vc vcatof(const vc& v);
vc vchypot(const vc& x, const vc& y);
vc vcfindmin(const vc&, const vc&);
vc vcfindmax(const vc&, const vc&);
vc vcfindeq(const vc&, const vc&, const vc&);
vc vcfindne(const vc&, const vc&, const vc&);
vc vclhsum(const vc&);
vc vclhsum2(const vc&);
vc vcrand();
vc vcsrand(const vc&);
#endif

#endif
