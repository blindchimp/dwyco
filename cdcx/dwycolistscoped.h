
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCOLISTSCOPED_H

struct simple_scoped
{
    DWYCO_LIST value;
    simple_scoped(DWYCO_LIST v) {
        value = v;
    }
    ~simple_scoped() {
        dwyco_list_release(value);
    }
    operator DWYCO_LIST() {
        return value;
    }
};

#endif
