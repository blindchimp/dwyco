
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
 * $Header: g:/dwight/repo/dwcls/rcs/dwassoc.h 1.10 1997/06/01 04:39:53 dwight Stable095 $
 */
#ifndef DWASSOC_H
#define DWASSOC_H

#include "dwhash.h"
// warning: this class is designed to be used with the
// map adt in this library: only the key is used for
// equality checking
template<class R, class D>
class DwAssocImp
{
private:
    R value;
    D key;

public:
    DwAssocImp(const R& v, const D& k);
    DwAssocImp() = default;
//    DwAssocImp(const DwAssocImp&);
//    DwAssocImp& operator=(const DwAssocImp&);
//    ~DwAssocImp() {}
    int operator==(const DwAssocImp &a) const {
        return key == a.key;
    }
    R get_value() const {
        return value;
    }
    D get_key() const {
        return key;
    }
    const R& peek_value() const {
        return value;
    }
    const D& peek_key() const {
        return key;
    }
    R& ref_value() {
        return value;
    }
    unsigned long hashValue() const {
        return ::hash(key);
    }
};

template<class R, class D>
inline
DwAssocImp<R,D>::DwAssocImp(const R& v, const D& k)
    : value(v), key(k)
{
}

//template<class R, class D>
//DwAssocImp<R,D>::DwAssocImp(const DwAssocImp<R,D>& c)
//    :
//    value(c.value),
//    key(c.key)
//{
//}

//template<class R, class D>
//DwAssocImp<R,D>&
//DwAssocImp<R,D>::operator=(const DwAssocImp<R,D>& c)
//{
//    if(this != &c)
//    {
//        value = c.value;
//        key = c.key;
//    }
//    return *this;
//}

#endif
