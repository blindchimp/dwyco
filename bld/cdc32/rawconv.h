
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
// $Header: g:/dwight/repo/cdc32/rcs/rawconv.h 1.15 1999/01/10 16:10:57 dwight Checkpoint $
#ifndef RAWCONV_H
#define RAWCONV_H

#include "audconv.h"

class RawConverter : public AudioConverter
{
public:
    RawConverter(int decom);

    virtual int convert(DWBYTE *buf, int len, int user_data, DWBYTE *&buf_out,
                        int &len_out);

};

#endif
