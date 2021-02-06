
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifdef _Windows
#include <windows.h>
#include <io.h>
#else
#define O_BINARY 0
#include <unistd.h>
#endif
#include <fcntl.h>
#include "vc.h"
#include "dwstr.h"
#include "fnmod.h"
using namespace dwyco;

extern vc StackDump;

void
load_lhcore()
{
    static char dump[1024];
    int fd = open(newfn("lhcore").c_str(), O_RDONLY|O_BINARY);
    if(fd < 0)
        return;
    int amt = read(fd, dump, sizeof(dump));
    close(fd);
    if(amt < 0)
        return;
    StackDump = vc(VC_BSTRING, dump, amt);
}

