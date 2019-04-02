#ifndef NO_VCEVAL
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <math.h>
#include "vc.h"
#include "vcmath.h"
#include "vcmap.h"

//static char Rcsid[]="$Header: g:/dwight/repo/vc/rcs/vcmath.cpp 1.39 1998/06/17 17:21:57 dwight Exp $";

// this is just here for a quicky at the moment, these
// should really be percolated to the numeric classes
// and accoutred with proper error handling.

vc
vcsqrt(vc v)
{
    return sqrt((double)v);
}

vc
vcsin(vc v)
{
    return sin((double)v);
}

vc
vccos(vc v)
{
    return cos((double)v);
}

vc
vctan(vc v)
{
    return tan((double)v);
}

vc
vcabs(vc v)
{
    return fabs((double)v);
}

vc
vcexp(vc v)
{
    return exp((double)v);
}

vc
vclog(vc v)
{
    return log((double)v);
}

vc
vclog10(vc v)
{
    return log10((double)v);
}

vc
vcpow(vc v, vc p)
{
	return pow((double)v, (double)p);
}

#if !defined(__GNUG__) && ((defined(__BORLANDC__) && __BORLANDC__ < 0x540) || defined(MSDOS))
vc
vcpow10(vc v)
{
	return pow10(v);
}
#endif

vc
vcacos(vc v)
{
    return acos((double)v);
}

vc
vcasin(vc v)
{
    return asin((double)v);
}

vc
vcatan(vc v)
{
    return atan((double)v);
}

vc
vcatan2(vc v1, vc v2)
{
    return atan2((double)v1, (double)v2);
}

vc
vcfloor(vc v)
{
    return floor((double)v);
}

vc
vcatof(vc v)
{
	return atof(v);
}

vc
vchypot(vc x, vc y)
{
    return hypot((double)x, (double)y);
}

vc
vcrand()
{
	return rand();
}

vc
vcsrand(vc v)
{
	srand((int)v);
	return vcnil;
}

#define fmm(funname, name, op, termsearch) \
vc \
vc##funname(vc vec, vc index) \
{ \
	if(vec.type() != VC_VECTOR) \
	{ \
		USER_BOMB("first arg to " name " must be vector", vcnil); \
	} \
	if(!index.is_nil() && index.type() != VC_STRING) \
	{ \
		USER_BOMB("second arg to " name " is either nil or string", vcnil); \
	} \
	int n = vec.num_elems(); \
	if(n == 0) \
		return vcnil; \
	vc minvc = vec[0]; \
	int idx = 0; \
	for(int i = 1; i < n; ++i) \
	{ \
		if(vec[i] op minvc) \
		{ \
			minvc = vec[i]; \
			idx = i; \
			termsearch; \
		} \
		CHECK_ANY_BO(vcnil); \
	} \
	if(!index.is_nil()) \
		index.local_bind(vc(idx)); \
	return minvc; \
}

fmm(findmin, "findmin", <, ;)
fmm(findmax, "findmax", >, ;)

#define fmm2(funname, name, op, termsearch) \
vc \
vc##funname(vc vec, vc val, vc index) \
{ \
	if(vec.type() != VC_VECTOR) \
	{ \
		USER_BOMB("first arg to " name " must be vector", vcnil); \
	} \
	if(!index.is_nil() && index.type() != VC_STRING) \
	{ \
		USER_BOMB("third arg to " name " is either nil or string", vcnil); \
	} \
	int n = vec.num_elems(); \
	if(n == 0) \
		return vcnil; \
	int idx = 0; \
	int i; \
	for(i = 0; i < n; ++i) \
	{ \
		if(vec[i] op val) \
		{ \
			idx = i; \
			termsearch; \
		} \
		CHECK_ANY_BO(vcnil); \
	} \
	if(i == n) \
		return vcnil; \
	if(!index.is_nil()) \
		index.local_bind(vc(idx)); \
	return vctrue; \
}
fmm2(findeq, "findeq", ==, break)
fmm2(findne, "findne", !=, break)

vc
vclhsum(vc v)
{
	if(v.type() != VC_VECTOR)
	{
		USER_BOMB("can't sum non-vector", vcnil);
	}
	int n = v.num_elems();
	vc sum = vc(0);
	for(int i = 0; i < n; ++i)
		sum = sum + v[i];
	return sum;
}

vc
vclhsum2(vc v)
{
	if(v.type() != VC_VECTOR)
	{
		USER_BOMB("can't take sum of squares of non-vector", vcnil);
	}
	int n = v.num_elems();
	vc sum = vc(0);
	for(int i = 0; i < n; ++i)
		sum = sum + (v[i] * v[i]);
	return sum;
}
#endif
