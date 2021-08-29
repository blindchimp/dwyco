#ifndef SIMPLE_PROPERTY_H
#define SIMPLE_PROPERTY_H

#include "ssns.h"
[[noreturn]] void oopanic(const char *);

namespace dwyco {
template<class T>
class sigprop
{
private:
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
};

}

#endif // SIMPLE_PROPERTY_H
