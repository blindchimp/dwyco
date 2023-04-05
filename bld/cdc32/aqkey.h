
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/aqkey.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef AQKEY_H
#define AQKEY_H
#include "vc.h"
#include "dwstr.h"

// class used to encapsulate acquiring text from the user's
// keyboard. subclasses handle the system dependent things
// that actually get the data to the client of this class.

class KeyboardAcquire
{
public:
    KeyboardAcquire ();
    virtual ~KeyboardAcquire() = default;

    int init_ok() {
        return inited;
    }

    virtual void flush() {}
    virtual int has_data() = 0;
    virtual char *get_data(int& len) = 0;
    virtual vc get_structured_data() {
        return vcnil;
    }

    void set_fail_reason(const DwString& a) {
        fail_reason = a;
    }
    DwString get_fail_reason() {
        return fail_reason;
    }

protected:
    int inited;
    DwString fail_reason;

};

#endif
