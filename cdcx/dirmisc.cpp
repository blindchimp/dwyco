
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include "ctype.h"
#include "dirmisc.h"
// since the qt stuff can show multiple lines in a row of a table view,
// we have to limit what people put in there, so they don't
// monopolize the directory listing.

DwOString
make_two_lines(const DwOString& d)
{
    DwOString ret(d);
    ret.srep_all("\r", "");
    //ret.srep("\r", "", 1);
    int i = ret.find_first_of("\n");
    if(i == DwOString::npos)
        return ret;
    int j = ret.find_last_of("\n");
    while(i != j)
    {
        ret[j] = ' ';
        j = ret.find_last_of("\n");
    }
    if(ret.length() > 100)
        ret.remove(100);
    return ret;

}

int
no_displayables(const char *str)
{
    int n = strlen(str);
    for(int i = 0; i < n; ++i)
    {
        if(!isspace(str[i]))
            return 0;
    }
    return 1;
}
