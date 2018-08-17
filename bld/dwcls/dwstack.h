
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/

/*
 * $Header: g:/dwight/repo/dwcls/rcs/dwstack.h 1.10 1997/06/01 04:40:00 dwight Stable095 $
 */
#ifndef DWSTACK_H
#define DWSTACK_H

#include "dwlista.h"

template<class T>
class DwStackA : private DwListA<T>
{
public:
    DwStackA(T def) : DwListA<T>(def) { }
    void push(const T& t) {
        append(t);
    }
    T pop() {
        T t(this->get_last());
        this->remove_last();
        return t;
    }
    void clear() {
        for(int i = num_elems(); i > 0; --i) {
            this->remove_last();
        }
    }

    DwListA<T>::num_elems;
};

#endif
