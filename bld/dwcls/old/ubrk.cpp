
/*
 * $Header: g:/dwight/repo/dwcls/rcs/ubrk.cpp 1.10 1997/06/01 04:40:28 dwight Stable095 $
 */
//
// UNIX brk(2) allocator, cannot be embedded in another memory
// object. Also, there can only be one of these objects
// created at a time.
//
#include <unistd.h>
#include "ubrk.h"

int UNIXBrkImp::is_created;
void *UNIXBrkImp::brk_when_created;

UNIXBrkImp::UNIXBrkImp() : Allocator(0)
{
	if(is_created)
		abort();
	is_created = 1;
	brk_when_created = sbrk(0);
}

UNIXBrkImp::~UNIXBrkImp()
{
	brk(brk_when_created);
	is_created = 0;
}

ALLRET&
UNIXBrkImp::alloc(u_long bytes)
{
	return *new POINTER(sbrk(bytes));
}

void
UNIXBrkImp::free(ALLRET& space)
{
	delete &space;
}

void *
UNIXBrkImp::map_to_phys(const ALLRET& a)
{
	return (void *)a;
}
