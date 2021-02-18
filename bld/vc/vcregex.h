
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCO_NO_VCREGEX
#ifndef VCREGEX_H
#define VCREGEX_H
// $Header: g:/dwight/repo/vc/rcs/vcregex.h 1.47 1997/11/06 21:37:56 dwight Stable $

#include "reobj.h"
#include "vc.h"
#include "vcstr.h"
#pragma warn -inl

class vc_regex : public vc_string
{
private:
	Regex re;

public:
	vc_regex(const char *s) : vc_string(s), re(s) {}
	~vc_regex() {}

	virtual int re_match(const vc& v, int len, int pos = 0) const;
	virtual int re_match(const vc& v) const;
	virtual int re_search(const vc& v, int len, int& matchlen, int startpos = 0) const;
	virtual int re_match_info(int& start, int& length, int nth = 0) const;

	enum vc_type type() const { return VC_REGEX; }
	virtual void stringrep(VcIO) const;
	virtual vc operator()(void) const {bomb_call_atom(); return vcnil;}
	//virtual vc operator()(void *p) const {bomb_call_atom(); return vcnil;}
	virtual vc operator()(VCArglist *al) const {bomb_call_atom(); return vcnil;}

	virtual vc operator()(vc v0) const {bomb_call_atom(); return vcnil;}
	virtual vc operator()(vc v0, vc v1) const {bomb_call_atom(); return vcnil;}
	virtual vc operator()(vc v0, vc v1, vc v2) const {bomb_call_atom(); return vcnil;}
	virtual vc operator()(vc v0, vc v1, vc v2, vc v3) const {bomb_call_atom(); return vcnil;}

};

#pragma .inl

#endif
#endif
