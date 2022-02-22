
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/snds.cc 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include "snds.h"

char *Over_snd;

void
init_snds()
{
    FILE *f;
    if((f = fopen("over.wav", "rb")) == 0)
        return;
    fseek(f, 0, SEEK_END);
    int off = ftell(f) + 1;
    Over_snd = new char[off];
    if(fread(Over_snd, off, 1, f) != 1)
    {
        delete [] Over_snd;
        Over_snd = 0;
    }
    fclose(f);
}

void
snds_play(int n)
{
    //PlaySound(Over_snds, NULL, SND_ASYNC|SND_MEMORY|SND_NOSTOP);
}
