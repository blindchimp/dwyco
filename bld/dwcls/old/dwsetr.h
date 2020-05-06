
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWSETR_H
#define DWSETR_H

#include "dwbagr.h"

template<class T> class DwSetRIter;

template<class T>
class DwSetR : public DwBagR<T>
{
    friend class DwSetRIter<T>;
public:
    DwSetR(T def, int t = 31) :
        DwBagR<T>(def, t) {}
    DwSetR(int t = 31) :
        DwBagR<T>(t) {}
    DwSetR(const DwSetR&) {}
    virtual ~DwSetR() {}

    void add(const T& key, T** wp = 0) {
        if(!this->contains(key))
            DwBagR<T>::add(key, wp);
    }
    T get_by_iter(DwIter<DwSetR<T>, T > *a) const {
        return DwBagR<T>::get_by_iter((DwIter<DwBagR<T>, T> *)a);
    }
};

template<class T>
class DwSetRIter : public DwBagRIter<T>
{
    friend class DwSetR<T>;
public:
    DwSetRIter(const DwSetR<T> *t) : DwBagRIter<T>(t) {}

};


#endif
