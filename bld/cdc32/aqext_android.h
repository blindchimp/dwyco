
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef AQEXT_ANDROID_H
#define AQEXT_ANDROID_H
#include "aqext.h"

class ExtAcquireAndroid : public ExtAcquire
{
public:
    void *get_data(int& cols, int& rows, void*& = VidAcquire::dummy,
                   void*&  = VidAcquire::dummy,
                   void*&  = VidAcquire::dummy,
                   int& = VidAcquire::dummy2,
                   unsigned long& = VidAcquire::dummy3,
                   int no_convert = 0);
};

#endif
