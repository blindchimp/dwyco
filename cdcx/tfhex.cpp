
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <stdio.h>
#include "dwstr.h"
#include "tfhex.h"

static int
hexd(char a)
{
    if(a >= '0' && a <= '9')
        return a - '0';
    if(a >= 'a' && a <= 'f')
        return a - 'a' + 10;
    if(a >= 'A' && a <= 'F')
        return a - 'A' + 10;
    return 0;
}

DwOString
to_hex(DwOString s)
{
    const char *a = s.c_str();
    int len = s.length();
    char *d = new char[len * 2 + 1];
    for(int i = 0; i < len; ++i)
    {
        sprintf(&d[2 * i], "%02x", a[i] & 0xff);
    }
    DwOString r(d, 0, len * 2);
    delete [] d;
    return r;
}

DwOString
from_hex(DwOString s)
{
    const char *a = s.c_str();
    int len = s.length();
    char *d = new char[len / 2];
    for(int i = 0; i < len / 2; ++i)
    {
        d[i] = (char)((hexd(a[i * 2]) << 4) | hexd(a[i * 2 + 1]));
    }
    DwOString r(d, 0, len / 2);
    delete [] d;
    return r;
}
