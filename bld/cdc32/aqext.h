
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqpgm.h 1.5 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef AQEXT_H
#define AQEXT_H
#include "vidaq.h"
#include "vidcvt.h"

// acquire data from an external source. this is used to
// support the dwyco DLL
class ExtAcquire : public VidAcquire
{
public:
    ExtAcquire();
    ~ExtAcquire();
    int generates_events();
    int init(int frame_rate);
    void need();
    void pass();
    int has_data();
    void stop();
    void *get_data(int& cols, int& rows, void*& = VidAcquire::dummy,
                   void*&  = VidAcquire::dummy,
                   void*&  = VidAcquire::dummy,
                   int& = VidAcquire::dummy2,
                   unsigned long& = VidAcquire::dummy3,
                   int no_convert = 0);

    void set_swap_rb(int);
private:
    VidConvert vidconv;
};

#endif
