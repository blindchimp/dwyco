
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/*
This objects are used in the interpreter to represent the location
in the source code (as seen by the lexer) of various parts of the
parse tree. It is used for debugging, printing, and simple coverage
reporting.

If VCDBG is NOT defined, simple parse-time and run time errors can give
approximate locations for errors (filename, line number locations).

If VCDBG is defined, more verbose runtime information is accumulated
and more detailed info about how the parse tree is being executed is kept.
Most of this info is printed when there is a request to dump an execution
backtrace.

For coverage tracking (useful mainly for testing), the following environment
variables are used:
DWYCO_COLORIZED_NAME - set this to the name of the file which is created to contain the
coverage information. If this is not set, the default "color.out" is used.
DWYCO_COLORIZED_APPEND - if this is defined, any existing coverage file will be appended
to instead of deleted on run. This is used to accumulate coverage info across multiple
runs.

The coverage info provides only a binary indication: if an expr is evaluated, the coordinates of
the expr are output to the coverage file once. The coordinates are of the form:
filename char-index-start char-index-end

the char-indexes start at 1 in the filename given.

there are some tools that read the coverage info and output ansi-colorized versions of
the source code to make it clear what parts of your program are not executed.

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
