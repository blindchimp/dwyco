
/*
 * $Header: g:/dwight/repo/dwcls/rcs/allret.h 1.11 1997/06/01 04:39:33 dwight Stable095 $
 */
//
// This class defines the cookies returned by allocators 
// in this package. The idea is that this class encapsulates
// the semantics of the various types of allocators implemented by the
// system (for example, the handles that come from Windows, the UNIX
// process break, and the offsets from a position independent allocator.)
// Each allocator that has a different sort of pointer derives a class
// from ALLRET and builds from there.
// Each pointer has a back-pointer to the allocator from which it was
// obtained (this isn't used now).
//
// To use the memory associated with an ALLRET, it can be cast to
// (void *). The resulting pointer has a lifetime that is dependent
// on the allocator from which is was obtained, but it is guaranteed to
// be valid until the next allocation operation. Currently there is no
// general way to map from (void *)'s to the ALLRETs they came from, so
// clients must do that mapping themselves (usually by keeping the
// ALLRET objects around.) There is one exception, allocators that use
// the generic POINTER class can map back and forth between (void *)'s
// and POINTERS.
//
// Currently, objects derived from ALLRET are allocated from
// storage set-aside at the start of the program
// (mostly for performance reasons.) Each *type* has a fixed pool of objects.
// The number is determined at pool creation time, and exceeding the
// number allocated results in program termination.
//

#ifndef ALLRET_H
#define ALLRET_H
#include <stdlib.h>
#include "aalloc.h"
#include "dwobj.h"

class Allocator;
extern Allocator *Default_alloc;

class ALLRET : public DwObj
{
friend class UNIXBrkImp;
friend class FineMallocImp;
friend class OffsetAllocImp;
friend class LeakImp;
friend class LEAK_POINTER;
friend class InHeap;
friend class SystemNewImp;
friend class GaskBrkImp;

protected:
	ALLRET(Allocator *from);
	ALLRET() { from = 0; }
	virtual ~ALLRET() {}
	
public:
	Allocator *from; // ack, cuz friends and templates don't work very well
	virtual operator void *() const;
	virtual unsigned long size() { abort(); return 0;}
	virtual int no_space() const = 0;
	virtual unsigned int hash() { return (unsigned int)this; }
};


class POINTER : public ALLRET
{
friend class UNIXBrkImp;
friend class FineMallocImp;
friend class SystemNewImp;

private:
	void *ptr;
	static ALLRET_alloc<POINTER> *pool;
	
public:
	POINTER() : ALLRET(Default_alloc) { ptr = 0; }
	POINTER(void *p) : ALLRET(Default_alloc) { ptr = p; }
	/*
	 * Note: we don't free storage here since we haven't made
	 * any attempt to know where the storage came from (this
	 * class is used by a lot of other allocators.) also
	 * we don't know if it is even safe to free things. it is also
	 * a performance win to be able to get plain, unemcumbered pointers
	 * from this package, which means in a lot of cases, the
	 * POINTER object is simply tossed after the pointer is extracted,
	 * so invalidating the pointer would be impolite.
	 */

	~POINTER() {}
	void *operator new(size_t) {return pool->alloc();}
	void operator delete(void *p) {pool->free((POINTER *)p);}
	virtual operator void *() const {return ptr;}

	static void init_pool(int nhandles = 100);
	int no_space() const { return ptr == 0; }
};


#endif
