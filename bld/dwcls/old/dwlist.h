
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
 * $Header: g:/dwight/repo/dwcls/rcs/dwlist.h 1.11 1997/06/01 04:39:55 dwight Stable095 $
 */
#ifndef DWLIST_H
#define DWLIST_H
#include <stdlib.h>
#include "dwlista.h"
#ifdef USE_INHEAP
#include "inheap.h"
#endif

#define LIST_ERROR 0		/* unlikely value for a pointer */

#ifdef USE_INHEAP
#define NEW new(heap)
#else
#define NEW new
#define heap 0
#endif

//
// List of pointers, operations return pointers and
// equality is based on what the pointer references, not
// the pointer value.

template<class T>
class DwList : public DwListA<T *>
{
protected:


public:
    DwList(Allocator *a = Default_alloc) : DwListA<T *>(0, a) {}

//	DwList& operator=(const DwList&);
    virtual int eqfun(T* const &e1, T* const &e2) const {
        return *e1 == *e2;
    }
    virtual int search_fun(T* const &key, T* const &elem) {
        return *key == *elem;
    }
    virtual int sort_fun(T* const &from_list, T* const &new_elem) {
        return 1;
    }

};


//
// Hardcore list of pointers
//
// equality is based on pointer values, not what is referenced.
//
template<class T>
class DwListHC : public DwListA<T *>
{
protected:

public:
    DwListHC(Allocator *a = Default_alloc) : DwListA<T *>(0, a) {}
    virtual int search_fun(T * const &key, T * const &elem) {
        return key == elem;
    }
    virtual int sort_fun(T * const &from_list, T * const &new_elem) {
        return 1;
    }
};

template<class T>
class DwListIter : public DwListAIter<T *>
{
public:
    DwListIter(const DwList<T> *t) : DwListAIter<T *>(t) {}
};

#define dwlist_foreach(e,list) for((list).rewind();(((e)=(list).get())!=LIST_ERROR);(list).forward())
#define dwlist_foreach_back(e,list) for((list).fastforward();(((e)=(list).get())!=LIST_ERROR);(list).backward())
#undef NEW
#undef heap
#endif
