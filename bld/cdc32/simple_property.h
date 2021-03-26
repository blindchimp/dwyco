#ifndef SIMPLE_PROPERTY_H
#define SIMPLE_PROPERTY_H

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
    }
    void del(const T& e) {
        val.del(e);
        value_changed.emit(val);
    }
};



}

#endif // SIMPLE_PROPERTY_H
