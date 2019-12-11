
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWVEC_H
#define DWVEC_H
//***********************
// WARNING!
// if you don't have ctors for classes when this is undefined,
// then you doin't get intialization of vector elements.
// unfortunately, gcc can't handle this at the moment...
//**********************
// NOTE: define DWVEC_DOINIT for cdc32, otherwise it
// won't work. you get a small performance boost
// by leaving it out in some cases (like vc doesn't need it.)
// NOTE: vc *does* need it in some cases (like for the vector user type, just
// not for all the vectors used for parameter passing and stuff).
// um, don't think that is true, as long as there is a ctor, the tail
// gets initialized by new. and all the user vectors have ctors too.
// eventually, it might be nice to select how you want it done
// at run time (or maybe with a different template or something.)

// the "noinit" arg to the ctor will do this, but it is probably
// a good idea to only use it for fundamental types that don't have
// reasonable default ctors.
// CDC-X needs this defined, otherwise DwVecP's don't get 0's on
// initialization.
#undef DWVEC_DOINIT
#define DWVEC_DOINIT
#ifndef DWVEC_DOINIT
// note: this doesn't do some error checking that
// is good to do while debugging.
// it is mainly used in order to make crude gc
// a bit faster.
#define DWVEC_FAST_APPEND
#endif

// note: undefine these for debugging.
// nice performance boost with them
// defined.

#if 0
#undef DWVEC_INLINES
#undef DWVEC_NO_INDEX_CHECK
#else
#define DWVEC_INLINES
#define DWVEC_NO_INDEX_CHECK
#endif
#include <stdlib.h>
#include "useful.h"
#include "dwiter.h"
#undef index
void oopanic(const char *);

// non-polymorphic vector (vector of objects.)
//
// if you want to put polymorphic objects in this container, you
// must specify pointers to the objects. it is then your
// responsibility to implement the desired copy semantics (the
// package will only assign the pointers...)  also, auto-enlarged
// arrays of pointers result in uninitialized pointer values.
//
//
// note: tried to do this using templates with fixed arguments, but
// all three of the compilers I have access to screw it up...
// template<class T, int is_fixed, int auto_expand>
//
// (though my problems with borland may have been due to lack of
// prototype causing the compiler to GPF. will have to try
// again at a later date...)
//

template<class T> class DwVecIter;

#ifndef DWVEC_DEFAULTBLOCKSIZE
#define DWVEC_DEFAULTBLOCKSIZE 4
#endif
#define DWVEC_FIXED 1
#define DWVEC_AUTO_EXPAND 1

template<class T>
class DwVec
{
protected:
    long count;			// count the user sees
    T *values;
    int is_fixed;		// true means size is fixed at create time
    int auto_expand;	// true means refs >= count cause expansion
    long blocksize;
    long alloced_count;	// total number of allocated entries
    int noinit;

public:
#ifdef DWVEC_DOINIT
    void (*initfun)(T&);
#endif
    typedef void (*Funcp)(T&);

    DwVec(long = 0, int is_fixed = 0, int auto_expand = 1,
          long = DWVEC_DEFAULTBLOCKSIZE,
          void (*)(T&) = 0,
          int noinit = 0
         );
    virtual ~DwVec();

    DwVec(const DwVec&);
    DwVec& operator=(const DwVec&);
    int operator==(const DwVec&) const;
    int operator!=(const DwVec& t) const {
        return !(*this == t);
    }

#ifdef DWVEC_INLINES
    inline const T& operator[](long count) const;
    inline T& operator[](long count);
#else
    const T& operator[](long count) const;
    T& operator[](long count);
#endif
    T get(int i) const {
        return (*this)[i];
    }
    const T& ref(int i) const {
        return (*this)[i];
    }

    long num_elems() const {
        return count;
    }
    void set_size(long count);
    inline void append(const T&);
    void insert(const T&, long);
    void spread(long idx, long cnt = 1);
    void del(long idx, long cnt = 1);
    long index(const T&) const;
    int contains(const T&) const;
    void apply(typename DwVec<T>::Funcp f);
    virtual T get_by_iter(DwIter<DwVec, T> *a) const ;
#ifdef DWVEC_DOINIT
    void init_value(T& v) {
        if(initfun) (*initfun)(v);
        else v = T();
    }
#endif
};


template<class T>
DwVec<T>::DwVec(const DwVec<T>& vec)
{
    count = vec.count;
    is_fixed = vec.is_fixed;
    auto_expand = vec.auto_expand;
    blocksize = vec.blocksize;
    alloced_count = count + blocksize - (count % blocksize);
    noinit = vec.noinit;
    values = new T[alloced_count];
    long real_count = alloced_count;

    long i;
    for(i = 0; i < count; ++i)
        values[i] = vec.values[i];
#ifdef DWVEC_DOINIT
    if(!noinit)
    {
        initfun = vec.initfun;
        for(i = count; i < real_count; ++i)
            init_value(values[i]);
    }
#endif

}

template<class T>
DwVec<T>&
DwVec<T>::operator=(const DwVec<T>& vec)
{
    if(this != &vec)
    {
        delete [] values;
        count = vec.count;
        is_fixed = vec.is_fixed;
        auto_expand = vec.auto_expand;
        blocksize = vec.blocksize;
        alloced_count = count + blocksize - (count % blocksize);
        noinit = vec.noinit;
        values = new T[alloced_count];

        int i;
        for(i = 0; i < count; ++i)
            values[i] = vec.values[i];
#ifdef DWVEC_DOINIT
        if(!noinit)
        {
            initfun = vec.initfun;
            for(i = count; i < alloced_count; ++i)
                init_value(values[i]);
        }
#endif
    }

    return *this;

}


template<class T>
DwVec<T>::DwVec(long icount, int fixed, int aexp, long blksize,
                void (*ifun)(T&),
                int anoinit
               )
{
    long real_count;			/* real allocation count */

    count = icount;
    blocksize = blksize;
    noinit = anoinit;

    alloced_count = real_count = count + blocksize - (count % blocksize);
    values = new T[real_count];

#ifdef DWVEC_DOINIT
    if(!noinit)
    {
        initfun = ifun;
        for(int i = 0; i < real_count; ++i)
            init_value(values[i]);
    }
#endif
    is_fixed = fixed;
    auto_expand = aexp;
}

template<class T>
DwVec<T>::~DwVec()
{
    delete [] values;
}


template<class T>
#ifdef DWVEC_INLINES
inline
#endif
const T&
DwVec<T>::operator[](long index) const
{
#ifndef DWVEC_NO_INDEX_CHECK
    if(index < 0)
        oopanic("negative index");
    if(index >= count)
        oopanic("index too big");
#endif
    return values[index];
}


template<class T>
#ifdef DWVEC_INLINES
inline
#endif
T&
DwVec<T>::operator[](long index)
{
#ifndef DWVEC_NO_INDEX_CHECK
    if(index < 0)
        oopanic("negative index2");
    if(is_fixed)
    {
        if(index >= count)
            oopanic("idx too big2");
    }
    else
    {
        if(index >= count)
        {
            if(auto_expand)
                set_size(index + 1);
            else
                oopanic("idx too big3");
        }
    }
#else
    if(index >= count && auto_expand)
        set_size(index + 1);
#endif

    return values[index];
}

//
// Note: because of preallocation, destructors
// may not be called when an array is downsized (via del, for example.)
// When the storage is actually reclaimed, the dtors are called, but
// this may not happen until the entire vector is destructed.
//
template<class T>
void
DwVec<T>::set_size(long new_count)
{
    long old_count;			/* old array count */
    long old_alloc_count;	/* old alloc allocation count */
    long new_alloc_count;	/* new alloc allocation count */

    if(is_fixed)
        oopanic("set size on fixed");
    if (new_count < 0) /* check argument */
        oopanic("setsize neg cnt");

    old_count = count; /* save old count */
    old_alloc_count = alloced_count;
    long lim = old_alloc_count + (old_alloc_count / 2);
    if(new_count > old_alloc_count && new_count < lim)
    {
        // boost the allocation by 50% of the existing
        // size. this will cause a lot of extra work
        // for initializing the tail in large allocations, but
        // it saves a lot of thrashing in the allocator
        // for fast growing items that can't be pre-allocated
        new_alloc_count = lim;
    }
    else if(new_count > old_alloc_count)
        new_alloc_count = new_count + blocksize - (new_count % blocksize);
    else
        new_alloc_count = old_alloc_count; // don't let it shrink
    count = new_count;

    if (old_alloc_count != new_alloc_count)
    {
        T *new_values = new T[new_alloc_count];
        long cnt = new_count < old_count ? new_count : old_count;
        long i;
        for(i = 0; i < cnt; ++i)
            new_values[i] = values[i];
        delete [] values;
        values = new_values;
#ifdef DWVEC_DOINIT
        if(!noinit)
        {
            if(new_alloc_count > old_alloc_count)
            {
                for(i = old_count; i < new_alloc_count; ++i)
                    init_value(values[i]);
            }
        }
#endif
        alloced_count = new_alloc_count;
    }
    else if(new_count > old_count)
    {
#ifdef DWVEC_DOINIT
        if(!noinit)
        {
            for(long i = old_count; i < new_count; ++i)
                init_value(values[i]);
        }
#endif
    }
    // note: we know the tail of the array is initialized at "new" time, so no need to
    // fiddle with it here...
}

template<class T>
inline
void
DwVec<T>::append(const T& t)
{
#ifdef DWVEC_FAST_APPEND
    if(count + 1 > alloced_count)
    {
#endif
        set_size(count + 1); // note: set_size changes count
        values[count - 1] = t;
#ifdef DWVEC_FAST_APPEND
    }
    else
    {
        values[count++] = t;
    }
#endif
}

template<class T>
void
DwVec<T>::insert(const T& t, long idx)
{
    if(is_fixed)
        oopanic("insert on fixed vector");
    if(idx == count)
    {
        append(t);
        return;
    }
    if(idx < 0 || (count > 0 && idx >= count))
        oopanic("bad insert");
    set_size(count + 1); // note: set_size changes count
    for(long i = count - 2; i >= idx; --i)
        values[i + 1] = values[i];
    values[idx] = t;
}

template<class T>
void
DwVec<T>::spread(long idx, long cnt)
{
    if(is_fixed)
        oopanic("spread on fixed vector");
    if(idx < 0 || (count > 0 && idx >= count) || cnt < 0)
        oopanic("bad spread");
    set_size(count + cnt); // note: set_size changes count
    long i;
    for(i = count - 1 - cnt; i >= idx; --i)
        values[i + cnt] = values[i];
    if(!noinit)
    {
        for(i = idx; i < idx + cnt; ++i)
#ifdef DWVEC_DOINIT
            init_value(values[i]);
#else
            values[i] = T();
#endif
    }
}

//
// note: this is a bit
// problematic, since it causes elements to
// be assigned to each other as it is deleting
// items. T must support proper "assignment"
// semantics for this to work reasonably.
//
template<class T>
void
DwVec<T>::del(long idx, long delcnt)
{
    if(is_fixed)
        oopanic("del on fix vector");
    if(delcnt == 0)
        return;
    if(idx < 0 || delcnt < 0 || idx >= count || idx + delcnt > count)
        oopanic("bad del");
    // force assignment to default value
    long i;
    if(!noinit)
    {
        for(i = idx; i < idx + delcnt; ++i)
#ifdef DWVEC_DOINIT
            init_value(values[i]);
#else
            values[i] = T();
#endif
    }
    for(i = 0; i < count - idx - delcnt; ++i)
        values[idx + i] = values[idx + i + delcnt];
    set_size(count - delcnt);
}

template<class T>
int
DwVec<T>::contains(const T& t) const
{
    return index(t) != -1;
}

template<class T>
long
DwVec<T>::index(const T& t) const
{
    for(long i = 0; i < count; ++i)
    {
        if(t == values[i])
            return i;
    }
    return -1;
}

template<class T>
int
DwVec<T>::operator==(const DwVec<T>& v2) const
{
    if(count != v2.count)
        return 0;
    for(long i = 0; i < count; ++i)
        if(!(values[i] == v2.values[i]))
            return 0;
    return 1;
}

template<class T>
void
DwVec<T>::apply(typename DwVec<T>::Funcp fun)
{
    for(long i = 0; i < count; ++i)
        (*fun)(values[i]);
}

template<class T>
class DwVecIter : public DwIter<DwVec<T>, T>
{
    friend class DwVec<T>;
private:
    long cur;
    long max;

public:
    DwVecIter(const DwVec<T> *t) : DwIter<DwVec<T>, T>(t) {
        cur = 0;
        max = t->num_elems();
    }
    //note: gcc 3.4.2 has become fussy wrt the "to_iterate" member in the
    // base class. overspecifying fixes it, and probably won't hurt other
    // compilers.
    void init() {
        cur = 0;
        max = DwIter<DwVec<T>, T>::to_iterate->num_elems() - 1;
    }
    void rewind() {
        cur = 0;
    }
    void fast_forward() {
        cur = DwIter<DwVec<T>, T>::to_iterate->num_elems() - 1;
    }
    void forward() {
        if(!(cur >= max)) ++cur;
    }
    void backward() {
        if(!(cur < 0)) --cur;
    }
    int eol() {
        return cur < 0 || cur >= max;
    }
};


template<class T>
T
DwVec<T>::get_by_iter(DwIter<DwVec<T>, T> *a) const
{
    return (*this)[((DwVecIter<T> *)a)->cur];
}

#endif
