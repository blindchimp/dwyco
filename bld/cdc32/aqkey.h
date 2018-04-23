
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

// class used to encapsulate aquiring text from the user's
// keyboard. subclasses handle the system dependent things
// that actually get the data to the client of this class.

class KeyboardAcquire
{
public:
    KeyboardAcquire () {
        inited = 0;
    }
    virtual ~KeyboardAcquire() {}

    int init_ok() {
        return inited;
    }

    virtual void flush() {}
    virtual int has_data() = 0;
    virtual char *get_data(int& len) = 0;
    virtual vc get_structured_data() {
        return vcnil;
    }

    void set_fail_reason(const char *a) {
        strcpy(fail_reason, a);
    }
    char *get_fail_reason() {
        return strdup(fail_reason);
    }

protected:
    int inited;
    char fail_reason[255];

};

#endif
