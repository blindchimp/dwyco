
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWSET_H
#define DWSET_H

#include "dwbag.h"

template <class T> class DwSetIter;

template<class T>
class DwSet : public DwBag<T>
{
    friend class DwSetIter<T>;
public:
    DwSet(int t) :
        DwBag<T>(t) {}


    void add(const T& key, T** wp = 0) {
        if(!this->contains(key))
            DwBag<T>::add(key, wp);
    }
    T get_by_iter(DwIter<DwSet<T>, T > *a) const {
        DwSetIter<T> *dbi = (DwSetIter<T> *)a;
        return dbi->list_iter->get();
    }
};

template<class T>
class DwSetIter : public DwBagIter<T>
{
    friend class DwSet<T>;
public:
    DwSetIter(const DwSet<T> *t) : DwBagIter<T>(t) {}

};


#endif
