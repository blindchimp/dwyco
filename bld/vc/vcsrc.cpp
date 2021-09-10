
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef NO_VCEVAL
#include "vcsrc.h"
#include "vclex.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vcsrc.cpp 1.45 1996/11/17 05:59:13 dwight Stable $";
#include <stdio.h>
FILE *vc_cvar_src_coord::f;

vc_cvar_src_coord::vc_cvar_src_coord()
{
    linenum = -1;
    char_index_start = -1;
    char_index_end = -1;
    already_printed = false;
}

void
vc_cvar_src_coord::init(VcLexer *l)
{
    linenum = l->token_linenum();
    filename = l->input_description();
    char_index_start = l->char_start_token;
    char_index_end = l->char_end_token;
}

void
vc_cvar_src_coord::print() const
{
    if(already_printed)
        return;
    if(!f)
    {
        const char *p = getenv("DWYCO_COLORIZED_NAME");
        if(!p)
            p = "color.out";
        const char *a = getenv("DWYCO_COLORIZED_APPEND");

        f = fopen(p, a == 0 ? "w" : "a");
    }
    if(char_index_start != -1 && char_index_end != -1)
    {
        fprintf(f, "%s %ld %ld\n", filename.c_str(), char_index_start, char_index_end);
        already_printed = true;
    }
}
#endif
