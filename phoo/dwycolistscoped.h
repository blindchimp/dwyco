
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCOLISTSCOPED_H
#include "dlli.h"
struct simple_scoped
{
private:
    simple_scoped();
    simple_scoped(const simple_scoped&);
    DWYCO_LIST value;
public:
    simple_scoped(DWYCO_LIST v) {
        value = v;
    }
    ~simple_scoped() {
        if(value) dwyco_list_release(value);
    }
    operator DWYCO_LIST() {
        return value;
    }
    int rows() {
        int n;
        if(dwyco_list_numelems(value, &n, 0))
            return n;
        return 0;
    }

    template<class T> T get(int row, const char *col) {
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(value, row, col, &val, &len, &type))
            return T();
        if(type != DWYCO_TYPE_STRING)
            return T();
        return T(val, len);

    }
    template<class T> T get(const char *col) {
        return get<T>(0, col);
    }

    int is_nil(const char *col) {
        return is_nil(0, col);
    }
    int is_nil(int row, const char *col) {
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(value, row, col, &val, &len, &type))
            return 0;
        if(type == DWYCO_TYPE_NIL)
            return 1;
        else
            return 0;
    }
};

#endif
