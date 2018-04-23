
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef JCOLCOD_H
#define JCOLCOD_H
#include "colcod.h"

class JPEGCoderColor : public DwCoderColor
{
public:
    JPEGCoderColor();
    virtual ~JPEGCoderColor();

    virtual DWBYTE *final_package(int& len);

};
#endif
