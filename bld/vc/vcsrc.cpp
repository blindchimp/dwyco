
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "vcsrc.h"
#include "vclex.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcsrc.cpp 1.45 1996/11/17 05:59:13 dwight Stable $";

long Vctok_char_pos;
long Vclinenum;

vc_cvar_src_coord::vc_cvar_src_coord()
{

}

void
vc_cvar_src_coord::init(VcLexer *l)
{
	linenum = l->token_linenum();
	filename = l->input_description();
	abs_charnum = 0;
	charnum = 0;
}
