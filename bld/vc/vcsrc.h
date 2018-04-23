
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCSRC_H
#define VCSRC_H
// $Header: g:/dwight/repo/vc/rcs/vcsrc.h 1.45 1996/11/17 05:59:14 dwight Stable $
class VcLexer;

struct vc_cvar_src_coord
{
	long abs_charnum;
	long linenum;
	long charnum;
	char *filename;

	vc_cvar_src_coord();
	void init(VcLexer *);
};

#endif
