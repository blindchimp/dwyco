
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

#define USE_GENERAL_MAP 3

template<class R, class D> class DwAMapIter;
template<class R, class D> class DwMapsIter;
enum maptype {LAZYC, LAZYV, MAPR};

thdr
class DwAMap : public DwMaps<R,D>
{
    friend class DwAMapIter<R,D>;

private:

    DwMaps<R,D> *map;
    int can_contract;
    int level;
    static maptype exp_table[4];
    void expand();
    void contract();
    DwMaps<R,D> *make_map(enum maptype, long size);
    DwMaps<R,D> *make_map(enum maptype, long size, const R&, const D&);

public:
    DwAMap(const R&, const D&, unsigned int contr = 0);
    DwAMap(int isize, unsigned int contr = 0);
    ~DwAMap();
    int no_expand;

    virtual int num_elems() const ;
    virtual int contains(const D&) ;
    virtual int find(const D&, R& out, R** wp = 0) ;
    virtual void add(const D&, const R&) ;
    virtual int replace(const D&, const R&, R** wp = 0) ;
    virtual R get(const D&) ;
    virtual int del(const D&) ;
    virtual void fast_clear();

    virtual DwAssocImp<R,D> get_by_iter(DwIter<DwMaps<R,D>, DwAssocImp<R,D> > *) const;
    DwMapsIter<R,D> *make_iter() const;
};

thdr
maptype tcls::exp_table[4] =
{LAZYC, LAZYC, LAZYC, LAZYV};

thdr
DwAssocImp<R,D>
tcls::get_by_iter(DwIter<DwMaps<R,D>, DwAssocImp<R,D> > *a) const
{
    typedef DwAMapIter<R,D> *iter_p;
    iter_p ip = (iter_p)a;
    return ip->dmi->get();
}


// note: leave the default smaller map at 32
// since LH has some problems with caching
// and expanding maps at the moment.
thdr
tcls::DwAMap(const R& r, const D& d, unsigned int ctr)
{
    can_contract = ctr;
    level = 2;
    map = make_map(exp_table[level], 32, r, d);
    no_expand = 0;
}

thdr
tcls::DwAMap(int isize, unsigned int ctr)
{
    can_contract = ctr;
    if(isize > 32)
    {
        level = USE_GENERAL_MAP;
        map = make_map(MAPR, isize);
    }
    else
    {
        level = 2;
        map = make_map(exp_table[level], 32);
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
tcls::contains(const D& key)
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
tcls::fast_clear()
{
    map->fast_clear();
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
tcls::make_map(enum maptype t, long size)
{
    return make_map(t, size, R(), D());
}

thdr
DwMaps<R,D> *
tcls::make_map(enum maptype t, long size, const R& r, const D& d)
{
    switch(t)
    {
    case LAZYC:
        if(size <= 8)
            return new DwQMapLazyC<R,D,8>(r, d);
        if(size <= 16)
            return new DwQMapLazyC<R,D,16>(r, d);
        if(size <= 32)
            return new DwQMapLazyC<R,D,32>(r, d);
        oopanic("size too big for lazyC map");
    /*NOTREACHED*/

    case LAZYV:
        return new DwQMapLazyV<R,D>(r, d, size);

    case MAPR:
        return new DwMapR<R,D>(r, d, size);

    default:
        oopanic("bad type in amap2");
        /*NOTREACHED*/
    }
    return 0;
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
    ++level;
    if(level >= USE_GENERAL_MAP)
        new_map = make_map(MAPR, /*next_prime(2 * map->num_elems())*/ 203);
    else
    {
        // if n is a power of 2, then we just want the next
        // higher power. otherwise, we want the power of
        // two one greater than the next level (this is because
        // this routine is called just before the table gets
        // full.)
        unsigned int n = map->num_elems();
        // if n is already a power of two, don't want to go
        // two levels up...
        int b;
        if((n & (n - 1)) == 0)
            b = 0;
        else
            b = 1;
        int l;
        for(l = 0; n ; ++l)
            n >>= 1;
        n = (1 << (l + b));
        new_map = make_map(exp_table[level], n);
    }

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
