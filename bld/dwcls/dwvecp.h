
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
#undef index

// class for vector of pointers... you can use
// this class to save some space if you have
// lots of these sorts of vectors since it only
// relies on one instance of DwVec<void *>
// note: iterating over this class is a bit
// different from other classes: you need
// to call "getp()" to get the properly cast
// pointer from the vector. calling "get()"
// gets you a void *...
//

template<class T> class DwVecPIter;

template<class T>
class DwVecP : private DwVec<void *>
{

public:
    typedef void (*Funcp)(T*&);

    DwVecP(long i_cnt = 0, int is_fixed = 0, int auto_expand = 1, void (*ifun)(void *&) = 0)
        : DwVec<void *>(i_cnt, is_fixed, auto_expand, DWVEC_DEFAULTBLOCKSIZE,
                        (ifun == 0) ? vpnull : ifun) {}

    DwVecP& operator=(const DwVecP& s) {
        return (DwVecP&)DwVec<void *>::operator=(s);
    }
    ~DwVecP() {}
    int operator==(const DwVecP& s) const {
        return DwVec<void *>::operator==(s);
    }
    int operator!=(const DwVecP& t) const {
        return !(*this == t);
    }

    const T*& operator[](long idx) const {
        return (const T*&)DwVec<void *>::operator[](idx);
    }
    T*& operator[](long idx) {
        return (T*&)DwVec<void *>::operator[](idx);
    }

    long num_elems() const {
        return DwVec<void *>::num_elems();
    }
    void set_size(long count) {
        DwVec<void *>::set_size(count);
    }
    void append(const T* e) {
        DwVec<void *>::append((void *)e);
    }
    void insert(const T* e, long i) {
        DwVec<void *>::insert((void *)e, i);
    }
    void spread(long idx, long cnt = 1) {
        DwVec<void *>::spread(idx, cnt);
    }
    void del(long idx, long cnt = 1) {
        DwVec<void *>::del(idx, cnt);
    }
    long index(const T* e) const {
        return DwVec<void *>::index((void *)e);
    }
    int contains(const T* e) const {
        return DwVec<void *>::contains((void *)e);
    }
    void apply(Funcp f) {
        DwVec<void *>::apply((DwVec<void *>::Funcp)f);
    }

    void* get_by_iter(DwIter<DwVec<void *>, void *> *a) const {
        return (T*)DwVec<void *>::get_by_iter(a);
    }
#ifdef DWVEC_DOINIT
    void init_value(void *&v) {
        v = 0;
    }
#endif
};

template<class T>
class DwVecPIter : public DwVecIter<void *>
{
public:
    DwVecPIter(const DwVecP<T> *t) : DwVecIter<void *>((DwVec<void *> *)t) {}
    T* getp() {
        return (T*)to_iterate->get_by_iter(this);
    }
};


#endif
