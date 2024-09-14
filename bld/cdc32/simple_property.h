
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef SIMPLE_PROPERTY_H
#define SIMPLE_PROPERTY_H

//
// if T has =,==, default ctor, you get a signal when the value actually
// changes.
//
// note: if T is a composite type that contains "add" and "del" methods
// you can get signals when new or existing  elements of type T are added or removed  (resp)
// from the set. note: this only works with vc right now, since elements and sets are the same type.
// WARNING: if it is a composite, and you assign a value that overwrites the entire
// composite value, you get ONE signal for "value_changed", it does NOT try to emit
// signals as if each element in the composite was deleted then added again.
//
// WARNING: this is not a very robust class, and is really only intended to
// accommodate simple intrinsic types and a couple of container classes
// from dwcls.
//
#include "ssns.h"
[[noreturn]] void oopanic(const char *);

namespace dwyco {
template<class T>
class sigprop
{
protected:
    T val;

public:
    sigprop() {val = T();}
    sigprop(const T& v) {val = v;}

    ssns::signal1<T> value_changed;
    ssns::signal3<T, T, int> element_changed;

    void set_val(const T& v) {
        if(val != v)
        {
            val = v;
            value_changed.emit(val);
        }
    }
    T get_val() const {return val;}
    T operator=(const T& v) {
        set_val(v);
        return val;
    }
    int operator==(const T& v) const {
        return val == v;
    }
    operator T() const {
        return val;
    }
    T operator->() const {
        if(val == 0)
            oopanic("splat null");
        return val;
    }
    operator bool() const {
        if(val == 0)
            return false;
        return true;
    }

    // surprising, but glad it works. if T doesn't have these members
    // as long as you don't invoke these members, the compiler won't complain
    void add(const T& e) {
        if(val.contains(e))
            return;
        val.add(e);
        value_changed.emit(val);
        element_changed.emit(val, e, 1);
    }
    void del(const T& e) {
        if(val.del(e))
        {
            value_changed.emit(val);
            element_changed.emit(val, e, 0);
        }
    }
};



}

#endif // SIMPLE_PROPERTY_H
