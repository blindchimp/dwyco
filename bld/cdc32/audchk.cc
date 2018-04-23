
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/audchk.cc 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "audchk.h"
#include "audo.h"
#include "audi.h"

int Audio_hw_full_duplex;	// 1 if hw supports full duplex
int Audio_full_duplex;		// 1 if hardware should be used in full duplex mode
int Has_audio_output;
int Has_audio_input;

int
check_duplex()
{
    //return 0; // testing
    if(!init_audio_input(0, 1))
        return 0;
    if(!init_audio_output(0))
    {
        exit_audio_input();
        return 0;
    }
    Audio_hw_full_duplex = 1;
    exit_audio_output();
    exit_audio_input();
    return 1;
}

int
check_audio_device()
{
    check_duplex();
    if(init_audio_input(0, 1))
    {
        Has_audio_input = 1;
        exit_audio_input();
    }
    if(init_audio_output(0))
    {
        Has_audio_output = 1;
        exit_audio_output();
    }
    return 0;
}
