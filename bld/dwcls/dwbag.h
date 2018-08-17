
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
 * $Header: g:/dwight/repo/dwcls/rcs/dwbag.h 1.14 1998/06/10 17:04:37 dwight Exp $
 */
#ifndef DWBAG_H
#define DWBAG_H
#include "dwvec.h"
#include "dwlista.h"
#include "dwiter.h"
#include "dwhash.h"

template <class T> class DwBagIter;

template<class T>
class DwBag
{
    friend class DwBagIter<T>;
private:
    DwVec<DwListA<T> > table;
    int table_size;
    int count;
    T def;

public:
    DwBag(T def, int tabsize);
    DwBag(const DwBag&);
    virtual ~DwBag();

    DwBag& operator=(const DwBag&);

    int num_elems() const {
        return count;
    }
    int contains(const T&);
    int del(const T&);
    int find(const T&, T& out);
    virtual void add(const T&);
    void set_size(int);
    T get_by_iter(DwIter<DwBag<T>, T > *a) const {
        DwBagIter<T> *dbi = (DwBagIter<T> *)a;
        return dbi->list_iter->get();
    }
};


template<class T>
DwBag<T>::DwBag(T deflt, int tabsz)
    : table(tabsz, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND)
{
    count = 0;
    table_size = tabsz;
    def = deflt;
}

#if 0
template<class T>
DwBag<T>::DwBag(int tabsz)
    : table(tabsz, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND)
{
    count = 0;
    table_size = tabsz;
    def = T();
}
#endif

template<class T>
DwBag<T>::DwBag(const DwBag<T>& m)
    : table(m.table_size, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND)
{
    count = m.count;
    table_size = m.table_size;
    def = m.def;
    table = m.table;
};

template<class T>
void
DwBag<T>::set_size(int sz)
{
    if(count != 0)
        ::abort();
    table.set_size(sz);
    table_size = sz;
}

template<class T>
DwBag<T>&
DwBag<T>::operator=(const DwBag<T>& m)
{
    count = m.count;
    table_size = m.table_size;
    def = m.def;
    table = m.table;
    return *this;
};

template<class T>
DwBag<T>::~DwBag()
{

}


template<class T>
void
DwBag<T>::add(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;
    table[hval].prepend(key);
    ++count;
}

template<class T>
int
DwBag<T>::del(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;

    if(!table[hval].exists(key))
        return 0;
    table[hval].remove();
    --count;
    return 1;
}


template<class T>
int
DwBag<T>::contains(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;

    return table[hval].exists(key);
}

template<class T>
int
DwBag<T>::find(const T& key, T& out)
{
    unsigned long hval = ::hash(key) % table_size;

    if(!table[hval].exists(key, out))
        return 0;
    return 1;
}

template <class T> class DwSet;

template<class T>
class DwBagIter : public DwIter<DwBag<T>, T>
{
    friend class DwBag<T>;
    friend class DwSet<T>;
private:
    int cur_buck;
    DwListAIter<T> *list_iter;
    void advance();

public:
    DwBagIter(const DwBag<T> *t) : DwIter<DwBag<T>, T>(t) {
        cur_buck = 0;
        list_iter = 0;
        advance();
    }
    ~DwBagIter() {
        if(list_iter != 0) delete list_iter;
    }
    void init() {
        cur_buck = 0;
        advance();
    }
    void rewind() {
        init();
    }
    void fast_forward() {
        rewind();
    }
    void forward();
    void backward() {
        forward();
    }
    int eol() {
        return cur_buck >= this->to_iterate->table_size;
    }
};

template<class T>
void
DwBagIter<T>::advance()
{
    if(list_iter != 0)
        delete list_iter;
    while(cur_buck < this->to_iterate->table_size && this->to_iterate->table[cur_buck].num_elems() == 0)
        ++cur_buck;
    if(cur_buck < this->to_iterate->table_size)
    {
        list_iter = new DwListAIter<T>(&this->to_iterate->table[cur_buck]);
    }
    else
        list_iter = 0;
}


template<class T>
void
DwBagIter<T>::forward()
{
    if(eol())
        return;
    list_iter->forward();
    if(list_iter->eol())
    {
        delete list_iter;
        list_iter = 0;
        ++cur_buck;
        advance();
        return;
    }
}
#endif
