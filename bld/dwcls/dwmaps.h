#ifndef DWMAPS_H
#define DWMAPS_H

#include "dwiter.h"
#include "dwassoc.h"

template<class R, class D> class DwMapsIter;

//
// base class for all manner of map implementations
//

template<class R, class D>
class DwMaps
{
public:

    DwMaps();
    virtual ~DwMaps() {}
    virtual int num_elems() const = 0;
    virtual int contains(const D&) = 0;
    virtual int find(const D&, R& out, R** wp = 0) = 0;
    virtual void add(const D&, const R&) = 0;
    virtual int replace(const D&, const R&, R** wp = 0) = 0;
    virtual R get(const D&) = 0;
    virtual int del(const D&) = 0;
    virtual void fast_clear() = 0;

    typedef DwIter<DwMaps<R,D> , DwAssocImp<R,D> > Foo;
    //virtual DwAssocImp<R,D> get_by_iter(DwIter<DwMaps<R,D>, DwAssocImp<R,D> > *) const {return DwAssocImp<R,D>();}
    virtual DwMapsIter<R,D> *make_iter() const = 0;
    virtual DwAssocImp<R,D> get_by_iter(Foo *) const = 0;

    virtual int under_threshold() const ;
    virtual int over_threshold() const ;
    int ruffled;
};


template<class R, class D>
DwMaps<R,D>::DwMaps()
{
    ruffled = 0;
}

#if 0
template<class R, class D>
DwMaps<R,D>::~DwMaps()
{
}
#endif

template<class R, class D>
int
DwMaps<R,D>::under_threshold() const
{
    return 0;
}

template<class R, class D>
int
DwMaps<R,D>::over_threshold() const
{
    return 0;
}

template<class R, class D>
class DwMapsIter : public DwIter<DwMaps<R,D>, DwAssocImp<R,D> >
{
private:

public:
    DwMapsIter(const DwMaps<R,D> *t) : DwIter<DwMaps<R,D>, DwAssocImp<R,D> > (t) {}
    ~DwMapsIter() {}
    virtual void init() = 0;
    virtual void rewind() = 0;
    virtual void fast_forward() = 0;
    virtual void forward() = 0;
    virtual void backward() = 0;
    virtual int eol() = 0;
};

#endif
