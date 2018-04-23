
/*
 * $Header: g:/dwight/repo/dwcls/rcs/inheap.h 1.11 1997/06/01 04:40:10 dwight Stable095 $
 */
#ifndef INHEAP_H
#define INHEAP_H
// Deriving a class from this class gets you some default allocation
// behavior that is useful when working with the memory allocation
// package. You can specify an allocator when creating an object, and this
// class records the allocator for later use by derived classes.
// So, if you want to do allocations inside the implementation of an object
// using the heap specified during its creation, just say new (heap) Foo.
// If you leave the (heap) part out, you get the default allocator.
// Note that we can't define an operator new that uses the "heap" member since
// it is called before the member has been initialized.
//
// operator delete is disabled for objects derived from this class in case
// bulk destruction is being done. if you want piecemeal freeing, redefine
// operator delete in the class you want to use that style of delete.
//
// the cookie returned from the allocator is tossed by this operator
// new and only the physical (void *) is returned.
//

#include "dwobj.h"
#include "ialloc.h"

class InHeap : public DwObj 
{
private:
//	InHeap() {}

public:
	InHeap(Allocator *a = Default_alloc) {heap = a;}
	virtual ~InHeap() {}
	Allocator *heap;
	void *operator new(size_t bytes, Allocator *a = Default_alloc) {
		ALLRET &tmp = a->alloc(bytes);
		void *p = (void *)tmp;
		delete &tmp;
		return p;
	}
    void operator delete(void *, size_t) {}
};
#endif
