
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWQMAP4_H
#define DWQMAP4_H

#include "dwiter.h"
#include "dwassoc.h"
#include "dwmaps.h"
#include "dwhash.h"
[[noreturn]] void oopanic(const char *);

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

template<class R, class D,int nelems> class DwQMap4Iter;

template<class R, class D, int nelems>
class DwQMap4// : public DwMaps<R,D>
{
    friend class DwQMap4Iter<R,D,nelems>;

private:
    DwQMap4(const DwQMap4&) = delete;
    const DwQMap4& operator=(const DwQMap4&) = delete;

    int count;				// number of elements in table
    int thresh;				// number of elements where ops degrade
    unsigned long deleted;
    unsigned long used;
    int size;			// total size of table

    void init1(int);
    void setdel(long);
    void setfull(long);
    void initdel();

    static long dum;
    static const int rehash32[31];
    static const int rehash16[15];
    static const int rehash8[7];

    const int *rehash;

    long search(const D& key, int& found, long& first_del = dum);

    char rng[nelems * sizeof(R)];
    char dom[nelems * sizeof(D)];

    R *addr_rng(long i) const {
        return (R*)(rng + i * sizeof(R));
    }

    D *addr_dom(long i) const {
        return (D*)(dom + i * sizeof(D));
    }

    void add_assoc(const D& key, const R& val, long idx) {
        int u = ((this->used) & (1UL << idx));
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
        if((this->used) & (1UL << idx))
            *addr_rng(idx) = r;
        else
            new(addr_rng(idx)) R(r);
    }

    inline const D& get_dom(long idx) const {
        return *addr_dom(idx);
    }

    void destr_fun() {
// WARNING: problems here with long/int on 64 bit stuff
// previous version of this function would be more
// general, but this is faster.
        unsigned long u3 = (this->used);
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

    void copy(const DwQMap4<R,D,nelems>& s) {
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

public:
    typedef DwIter<DwQMap4<R,D,nelems> , DwAssocImp<R,D> > Foo;
    DwAssocImp<R,D> get_by_iter(Foo *) const;
    DwQMap4();
    // note: for now, we leave copy and op= disabled since they aren't needed
    ~DwQMap4();

    int num_elems() const;
    int contains(const D&);
    int find(const D&, R& out, R** wp = 0);
    void add(const D&, const R&);
    int replace(const D&, const R&, R** wp = 0);
    R get(const D&);
    int del(const D&);
    DwQMap4Iter<R,D,nelems> *make_iter() const;

    int over_threshold() const {
        return count >= thresh;
    }
};

#define thdr template<class R, class D, int nelems>
#define tcls DwQMap4<R,D,nelems>

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
    init1(nelems);
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
    initdel();
    size = maxsz;
    thresh = (9 * size) / 10;
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
        return R();
    return get_rng(idx);
}

// this probably needs to be fixed: del should
// probably call the dtor for both the key
// and value, or create a new api that deletes
// just the value associated with the key or something
// also, an optimization: if the count drops to zero, you
// can reset the deleted and used items to 0 to speed
// the hash probe up
thdr
int
tcls::del(const D& key)
{
    int found;
    long idx = search(key, found);
    if(!found)
        return 0;
    setdel(idx);
    --count;
    return 1;
}

thdr
void
tcls::initdel()
{
    deleted = 0;
    used = 0;
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

thdr
DwAssocImp<R,D>
tcls::get_by_iter(DwIter<DwQMap4<R,D,nelems>, DwAssocImp<R,D> > *a) const
{
    typedef DwQMap4Iter<R,D,nelems> dmi_t;
    dmi_t *t = (dmi_t *)a;
    if(t->eol())
        return DwAssocImp<R,D>();
    DwAssocImp<R,D> ret(get_rng(t->cur_buck), get_dom(t->cur_buck));
    return ret;
}

thdr
DwQMap4Iter<R,D,nelems> *
tcls::make_iter() const
{
    return new DwQMap4Iter<R,D,nelems>(this);
}


#undef tcls
#define tcls DwQMap4Iter<R,D,nelems>

template<class R, class D, int nelems>
class DwQMap4Iter : public DwIter<DwQMap4<R,D,nelems>, DwAssocImp<R,D> >
{
    friend class DwQMap4<R,D,nelems>;
private:
    int cur_buck;
    void advance();

public:
    DwQMap4Iter(const DwQMap4<R,D,nelems> *t) : DwIter<DwQMap4<R,D,nelems>, DwAssocImp<R,D> >(t) {
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
        return cur_buck >= ((DwQMap4<R,D,nelems> *)this->to_iterate)->size;
    }
};

thdr
void
tcls::advance()
{
    if(eol())
        return;
    DwQMap4<R,D,nelems> *toit = (DwQMap4<R,D,nelems> *)this->to_iterate;
    int i;
    for(i = cur_buck + 1; i < toit->size; ++i)
    {
        if(!(toit->deleted & (1UL << i)) && (toit->used & (1UL << i)))
            break;
    }
    cur_buck = i;
}

#undef thdr
#undef tcls

#endif
