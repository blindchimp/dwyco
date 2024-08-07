
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
#include "dwvecp.h"
#include "dwlista.h"
#include "dwiter.h"
#include "dwhash.h"

template <class T> class DwBagIter;

// note: this is gross. this provides a way to
// delete an item without searching for it first.
// used for performance stuff, it is easy to create
// dangling refs and other bad stuff using this.
template<class T> struct dwinternal_pos {
    dwinternal_pos(listelem<T> *p = 0, int i = -1) : pos(p), idx(i) {}
    listelem<T> *pos;
    int idx;
};

template<class T>
class DwBag
{
    friend class DwBagIter<T>;
private:
    DwVecP<DwListA<T> > table;
    int table_size;
    int count;
    T def;

    void init(int idx) {
        if(table[idx] == 0)
            table[idx] = new DwListA<T>;
    }

public:

    DwBag(T def, int tabsize);
    DwBag(const DwBag&);
    virtual ~DwBag();

    DwBag& operator=(const DwBag&);

    int num_elems() const {
        return count;
    }
    int contains(const T&) const;
    int del(const T&);
    int del_all(const T&);
    int find(const T&, T& out, T** wp = 0);
    DwVec<T> find(const T&);
    virtual void add(const T&, T** wp = 0);
    dwinternal_pos<T> add2(const T&);
    void del2(dwinternal_pos<T>);
    int count_keys(const T&);
    // warning, this replaces *all* existing keys
    int replace(const T&, T** wp = 0);
    void set_size(int);
    void clear();
    T get_by_iter(DwIter<DwBag<T>, T > *a) const {
        DwBagIter<T> *dbi = (DwBagIter<T> *)a;
        return dbi->list_iter->get();
    }
};


template<class T>
DwBag<T>::DwBag(T deflt, int tabsz)
    : table(tabsz == 0 ? 31 : tabsz, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND)
{
    count = 0;
    table_size = table.num_elems();
    def = deflt;
}

template<class T>
DwBag<T>::DwBag(const DwBag<T>& m)
    : table(m.table_size, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND)
{
    count = m.count;
    table_size = m.table_size;
    def = m.def;
    table = m.table;
    for(int i = 0; i < table_size; ++i)
    {
        if(m.table[i])
            table[i] = new DwListA<T>(*m.table[i]);
    }
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
    clear();
    count = m.count;
    table_size = m.table_size;
    def = m.def;
    table.set_size(m.table_size);
    for(int i = 0; i < table_size; ++i)
    {
        if(m.table[i])
            table[i] = new DwListA<T>(*m.table[i]);
    }
    return *this;
};

template<class T>
DwBag<T>::~DwBag()
{
    for(int i = 0; i < table_size; ++i)
        delete table[i];
}

template<class T>
void
DwBag<T>::clear()
{
    for(int i = 0; i < table_size; ++i)
    {
        delete table[i];
        table[i] = 0;
    }
    count = 0;
}


template<class T>
void
DwBag<T>::add(const T& key, T **wp)
{
    unsigned long hval = ::hash(key) % table_size;
    init(hval);
    table[hval]->prepend(key);
    ++count;
}

template<class T>
dwinternal_pos<T>
DwBag<T>::add2(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;
    init(hval);
    table[hval]->prepend(key);
    table[hval]->rewind();
    ++count;

    return dwinternal_pos<T>(table[hval]->getpos(), hval);
}

template<class T>
void
DwBag<T>::del2(dwinternal_pos<T> p)
{
    table[p.idx]->setpos(p.pos);
    table[p.idx]->remove();
    count--;
    return;
}

// warning: this version replaces *all* the existing keys
//
template<class T>
int
DwBag<T>::replace(const T& key, T** wp)
{
    int ret = 0;
    while(del(key))
        ret = 1;
    add(key, wp);
    return ret;
}


template<class T>
int
DwBag<T>::del_all(const T& key)
{
    int ret = 0;
    while(del(key))
        ret = 1;
    return ret;
}

template<class T>
int
DwBag<T>::del(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;
    if(table[hval] == 0)
        return 0;
    if(!table[hval]->exists(key))
        return 0;
    table[hval]->remove();
    --count;
    return 1;
}


template<class T>
int
DwBag<T>::contains(const T& key) const
{
    unsigned long hval = ::hash(key) % table_size;
    if(table[hval] == 0)
        return 0;
    return table[hval]->exists(key);
}

template<class T>
int
DwBag<T>::find(const T& key, T& out, T **wp)
{
    unsigned long hval = ::hash(key) % table_size;

    if(table[hval] == 0)
        return 0;
    if(!table[hval]->exists(key, out))
        return 0;
    return 1;
}

template<class T>
DwVec<T>
DwBag<T>::find(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;
    DwVec<T> ret;
    if(table[hval] == 0)
        return ret;
    DwListA<T> *l = table[hval];
    l->rewind();
    while(!l->eol())
    {
        const T& d = l->peek_read();
        if(d == key)
            ret.append(d);
    }
    return ret;
}

template<class T>
int
DwBag<T>::count_keys(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;
    if(table[hval] == 0)
        return 0;
    DwListA<T> *l = table[hval];
    l->rewind();
    int cnt = 0;
    while(!l->eol())
    {
        const T& d = l->peek_read();
        if(d == key)
            ++cnt;
    }
    return cnt;
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
    while(cur_buck < this->to_iterate->table_size && (this->to_iterate->table[cur_buck] == 0 || this->to_iterate->table[cur_buck]->num_elems() == 0))
        ++cur_buck;
    if(cur_buck < this->to_iterate->table_size)
    {
        list_iter = new DwListAIter<T>(this->to_iterate->table[cur_buck]);
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
