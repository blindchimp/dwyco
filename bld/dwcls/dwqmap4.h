#ifndef DWQMAP4_H
#define DWQMAP4_H

#include "dwiter.h"
#include "dwassoc.h"
#include "dwmaps.h"
#include "dwhash.h"
void oopanic(const char *);

//
// fast closed hashing class for
// use in situations where < 32
// items need to be mapped.
// this can provide performance
// improvements in situations where
// maps must be created and destroyed
// quickly, and probably don't have
// a lot of elements.
//
// has roughly the same interface as the
// more general/slower bag/set/map classes.
//

// weak pointer notes:
// this class never sets "ruffled" because the range
// cells have a fixed address when the object is created
// and the size never changes. if a pointer becomes
// invalid, it can only be because of object destruction
// or some application related actions, so it is up to
// the caller to figure that out.
//

template<class R, class D> class DwQMapIter;

template<class R, class D, int elems>
class DwQMap4 : public DwMaps<R,D>
{
    friend class DwQMapIter<R,D>;

private:
    R dr;	// default range
    D dd;	// default domain

    int count;				// number of elements in table
    int thresh;				// number of elements where ops degrade
    unsigned long deleted;

    void init1(int);
    void setdel(long);
    void setempty(long);
    void setfull(long);
    void initdel(long);

    static long dum;
    static const int rehash32[31];
    static const int rehash16[15];
    static const int rehash8[7];

    const int *rehash;

    long search(const D& key, int& found, long& first_del = dum);

private:
    char rng[elems * sizeof(R)];
    char dom[elems * sizeof(D)];

    R *addr_rng(long i) const {
        return (R*)(rng + i * sizeof(R));
    }

    D *addr_dom(long i) const {
        return (D*)(dom + i * sizeof(D));
    }

    void add_assoc(const D& key, const R& val, long idx) {
        int u = ((this->used|this->prev_used) & (1UL << idx));
        D *d = addr_dom(idx);
        R *r = addr_rng(idx);

        if(u)
        {
            *d = key;
            *r = val;
        }
        else
        {
            new (d) D(key);
            new (r) R(val);
        }
    }

    inline const R& get_rng(long idx) const {
        return *addr_rng(idx);
    }

    inline void set_rng(const R& r, long idx) {
        if((this->used|this->prev_used) & (1UL << idx))
            *addr_rng(idx) = r;
        else
            new(addr_rng(idx)) R(r);
    }

    inline const D& get_dom(long idx) const {
        return *addr_dom(idx);
    }

    void destr_fun() {
        //unsigned long u = used;
        //unsigned long u2 = prev_used;
// WARNING: problems here with long/int on 64 bit stuff
// previous version of this function would be more
// general, but this is faster.
        unsigned long u3 = (this->used | this->prev_used);
        for(int i = 0; u3; ++i)
        {
            static char table[64] =
            {   32, 0, 1,12, 2, 6, 99,13,   3, 99, 7, 99, 99, 99, 99,14,
                10, 4, 99, 99, 8, 99, 99,25,   99, 99, 99, 99, 99,21,27,15,
                31,11, 5, 99, 99, 99, 99, 99,   9, 99, 99,24, 99, 99,20,26,
                30, 99, 99, 99, 99,23, 99,19,  29, 99,22,18,28,17,16, 99
            };

            unsigned int x = u3;
            x = (x & -x)*0x0450FBAF;
            int sft = table[x >> 26];
            // note: can't do it in one sft+1 shift
            // because sft might be 31, and shifting
            // 32 is ignored (on an x86 anyway).
            u3 >>= sft;
            u3 >>= 1;
            i += sft;
            addr_rng(i)-> ~ R ();
            addr_dom(i)-> ~ D ();
        }
    }

    void copy(const DwQMap4<R,D,int>& s) {
        unsigned long u = s.used;
        for(int i = 0; u && i < s.size; ++i)
        {
            if(u & 1)
            {
                new (addr_dom(i)) D(*s.addr_dom(i));
                new (addr_rng(i)) R(*s.addr_rng(i));
            }
            u >>= 1;
        }
    }

protected:
    unsigned long used;
    // for finalization, we need where *any* slot
    // might need to be destroyed, across multiple
    // "fast_clears"
    unsigned long prev_used;
    int size;			// total size of table

public:
    typedef DwIter<DwMaps<R,D> , DwAssocImp<R,D> > Foo;
    DwAssocImp<R,D> get_by_iter(Foo *) const;
    DwQMap(const R& defr, const D& defd);
    DwQMap();
    // note: default ctor, op= work ok.
    ~DwQMap();

    int num_elems() const;
    int contains(const D&);
    int find(const D&, R& out, R** wp = 0);
    void add(const D&, const R&);
    int replace(const D&, const R&, R** wp = 0);
    R get(const D&);
    int del(const D&);
    void fast_clear();
    DwMapsIter<R,D> *make_iter() const;

    int over_threshold() const {
        return count >= thresh;
    }
};

#define thdr template<class R, class D, int nelems>
#define tcls DwQMap<R,D,int>

thdr
long tcls::dum = 0;

thdr
const int tcls::rehash32[31] = {
    1, 16, 8, 4, 18, 9, 20, 26, 13, 6, 19, 25, 28, 30, 31, 15, 7, 3, 17,
    24, 12, 22, 27, 29, 14, 23, 11, 21, 10, 5, 2
};

thdr
const int tcls::rehash16[15] = {
    1, 8, 4, 2, 9, 12, 6, 11, 5, 10, 13, 14, 15, 7, 3
};

thdr
const int tcls::rehash8[7] = {
    5, 1, 2, 4, 3, 6, 7
};

thdr
tcls::DwQMap4()
{
    init1(elems);
}

thdr
void
tcls::init1(int maxsz)
{
    if(maxsz != 8 && maxsz != 16 && maxsz != 32)
        oopanic("bad qmap3 size");
    if(maxsz == 32)
        rehash = rehash32;
    else if(maxsz == 16)
        rehash = rehash16;
    else
        rehash = rehash8;
    count = 0;
    initdel(maxsz);
    size = maxsz;
    thresh = (9 * size) / 10;
}

thdr
tcls::DwQMap4(const R& r, const D& d)
    : dd(d), dr(r)
{
    init1(elems);
}

thdr
tcls::~DwQMap4()
{
    destr_fun();
}

thdr
inline int
tcls::num_elems() const
{
    return count;
}


thdr
int
tcls::contains(const D& key)
{
    int found;
    search(key, found);
    return found;
}

thdr
int
tcls::find(const D& key, R& out, R** wp)
{
    int found;
    long idx = search(key, found);
    if(!found)
    {
        return 0;
    }
    out = get_rng(idx);
    if(wp)
        *wp = addr_rng(idx);
    return 1;
}

thdr
void
tcls::add(const D& key, const R& value)
{
    int found;
    long first_del;
    long idx = search(key, found, first_del);
    if(found)
        return;
    if(idx < 0 && first_del < 0)
        oopanic("qmap4 bad add");

    if(first_del != -1)
        idx = first_del;
    add_assoc(key, value, idx);
    setfull(idx);
    ++count;
}

thdr
int
tcls::replace(const D& key, const R& value, R** wp)
{
    int found;
    long first_del;
    long idx = search(key, found, first_del);
    if(idx < 0 && first_del < 0)
        oopanic("qmap4 bad replace");
    if(!found)
    {
        if(first_del != -1)
            idx = first_del; // use first deleted
        ++count;
        add_assoc(key, value, idx);
    }
    else
        set_rng(value, idx);
    setfull(idx);
    if(wp)
        *wp = addr_rng(idx);
    return found;
}

thdr
R
tcls::get(const D& key)
{
    int found;
    long idx = search(key, found);
    if(!found)
        return dr;
    return get_rng(idx);
}

thdr
int
tcls::del(const D& key)
{
    int found;
    long idx = search(key, found);
    if(!found)
        return 0;
    setdel(idx);
    setempty(idx);
    --count;
    return 1;
}

thdr
void
tcls::fast_clear()
{
    count = 0;
    deleted = 0;
    prev_used |= used;
    used = 0;
}

thdr
void
tcls::initdel(long isize)
{
    deleted = 0;
    used = 0;
    prev_used = 0;
}

thdr
inline void
tcls::setdel(long idx)
{
#ifdef CAREFUL
    if(idx < 0 || idx >= size)
        oopanic("bad del idx");
#endif
    deleted |= (1UL << idx);
}

thdr
inline void
tcls::setfull(long idx)
{
#ifdef CAREFUL
    if(idx < 0 || idx >= size)
        oopanic("bad full idx");
#endif

    used |= (1UL << idx);
    deleted &= ~(1UL << idx);
}

thdr
inline void
tcls::setempty(long idx)
{
#ifdef CAREFUL
    if(idx < 0 || idx >= size)
        oopanic("bad empty idx");
#endif
    //used &= ~(1UL << idx);
}


thdr
long
tcls::search(const D& key, int& found, long& first_del)
{
    unsigned long hv0 = ::hash(key) % size;
    unsigned long hv = hv0;
    int k = 0;
    first_del = -1;
    do
    {
        if(!(used & (1UL << hv)))
        {
            found = 0;
            return hv;
        }
        if(deleted & (1UL << hv))
        {
            if(first_del == -1)
                first_del = hv;
        }
        else if(get_dom(hv) == key)
        {
            found = 1;
            return hv;
        }

        hv = (hv0 + rehash[k]) % size;
        ++k;
    } while(k < size - 1);
    found = 0;
    return -1;
}


template<class R, class D>
class DwQMapIter : public DwMapsIter<R,D>
{
    friend class DwQMap4<R,D,int>;
private:
    int cur_buck;
    void advance();

public:
    DwQMapIter(const DwQMap<R,D> *t) : DwMapsIter<R,D>(t) {
        init();
    }

    void init() {
        cur_buck = -1;
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
    void forward() {
        advance();
    }
    void backward() {
        forward();
    }
    int eol() {
        return cur_buck >= ((DwQMap4<R,D,int> *)this->to_iterate)->size;
    }
};

thdr
void
DwQMapIter<R,D>::advance()
{
    if(eol())
        return;
    DwQMap4<R,D,int> *toit = (DwQMap4<R,D,int> *)this->to_iterate;
    int i;
    for(i = cur_buck + 1; i < toit->size; ++i)
    {
        if(!(toit->deleted & (1UL << i)) && (toit->used & (1UL << i)))
            break;
    }
    cur_buck = i;
}

thdr
DwAssocImp<R,D>
tcls::get_by_iter(DwIter<DwMaps4<R,D,int>, DwAssocImp<R,D> > *a) const
{
    typedef DwQMapIter<R,D> dmi_t;
    dmi_t *t = (dmi_t *)a;
    if(t->eol())
        return DwAssocImp<R,D>(dr, dd);
    DwAssocImp<R,D> ret(get_rng(t->cur_buck), get_dom(t->cur_buck));
    return ret;
}

thdr
DwMapsIter<R,D> *
tcls::make_iter() const
{
    return new DwQMapIter<R,D>(this);
}


#undef tcls
#undef thdr
#if 0
// -------- lazy variants

// lazy with storage statically alloced in object
template<class R, class D, int elems>
class DwQMapLazyC : public DwQMapLazy<R,D>
{
private:
    char rng[elems * sizeof(R)];
    char dom[elems * sizeof(D)];

    R *addr_rng(long i) const {
        return (R*)(rng + i * sizeof(R));
    }
    D *addr_dom(long i) const {
        return (D*)(dom + i * sizeof(D));
    }


public:
    DwQMapLazyC(const R& r, const D& d) : DwQMapLazy<R,D>(r, d, elems) {}
    DwQMapLazyC() : DwQMapLazy<R,D>(elems) {}
    DwQMapLazyC(const DwQMapLazyC& s) {
        if(elems < s.size) oopanic("bad lazy ctor");
        copy(s);
    }
    const DwQMapLazyC& operator=(const DwQMapLazyC& s) {
        if(elems < s.size) oopanic("bad lazy op=");
        if(this != &s)
        {
            this->destr_fun();
            copy(s);
        }
        return *this;
    }
    ~DwQMapLazyC() {
        this->destr_fun();
    }
};
#endif


#undef thdr
#undef tcls


#endif
