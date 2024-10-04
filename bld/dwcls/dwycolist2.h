
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCOLIST2_H
#define DWYCOLIST2_H
#include "dlli.h"
#include <stdlib.h>

// note: dwyco_list and dwyco_list_throw are just convenience wrappers
// for the DWYCO_LIST. They do not try to manage the lifetime of the list.
// if you want the list to release when it goes out of scope, use "simple_scoped"
struct dwyco_list
{
private:
    dwyco_list() = delete;
    dwyco_list(const dwyco_list&) = delete;
    dwyco_list(const dwyco_list&&) = delete;
    dwyco_list& operator=(const dwyco_list&) = delete;
    dwyco_list& operator=(const dwyco_list&&) = delete;
protected:
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
    template<class T> T get(int row) {
        return get<T>(row, DWYCO_NO_COLUMN);
    }

    bool is_nil(const char *col) {
        return is_nil(0, col);
    }
    bool is_nil(int row, const char *col) {
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(value, row, col, &val, &len, &type))
            return 0;
        if(type == DWYCO_TYPE_NIL)
            return true;
        else
            return false;
    }
    long get_long(int row, const char *col) {
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

    long get_long(const char *col) {
        return get_long(0, col);
    }

};

struct dwyco_list_throw : public dwyco_list
{
private:
    dwyco_list_throw() = delete;
    dwyco_list_throw(const dwyco_list_throw&) = delete;
    dwyco_list_throw(const dwyco_list_throw&&) = delete;
    dwyco_list_throw& operator=(const dwyco_list_throw&) = delete;
    dwyco_list_throw& operator=(const dwyco_list_throw&&) = delete;

public:
    dwyco_list_throw(DWYCO_LIST l) : dwyco_list(l) {}

    int rows() {
        int n;
        if(dwyco_list_numelems(value, &n, 0))
            return n;
        throw -1;
    }

    template<class T> T get(int row, const char *col) {
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(value, row, col, &val, &len, &type))
            throw -1;
        if(type != DWYCO_TYPE_STRING)
            throw -1;
        return T(val, len);
    }
    template<class T> T get(const char *col) {
        return get<T>(0, col);
    }
    template<class T> T get(int row) {
        return get<T>(row, DWYCO_NO_COLUMN);
    }

    bool is_nil(const char *col) {
        return is_nil(0, col);
    }
    bool is_nil(int row, const char *col) {
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(value, row, col, &val, &len, &type))
            throw -1;
        if(type == DWYCO_TYPE_NIL)
            return true;
        else
            return false;
    }
    long get_long(int row, const char *col) {
        const char *val;
        int len;
        int type;
        if(!dwyco_list_get(value, row, col, &val, &len, &type))
            throw -1;
        if(type == DWYCO_TYPE_INT)
        {
            return atol(val);
        }
        else
            throw -1;
    }

    long get_long(const char *col) {
        return get_long(0, col);
    }

};

struct simple_scoped : public dwyco_list
{
private:
    simple_scoped() = delete;
    simple_scoped(const simple_scoped&) = delete;
    simple_scoped& operator=(const simple_scoped&) = delete;
    simple_scoped& operator=(const simple_scoped&&) = delete;
    simple_scoped(const simple_scoped&&) = delete;

public:
    simple_scoped(DWYCO_LIST& v): dwyco_list(v) {
        // this is just a safety thing, making sure you don't
        // directly use the list you sent in.
        v = 0;
    }
    ~simple_scoped() {
        release();
    }
};

#endif
