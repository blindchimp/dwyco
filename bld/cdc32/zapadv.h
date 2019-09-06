
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#if !defined(__zapadv_h)              // Sentry, use file only if it's not already included.
#define __zapadv_h

#include "uicfg.h"

#include "qauth.h"

struct ZapAdvXfer {
public:
    ZapAdvXfer();
    void save();
    void load();

    DWUIDECL_BEGIN
    DWUIDECLVAL(bool, always_server)
    DWUIDECLVAL(bool, always_accept)
    DWUIDECLVAL(bool, ignore)
    DWUIDECLVAL(bool, use_old_timing)
    DWUIDECLVAL(bool, save_sent)
    DWUIDECLVAL(bool, no_forward_default)
    DWUIDECL_END
};

extern ZapAdvXfer ZapAdvData;

#endif                                      // __zapadv_h sentry.

