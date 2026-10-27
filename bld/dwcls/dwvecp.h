
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWVECP_H
#define DWVECP_H
#include "useful.h"
#include "dwvec.h"
#include <utility>
#undef index

// class for vector of pointers. stores T* directly (not void*),
// so there is no type punning / strict-aliasing issue.
//
template<class T> class DwVecPIter;

template<class T>
class DwVecP : private DwVec<T*>
{
public:
    typedef void (*Funcp)(T*&);

    static void pnull(T*& v) { v = nullptr; }

    DwVecP(long i_cnt = 0, int is_fixed = 0, int auto_expand = 1,
           void (*ifun)(T*&) = 0)
        : DwVec<T*>(i_cnt, is_fixed, auto_expand, DWVEC_DEFAULTBLOCKSIZE,
                    ifun ? ifun : &DwVecP<T>::pnull) {}

    DwVecP(const DwVecP& v) : DwVec<T*>(v) {}
    DwVecP(DwVecP&& v) noexcept : DwVec<T*>(std::move(v)) {}

    DwVecP& operator=(const DwVecP& s) {
        return (DwVecP&)DwVec<T*>::operator=(s);
    }
    DwVecP& operator=(DwVecP&& s) noexcept {
        return (DwVecP&)DwVec<T*>::operator=(std::move(s));
    }

    ~DwVecP() {}

    int operator==(const DwVecP& s) const {
        return DwVec<T*>::operator==(s);
    }
    int operator!=(const DwVecP& t) const {
        return !(*this == t);
    }

    T*& operator[](long idx) {
        return DwVec<T*>::operator[](idx);
    }
    T* const & operator[](long idx) const {
        return DwVec<T*>::operator[](idx);
    }

    long num_elems() const {
        return DwVec<T*>::num_elems();
    }
    void set_size(long count) {
        DwVec<T*>::set_size(count);
    }
    void append(const T* e) {
        T* p = const_cast<T*>(e);
        DwVec<T*>::append(p);
    }
    void insert(const T* e, long i) {
        T* p = const_cast<T*>(e);
        DwVec<T*>::insert(p, i);
    }
    void spread(long idx, long cnt = 1) {
        DwVec<T*>::spread(idx, cnt);
    }
    void del(long idx, long cnt = 1) {
        DwVec<T*>::del(idx, cnt);
    }
    long index(const T* e) const {
        T* p = const_cast<T*>(e);
        return DwVec<T*>::index(p);
    }
    int contains(const T* e) const {
        T* p = const_cast<T*>(e);
        return DwVec<T*>::contains(p);
    }
    void apply(Funcp f) {
        DwVec<T*>::apply(f);
    }

    T* get_by_iter(DwIter<DwVec<T*>, T*> *a) const {
        return DwVec<T*>::get_by_iter(a);
    }
};

template<class T>
class DwVecPIter : public DwVecIter<T*>
{
public:
    DwVecPIter(const DwVecP<T> *t) : DwVecIter<T*>((DwVec<T*> *)t) {}
    T* getp() {
        return this->to_iterate->get_by_iter(this);
    }
};

#endif