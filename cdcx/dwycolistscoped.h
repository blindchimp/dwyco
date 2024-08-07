
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCOLISTSCOPED_H
#include "dlli.h"
#include <stdlib.h>

struct dwyco_list
{
private:
    dwyco_list();
    dwyco_list(const dwyco_list&);
    DWYCO_LIST value;
public:
    dwyco_list(DWYCO_LIST v) {
        value = v;
    }

    operator DWYCO_LIST() {
        return value;
    }

    void release() {
        if(value)
            dwyco_list_release(value);
        value = 0;
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
    int get_long(int row, const char *col) {
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(value, row, col, &val, &len, &type))
            return 0;
        if(type == DWYCO_TYPE_INT)
        {
            return atol(val);
        }
        else
            return 0;
    }

};

struct simple_scoped : public dwyco_list
{
private:
    simple_scoped();
    simple_scoped(const simple_scoped&);
public:
    simple_scoped(DWYCO_LIST v): dwyco_list(v) {
    }
    ~simple_scoped() {
        release();
    }
};

#endif
