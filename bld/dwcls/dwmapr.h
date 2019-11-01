
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWMAPR_H
#define DWMAPR_H

#include "dwsetr.h"
#include "dwassoc.h"
#include "dwmaps.h"
#include "dwiter.h"

template<class R, class D> class DwMapRIter;

template<class R, class D>
class DwMapR : public DwMaps<R,D>
{
    friend class DwMapRIter<R,D>;

private:
    R def;
    DwBagR<DwAssocImp<R,D> > set;

public:
    DwMapR(const R& defr, const D& defd, unsigned int tsize = 31);
    DwMapR(unsigned int tsize = 31);
    virtual ~DwMapR();
    void set_size(int);

    int num_elems() const;
    int contains(const D&);
    int find(const D&, R& out, R** wp = 0);
    void add(const D&, const R&);
    int replace(const D&, const R&, R** wp = 0);
    R get(const D&);
    int del(const D&);
    void fast_clear();

    DwAssocImp<R,D> get_by_iter(DwIter<DwMaps<R,D>, DwAssocImp<R,D> > *a) const;
    DwMapsIter<R,D> *make_iter() const;
};

#define thdr template<class R, class D>
#define tcls DwMapR<R,D>
thdr
tcls::DwMapR(const R& defr, const D& defd, unsigned int tsize)
    : set(DwAssocImp<R,D>(defr, defd), tsize)
{
    def = defr;
}

thdr
tcls::DwMapR(unsigned int tsize)
    : set(DwAssocImp<R,D>(), tsize)
{
}

thdr
tcls::~DwMapR()
{
}

thdr
void
tcls::set_size(int sz)
{
    set.set_size(sz);
}

thdr
void
tcls::fast_clear()
{
    set.clear();
}

thdr
DwAssocImp<R,D>
tcls::get_by_iter(DwIter<DwMaps<R,D>, DwAssocImp<R,D> > *a) const
{
    typedef DwMapRIter<R,D> *mri_tp;
    mri_tp mip = (mri_tp)a;
    return mip->iter.get();
}

thdr
DwMapsIter<R,D> *
tcls::make_iter() const
{
    return new DwMapRIter<R,D>(this);
}

thdr
int
tcls::num_elems() const
{
    return set.num_elems();
}


thdr
int
tcls::contains(const D& key)
{
    DwAssocImp<R,D> tmp(def, key);
    return set.contains(tmp);
}

thdr
int
tcls::find(const D& key, R& out, R** wp)
{
    DwAssocImp<R,D> tmp(def, key);
    DwAssocImp<R,D> aout;

    // NASTY!
    DwAssocImp<R,D> *wpa;
    if(set.find(tmp, aout, &wpa))
    {
        out = aout.get_value();
        if(wp)
        {
            *wp = &wpa->ref_value();
        }
        return 1;
    }
    return 0;
}

thdr
void
tcls::add(const D& key, const R& value)
{
    DwAssocImp<R,D> tmp(value, key);
    if(set.contains(tmp))
        return;
    set.add(tmp);
}

thdr
int
tcls::replace(const D& key, const R& value, R** wp)
{
    DwAssocImp<R,D> tmp(value, key);
    DwAssocImp<R,D> *wpa;
    int ret = set.replace(tmp, &wpa);
    if(wp)
    {
        *wp = &wpa->ref_value();
    }
    return ret;
}

thdr
R
tcls::get(const D& key)
{
    R ret;

    if(find(key, ret))
        return ret;
    return def;
}

thdr
int
tcls::del(const D& key)
{
    DwAssocImp<R,D> tmp(def, key);
    return set.del(tmp);
}



template<class R, class D>
class DwMapRIter : public DwMapsIter<R,D>
{
    friend class DwMapR<R,D>;
private:
    DwBagRIter<DwAssocImp<R,D> > iter;

public:
    DwMapRIter(const DwMapR<R,D> *t) : DwMapsIter<R,D>(t) , iter(&t->set) {}
    ~DwMapRIter() {}
    void init() {
        iter.init();
    }
    void rewind() {
        iter.rewind();
    }
    void fast_forward() {
        iter.fast_forward();
    }
    void forward() {
        iter.forward();
    }
    void backward() {
        iter.backward();
    }
    int eol() {
        return iter.eol();
    }
};

#undef thdr
#undef tcls
#endif
