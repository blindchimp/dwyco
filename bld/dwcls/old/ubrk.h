
/*
 * $Header: g:/dwight/repo/dwcls/rcs/ubrk.h 1.10 1997/06/01 04:40:28 dwight Stable095 $
 */

//
// Allocator to get memory using the UNIX brk functions.
// There can only be one of these objects at any time. This
// type of memory allocator cannot be embedded in other allocators.
// It is best if this allocator is used as the bottom of the
// allocation hierarchy to allocate large chunks of memory (a brk
// system call can be expensive.)
//
//
#include "ialloc.h"
#include "allret.h"

class UNIXBrkImp : public Allocator
{
private:
	static int is_created;
	static void *brk_when_created;

public:
	UNIXBrkImp();
	~UNIXBrkImp(); 
	virtual ALLRET& alloc(u_long bytes);
	virtual void free(ALLRET&);
	virtual void *map_to_phys(const ALLRET&);
};
