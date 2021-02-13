
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWITER_H
#define DWITER_H


[[noreturn]] void oopanic(const char *);
// T is the container type, I is the element type
// stored in the container
template<class T, class I>
class DwIter
{
protected:
    const T *to_iterate;

    DwIter(const T *cont);
    virtual ~DwIter() {}

    virtual void init() = 0;
    virtual void rewind() = 0;
    virtual void fast_forward() = 0;
    virtual void forward() = 0;
    virtual void backward() = 0;
    virtual int eol() = 0;
public:
    virtual I get() ;
};


template<class T, class I>
DwIter<T,I>::DwIter(const T *container)
{
    to_iterate = container;
}

template<class T, class I>
I
DwIter<T,I>::get()
{
    if(eol())
        oopanic("get at eol on dwiter");
    return to_iterate->get_by_iter(this);
}

#endif

