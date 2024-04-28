
/* ===
; This is free and unencumbered software released into the public domain.
;
; For more information, please refer to <http://unlicense.org>
;
; Dwight Melcher
; Dwyco, Inc.
; 
*/
#ifndef DWLISTA_H
#define DWLISTA_H

#include <stdlib.h>
#include "dwiter.h"

// i had to pull this class out of
// DwListA because the Borland C++ compiler wouldn't let
// me reference the type in a friend class...
//
template<class T> class DwListA;
template<class T> class DwListAIter;

template<class T>
class listelem
{
    friend class DwListA<T>;
    friend class DwListAIter<T>;
private:
    listelem(const T& cd) : clnt_data(cd) {}
    T clnt_data;
    listelem *next;
    listelem *prev;
};

template<class T> class DwListAIter;

template<class T>
class DwListA
{
    friend class DwListAIter<T>;
protected:

    int             count;
    listelem<T> *        current;
    listelem<T> *        listheader;
    T def;

    void basic_init(const T& def);
    void do_copy(const DwListA&);

public:

    DwListA(T def);
    DwListA();
    DwListA(const DwListA&);
    virtual ~DwListA();

    virtual DwListA& operator=(const DwListA&);
    virtual int operator==(const DwListA&) const;
    void clear();
    virtual void append(const T&);
    virtual void prepend(const T&);
    T get() const ;
    const T* peek_ptr() const;
    T get_last() const ;
    T get_first() const ;
    T read_back() ;
    T read() ;
    const T& peek_read();
    T* nasty();
    void rewind();
    void fastforward();
    void forward();
    void backward();
    T search(const T& key);
    int exists(const T& key);
    int exists(const T& key) const;
    int exists(const T& key, T& val_out);
    int num_elems() const;
    virtual void insert(const T&);
    virtual int remove();
    virtual int remove_first();
    virtual int remove_last();
    void sort_add(const T&);
    virtual int search_fun(const T& key, const T& elem);
    virtual int search_fun(const T& key, const T& elem) const;
    virtual int sort_fun(const T& from_list, const T& new_elem);
    virtual int eqfun(const T& e1, const T& e2) const;
    int eol() const;

    T get_by_iter(DwIter<DwListA<T>, T> *a) const;

    // dangerous functions, do not use.
    listelem<T> *getpos();
    void setpos(listelem<T> *p);
    //T* setelem(T*) ;
};

template<class T>
int
DwListA<T>::eqfun(const T& e1, const T& e2) const
{
    return e1 == e2;
}

template<class T>
inline
int
DwListA<T>::eol() const
{
    return current == listheader;
}

template<class T>
T
DwListA<T>::get_by_iter(DwIter<DwListA<T>, T> *a) const
{
    return ((DwListAIter<T> *)a)->cur->clnt_data;
}


template<class T>
void
DwListA<T>::basic_init(const T& deflt)
{
    count = 0;
    def = deflt;
    listheader = new listelem<T>(def);
    listheader->prev = listheader->next = listheader;
    current = listheader;
}

template<class T>
void
DwListA<T>::do_copy(const DwListA<T>& l)
{
    int i;
    listelem<T> * elem = l.listheader->next;

    for(i = 0; i < l.count; ++i)
    {
        append(elem->clnt_data);
        elem = elem->next;
    }
}

template <class T>
DwListA<T>::DwListA(T deflt)
{
    basic_init(deflt);
}

template <class T>
DwListA<T>::DwListA()
{
    T tmp = T();
    basic_init(tmp);
}


template <class T>
DwListA<T>::~DwListA()
{
    listelem<T> *        elem;
    listelem<T> *        elem2;

    rewind();
    elem = listheader->next;
    while (count-- > 0)
    {
        if (elem == 0)
            abort();
        elem2 = elem->next;
        delete elem;
        elem = elem2;
    }
    delete listheader;
}


template<class T>
DwListA<T>&
DwListA<T>::operator=(const DwListA<T>& l)
{
    int i;

    rewind();
    for(i = count; i > 0; --i)
        remove();
    do_copy(l);
    return *this;
}

template<class T>
int
DwListA<T>::operator==(const DwListA<T>& l) const
{
    int i;

    if(count != l.count)
        return 0;

    listelem<T> * elem1 = listheader->next;
    listelem<T> * elem2 = l.listheader->next;

    for(i = 0; i < count; ++i)
    {
#if 0
        if(!(elem1->clnt_data == elem2->clnt_data))
#endif
            if(!eqfun(elem1->clnt_data, elem2->clnt_data))
                return 0;
        elem1 = elem1->next;
        elem2 = elem2->next;
    }
    return 1;
}

template<class T>
DwListA<T>::DwListA(const DwListA<T>& l)
{
    basic_init(l.def);
    do_copy(l);
}

template <class T>
void
DwListA<T>::clear()
{
    for(int i = count; i > 0; --i)
        remove_last();
}

template <class T>
void
DwListA<T>::append(const T& data)
{
    listelem<T> *        e;

    e = new listelem<T>(data);
    e->next = listheader;
    e->prev = listheader->prev;
    listheader->prev->next = e;
    listheader->prev = e;
    ++count;
}

template <class T>
void
DwListA<T>::prepend(const T& data)
{
    listelem<T> * e = new listelem<T>(data);

    e->prev = listheader;
    e->next = listheader->next;
    listheader->next->prev = e;
    listheader->next = e;
    ++count;
}

template<class T>
const T&
DwListA<T>::peek_read()
{
    if (current == listheader)
        return def;
    const T& data = current->clnt_data;
    forward();
    return data;
}

template<class T>
T*
DwListA<T>::nasty()
{
    if (current == listheader)
        return 0;
    return &current->clnt_data;
}

template<class T>
T
DwListA<T>::read()
{
    if (current == listheader)
        return def;
    const T& data = current->clnt_data;
    forward();
    return data;
}

template<class T>
T
DwListA<T>::read_back()
{
    if (current == listheader)
        return def;
    const T& data = current->clnt_data;
    backward();
    return data;
}

template<class T>
inline
T
DwListA<T>::get() const
{
    if (current == listheader)
        return def;
    return current->clnt_data;
}

template<class T>
inline
const T*
DwListA<T>::peek_ptr() const
{
    if (current == listheader)
        return &def;
    return &current->clnt_data;
}


template<class T>
T
DwListA<T>::get_first() const
{
    if (count == 0)
        return def;
    return listheader->next->clnt_data;
}


template<class T>
T
DwListA<T>::get_last() const
{
    if (count == 0)
        return def;
    return listheader->prev->clnt_data;
}

template<class T>
inline
void
DwListA<T>::rewind()
{
    current = listheader->next;
}

template<class T>
void
DwListA<T>::fastforward()
{
    current = listheader->prev;
}

template<class T>
inline
void
DwListA<T>::forward()
{
    current = current->next;
}

template<class T>
void
DwListA<T>::backward()
{
    current = current->prev;
}

template<class T>
T
DwListA<T>::search(const T& key)
{
    rewind();
    for(int i = 0; i < count; ++i)
    {
        const T& d = peek_read();
        if (search_fun(key, d))
        {
            backward();
            return d;
        }
    }
    return def;
}

template<class T>
int
DwListA<T>::exists(const T& key)
{
    rewind();
    for(int i = 0; i < count; ++i)
    {
        const T& d = peek_read();
        if (search_fun(key, d))
        {
            backward();
            return 1;
        }
    }
    return 0;
}

template<class T>
int
DwListA<T>::exists(const T& key) const
{
    DwListAIter<T> i(this);
    while(!i.eol())
    {
        const T& d = i.get();
        if (search_fun(key, d))
        {
            return 1;
        }
        i.forward();
    }
    return 0;
}

template<class T>
int
DwListA<T>::exists(const T& key, T& val_out)
{
    rewind();
    for(int i = 0; i < count; ++i)
    {
        const T& d = peek_read();
        if (search_fun(key, d))
        {
            backward();
            val_out = d;
            return 1;
        }
    }
    return 0;
}


template<class T>
int
DwListA<T>::num_elems() const
{
    return count;
}

template<class T>
void
DwListA<T>::insert(const T& data)
{
    listelem<T> * e = new listelem<T>(data);

    e->next = current;
    e->prev = current->prev;
    current->prev->next = e;
    current->prev = e;

    ++count;
}

template<class T>
int
DwListA<T>::remove()
{

    if (current == listheader)
    {
        abort();
    }
    current->prev->next = current->next;
    current->next->prev = current->prev;
    listelem<T> * temp = current;
    current = current->next;

    delete temp;
    --count;
    return 1;
}

template<class T>
int
DwListA<T>::remove_last()
{

    if (count == 0)
    {
        abort();
    }
    if(current == listheader->prev)
        return remove();
    listelem<T> * lastnode = listheader->prev;
    lastnode->prev->next = lastnode->next;
    lastnode->next->prev = lastnode->prev;

    delete lastnode;
    --count;
    return 1;
}

template<class T>
int
DwListA<T>::remove_first()
{

    if (count == 0)
    {
        abort();
    }
    if(current == listheader->prev)
        return remove();
    listelem<T> * firstnode = listheader->next;
    firstnode->prev->next = firstnode->next;
    firstnode->next->prev = firstnode->prev;

    delete firstnode;
    --count;
    return 1;
}

template<class T>
void
DwListA<T>::sort_add(const T& data)
{
    if (count == 0)
    {
        append(data);
        return;
    }
    rewind();
    for(int i = 0; i < count; ++i)
    {
        const T& e = peek_read();
        /*
         * func should return nonzero if data is to be inserted
         * before e in the LIST
         */
        if (sort_fun(e, data))
        {
            backward();
            insert(data);
            return;
        }
    }
    append(data);
}

template<class T>
int
DwListA<T>::search_fun(const T& key, const T& elem)
{
    return key == elem;
}

template<class T>
int
DwListA<T>::search_fun(const T& key, const T& elem) const
{
    return key == elem;
}


template<class T>
int
DwListA<T>::sort_fun(const T& /*from_list*/, const T& /*new_elem*/)
{
    return 1;
}

// these are dangerous functions, don't use unless
// absolutely necessary.
template<class T>
listelem<T> *
DwListA<T>::getpos()
{
    return current;
}

template<class T>
void
DwListA<T>::setpos(listelem<T> *p)
{
    current = p;
}


template<class T>
class DwListAIter : public DwIter<DwListA<T>, T>
{
    friend class DwListA<T>;
private:
    listelem<T> *cur;

public:
    // gcc 3.3+ does name lookup differently, that is why all the
    // this-> stuff is going on here
    DwListAIter(const DwListA<T> *t) : DwIter<DwListA<T>, T>(t) {
        cur = t->listheader->next;
    }
    ~DwListAIter() {}
    void init() {
        cur = this->to_iterate->listheader->next;
    }
    void rewind() {
        cur = this->to_iterate->listheader->next;
    }
    void fast_forward() {
        cur = this->to_iterate->listheader->prev;
    }
    void forward() {
        if(!eol()) cur = cur->next;
    }
    void backward() {
        if(!eol()) cur = cur->prev;
    }
    int eol() {
        return cur == this->to_iterate->listheader;
    }
};

#define dwlista_foreach(e,list) for((list).rewind();!(list).eol() && (((e)=(list).get()), 1);(list).forward())
#define dwlista_foreach_back(e,list) for((list).fastforward();!(list).eol() && (((e)=(list).get()), 1);(list).backward())
#define dwlista_foreach_iter(i, e, list) for((i).rewind();!(i).eol() && (((e)=(i).get()), 1);(i).forward())
#define dwlista_foreach_peek(e,list) for((list).rewind();!(list).eol() && (((e)=(list).peek_ptr()), 1);(list).forward())
#define dwlista_foreach_peek_back(e,list) for((list).fastforward();!(list).eol() && (((e)=(list).peek_ptr()), 1);(list).backward())
#endif
