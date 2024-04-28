
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
 * $Header: g:/dwight/repo/dwcls/rcs/dwamap.h 1.14 1997/06/01 04:39:52 dwight Stable095 $
 */
#ifndef DWAMAP_H
#define DWAMAP_H

#include "dwmaps.h"
#include "dwqmap3.h"
#include "dwmapr.h"
#include "dwassoc.h"

#define thdr template<class R, class D>
#define tcls DwAMap<R,D>

template<class R, class D> class DwAMapIter;
template<class R, class D> class DwMapsIter;


thdr
class DwAMap : public DwMaps<R,D>
{
    friend class DwAMapIter<R,D>;

private:

    DwMaps<R,D> *map;
    int can_contract;
    void expand();
    void contract();
    DwMaps<R,D> *make_map(long size);

public:
    DwAMap(int isize, unsigned int contr = 0);
    ~DwAMap();
    int no_expand;

    virtual int num_elems() const ;
    virtual int contains(const D&) const ;
    virtual int find(const D&, R& out, R** wp = 0) ;
    virtual void add(const D&, const R&) ;
    virtual int replace(const D&, const R&, R** wp = 0) ;
    virtual R get(const D&) ;
    virtual int del(const D&) ;

    virtual DwAssocImp<R,D> get_by_iter(DwIter<DwMaps<R,D>, DwAssocImp<R,D> > *) const;
    DwMapsIter<R,D> *make_iter() const;
};

thdr
DwAssocImp<R,D>
tcls::get_by_iter(DwIter<DwMaps<R,D>, DwAssocImp<R,D> > *a) const
{
    typedef DwAMapIter<R,D> *iter_p;
    iter_p ip = (iter_p)a;
    return ip->dmi->get();
}


thdr
tcls::DwAMap(int isize, unsigned int ctr)
{
    can_contract = ctr;
    if(isize > 32)
    {
        map = make_map(isize);
    }
    else
    {
        map = make_map(32);
    }
    no_expand = 0;
}

thdr
tcls::~DwAMap()
{
    delete map;
}

thdr
int
tcls::num_elems() const
{
    return map->num_elems();
}

thdr
int
tcls::contains(const D& key) const
{
    return map->contains(key);
}

thdr
int
tcls::find(const D& key, R& out, R** wp)
{
    return map->find(key, out, wp);
}

thdr
void
tcls::add(const D& key, const R& val)
{
    if(map->over_threshold())
    {
        // re-build the hash table at the next
        // bigger size.
        expand();
        this->ruffled = 1;
    }
    map->add(key, val);
}

thdr
int
tcls::replace(const D& key, const R& val, R** wp)
{
    if(map->over_threshold())
    {
        expand();
        this->ruffled = 1;
    }
    return map->replace(key, val, wp);
}

thdr
R
tcls::get(const D& key)
{
    return map->get(key);
}

thdr
int
tcls::del(const D& key)
{
    int ret = map->del(key);
    if(can_contract && map->under_threshold())
    {
        contract();
        this->ruffled = 1;
    }
    return ret;
}

thdr
DwMaps<R,D> *
tcls::make_map(long size)
{

    if(size <= 8)
        return new DwQMapLazyC<R,D,8>();
    if(size <= 16)
        return new DwQMapLazyC<R,D,16>();
    if(size <= 32)
        return new DwQMapLazyC<R,D,32>();
    return new DwMapR<R,D>(size);

}

thdr
void
tcls::expand()
{
    if(no_expand)
    {
        oopanic("can't expand map");
        // NOT REACHED
    }
    DwMaps<R,D> *new_map;
    int n = num_elems();
    if(n <= 8)
        n = 16;
    else if(n <= 16)
        n = 32;
    else
        n= 203;

    new_map = make_map(n);

    DwMapsIter<R,D> *i = map->make_iter();
    for(; !i->eol(); i->forward())
    {
        DwAssocImp<R,D> a = i->get();
        const D& k = a.peek_key();
        const R& v = a.peek_value();
        new_map->add(k, v);
    }

    delete map;
    delete i;
    map = new_map;
}

thdr
void
tcls::contract()
{
    oopanic("contraction not implemented");
}

thdr
class DwAMapIter : public DwMapsIter<R,D>
{
    friend class DwAMap<R,D>;
private:
    DwMapsIter<R,D> *dmi;

public:
    DwAMapIter(const DwAMap<R,D> *t) : DwMapsIter<R,D>(t) {
        dmi = t->map->make_iter();
    }
    ~DwAMapIter() {
        delete dmi;
    }
    void init() {
        dmi->init();
    }
    void rewind() {
        dmi->rewind();
    }
    void fast_forward() {
        dmi->fast_forward();
    }
    void forward() {
        dmi->forward();
    }
    void backward() {
        dmi->backward();
    }
    int eol() {
        return dmi->eol();
    }
};

thdr
DwMapsIter<R,D> *
tcls::make_iter() const
{
    return new DwAMapIter<R,D>(this);
}


#undef tcls
#undef thdr

#endif
