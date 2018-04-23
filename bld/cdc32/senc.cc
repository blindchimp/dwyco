
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/senc.cc 1.2 1999/01/10 16:09:52 dwight Checkpoint $

#include <stdlib.h>
#include <time.h>
#include "dwyco_rand.h"

void
random_block(char *buf, int len)
{
    for(int i = 0; i < len; ++i)
    {
        buf[i] = dwyco_rand() / 256;
    }
}

