
/*
 * $Header: g:/dwight/repo/dwcls/rcs/ialloc.h 1.11 1997/06/01 04:40:10 dwight Stable095 $
 */
//
// Integrated allocation package
//
// This class hierarchy is based on the idea of embedding memory allocators
// inside each other. For example, there is one system allocator from which
// many other allocators obtain memory and dole out chunks to yet other
// objects and allocators, etc.
//
// When you create an allocator, you specify the allocator that is to contain
// it. Whenever the allocator needs more memory to satify a request, it
// calls the containing allocator for more memory. It then parcels this
// out according to the requests it gets. The nesting can be arbitrarily
// deep.
//
// Some allocators do not provide memory allocation, but other services
// such as leak tracing or memory initialization. These allocators
// usually just pass the requests through to the containing allocator
// while performing their services on the memory before handing the
// pointers back up to the requester.
//
// Each allocator as associated with a generic pointer that has special
// semantics for the allocations returned by the allocator. The allocators
// themselves do not use the special aspects of the pointers, and only
// assume the basic operations are available: cast to (void *) to obtain
// a dereferencable pointer and no_space() to determine if the pointer
// is valid. The lifetime of a (void *) is assumed to be only
// as long as the time before the next call to any memory allocation
// function. By only assuming these two things, the allocators can be
// nested in almost any combination desired.
//
// It is not expected that clients external to this package would use
// the special pointer objects, since it would be cumbersome. In this
// case, the pointers can be converted to (void *) and used normally.
// (except in cases where the allocators used return abstract resources,
// in which case the client has to provide a mapping from the abstract
// resource to whatever it needs.)
//
#ifndef IALLOC_H
#define IALLOC_H
#include "dwobj.h"
#include "allret.h"

#include "useful.h"

class Allocator : public DwObj
{
protected:
	Allocator *container;

public:

	Allocator();
	Allocator(Allocator *container);
	virtual ~Allocator() {}

	virtual ALLRET& alloc(u_long byte_count) = 0;
	virtual ALLRET& realloc(ALLRET&, u_long byte_count);
	virtual ALLRET& realloc(ALLRET&, u_long byte_count, u_long old_byte_count);
	virtual void free(ALLRET&) = 0;
	virtual void free(void *) {::abort();}
	virtual void *map_to_phys(const ALLRET&) = 0;
	virtual unsigned char *start_addr() {return 0;}
	virtual Allocator *make_one_like() const { abort(); return 0; }
    virtual u_int hash() {return (u_int) this;}
};

extern Allocator *Default_alloc;

class InitAlloc : public Allocator
{
protected:
	InitAlloc(Allocator *a) : Allocator(a) {}

public:
	ALLRET& alloc(u_long bytes);
	virtual void init_fun(ALLRET&, u_long bytes) const = 0;
	void free(ALLRET& p) {container->free(p);}
	void *map_to_phys(const ALLRET& p) {return container->map_to_phys(p);}
	u_char *start_addr() {return container->start_addr();}
};

//
// Allocators
//
class VanillaMallocImp : public Allocator
{

};

class StackAllocImp : public Allocator
{

};


class AbstractResourceImp : public Allocator
{

};


#endif
