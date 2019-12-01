
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
 * $Header: g:/dwight/repo/dwcls/rcs/dwbagr.h 1.12 1997/06/01 04:39:53 dwight Stable095 $
 */
#ifndef DWBAGR_H
#define DWBAGR_H

#include "dwvec.h"
#include "dwlista.h"
#include "dwiter.h"
#include "dwhash.h"

class DwBagBase
{
    friend class TableElem;
public:
    virtual ~DwBagBase() {}
private:
    virtual int eq(const void *, const void *) const = 0;
};

class TableElem
{
public:
    void *rep;
    int operator==(const TableElem& t) const {
        return bag->eq(rep, t.rep);
    }
    TableElem(void *v = 0) : rep(v), bag(0) {}
    TableElem(const DwBagBase *b, void *v = 0)
        : rep(v), bag(b) {}
private:
    const DwBagBase *bag;
};

typedef DwListA<TableElem> TableList;
typedef DwListAIter<TableElem> TableListIter;

template<class T> class DwBagRIter;

template<class T>
class DwBagR : public DwBagBase
{
    friend class DwBagRIter<T>;
protected:
    T def;
    DwVec<TableList> table;
    int table_size;
    int count;

    int eq(const void *v1, const void *v2) const ;

public:
    DwBagR(T def, int tabsize = 31);
    DwBagR(int tabsize = 31);
    DwBagR(const DwBagR&);
    virtual ~DwBagR();
    void set_size(int);

    DwBagR& operator=(const DwBagR&);

    int num_elems() const;
    int contains(const T&);
    int del(const T&);
    int find(const T&, T& out, T** wp = 0);
    virtual void add(const T&, T** wp = 0);
    int replace(const T&, T** wp = 0);
    void clear();
#if (defined(__BCPLUSPLUS__) && __BCPLUSPLUS__ >= 0x540)
    T get_by_iter(DwIter<DwBagR<T>, T > *a) const ;
#else
    T get_by_iter(DwIter<DwBagR, T > *a) const ;
#endif
};


template<class T>
int
DwBagR<T>::num_elems() const
{
    return count;
}

template<class T>
void
DwBagR<T>::clear()
{
    int n = table.num_elems();
    for(int i = 0; i < n; ++i)
    {
        TableList &tl = table[i];
        int lcnt = tl.num_elems();
        for(int j = 0; j < lcnt; ++j)
        {
            //note: we partially destroy the list here
            // instead of letting the destructor do it
            // to avoid needing a list iterator
            TableElem e = tl.get_last();
            delete (T*)e.rep;
            tl.remove_last();
        }
    }
    count = 0;
}

template<class T>
int
DwBagR<T>::eq(const void *v1, const void *v2) const
{
    return *(const T*)v1 == *(const T*)v2;
}

template<class T>
DwBagR<T>::DwBagR(T deflt, int tabsz)
    : table(tabsz == 0 ? 31 : tabsz, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND)
{
    count = 0;
    table_size = tabsz;
    if(table_size == 0)
        table_size = 31;
    def = deflt;
}

template<class T>
DwBagR<T>::DwBagR(int tabsz)
    : table(tabsz == 0 ? 31 : tabsz, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND)
{
    count = 0;
    table_size = tabsz;
    if(table_size == 0)
        table_size = 31;
}

template<class T>
void
DwBagR<T>::set_size(int sz)
{
    if(count != 0)
        ::abort();
    table.set_size(sz);
    table_size = sz;
}

// note: these functions need remodularization...

template<class T>
DwBagR<T>::DwBagR(const DwBagR<T>& m)
    : table(m.table_size, !DWVEC_FIXED, !DWVEC_AUTO_EXPAND), def(m.def)
{
    count = m.count;
    table_size = m.table_size;
    //def = m.def;
    //table = m.table;
    for(int i = 0; i < table_size; ++i)
    {
        const TableList &tl = m.table[i];
        TableListIter tli(&tl);
        for(; !tli.eol(); tli.forward())
        {
            // need to explicitly copy
            // referenced items.
            TableElem e = tli.get();
            TableElem e2(this, new T(*(T*)e.rep));
            table[i].append(e2);
        }
    }
}

template<class T>
DwBagR<T>&
DwBagR<T>::operator=(const DwBagR<T>& m)
{
    if(this != &m)
    {
        count = m.count;
        table_size = m.table_size;
        def = m.def;
        //table = m.table;
        for(int i = 0; i < table_size; ++i)
        {
            TableList &tl = table[i];
            int lcnt = tl.num_elems();
            for(int j = 0; j < lcnt; ++j)
            {
                TableElem e = tl.get_last();
                delete (T*)e.rep;
                tl.remove_last();
            }
            const TableList &tl2 = m.table[i];
            TableListIter tli(&tl2);
            for(; !tli.eol(); tli.forward())
            {
                // need to explicitly copy
                // referenced items.
                TableElem e = tli.get();
                TableElem e2(this, new T(*(T*)e.rep));
                table[i].append(e2);
            }
        }
    }
    return *this;
}

template<class T>
DwBagR<T>::~DwBagR()
{
    int n = table.num_elems();
    for(int i = 0; i < n; ++i)
    {
        TableList &tl = table[i];
        int lcnt = tl.num_elems();
        for(int j = 0; j < lcnt; ++j)
        {
            //note: we partially destroy the list here
            // instead of letting the destructor do it
            // to avoid needing a list iterator
            TableElem e = tl.get_last();
            delete (T*)e.rep;
            tl.remove_last();
        }
    }
}


template<class T>
void
DwBagR<T>::add(const T& key, T** wp)
{
    unsigned long hval = ::hash(key) % table_size;
    TableElem e(this, new T(key));
    table[hval].prepend(e);
    if(wp)
    {
        table[hval].rewind();
        TableElem *t = table[hval].nasty();
        *wp = (T*)t->rep;
    }
    ++count;
}

template<class T>
int
DwBagR<T>::del(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;

    TableElem e(this, (T*)&key);
    TableElem out;

    if(!table[hval].exists(e, out))
        return 0;
    delete (T*)out.rep;
    table[hval].remove();
    --count;
    return 1;
}


template<class T>
int
DwBagR<T>::contains(const T& key)
{
    unsigned long hval = ::hash(key) % table_size;

    TableElem e(this, (T*)&key);

    return table[hval].exists(e);
}

template<class T>
int
DwBagR<T>::find(const T& key, T& out, T** wp)
{
    unsigned long hval = ::hash(key) % table_size;

    TableElem e(this, (T*)&key);
    TableElem eout;

    if(table[hval].exists(e, eout))
    {
        out = *(T*)eout.rep;
        if(wp)
            *wp = (T*)eout.rep;
        return 1;
    }
    return 0;
}

template<class T>
int
DwBagR<T>::replace(const T& key, T** wp)
{
    unsigned long hval = hash(key) % table_size;

    TableElem e(this, (T*)&key);
    TableElem eout;

    if(table[hval].exists(e, eout))
    {
        *(T*)eout.rep = key;
        if(wp)
            *wp = (T*)eout.rep;
        return 1;
    }
    add(key, wp);
    return 0;
}


template<class T>
class DwBagRIter : public DwIter<DwBagR<T>, T>
{
    friend class DwBagR<T>;
private:
    int cur_buck;
    DwListAIter<TableElem> *list_iter;
    void advance();

public:
    DwBagRIter(const DwBagR<T> *t) : DwIter<DwBagR<T>, T>(t) {
        cur_buck = 0;
        list_iter = 0;
        advance();
    }
    ~DwBagRIter() {
        if(list_iter != 0) delete list_iter;
    }
    void init() {
        cur_buck = 0;
        advance();
    }
    void rewind() {
        init();
    }
    // note: no ordering implied, so backward and
    // forward can be the same.
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
DwBagRIter<T>::advance()
{
    if(list_iter != 0)
        delete list_iter;
    while(cur_buck < this->to_iterate->table_size && this->to_iterate->table[cur_buck].num_elems() == 0)
        ++cur_buck;
    if(cur_buck < this->to_iterate->table_size)
    {
        list_iter = new DwListAIter<TableElem>(&this->to_iterate->table[cur_buck]);
    }
    else
        list_iter = 0;
}


template<class T>
void
DwBagRIter<T>::forward()
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

template<class T>
T
DwBagR<T>::get_by_iter(DwIter<DwBagR<T>, T > *a) const
{
    DwBagRIter<T> *dbi = (DwBagRIter<T> *)a;
    return *(T*)dbi->list_iter->get().rep;
}
#endif
