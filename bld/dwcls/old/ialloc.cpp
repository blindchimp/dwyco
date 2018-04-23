
/*
 * $Header: g:/dwight/repo/dwcls/rcs/ialloc.cpp 1.10 1997/06/01 04:40:09 dwight Stable095 $
 */

#include "allret.h"
#include "ialloc.h"
#include <stdlib.h>
#include <string.h>


#define dw_min(x, y) (((x) > (y)) ? (y) : (x))

Allocator::Allocator()
{
	container = Default_alloc;
}

Allocator::Allocator(Allocator *c)
{
	container = c;
}

ALLRET&
Allocator::realloc(ALLRET& ptr, u_long nbytes, u_long old_bytes)
{
	
	ALLRET& tmp = alloc(nbytes);

	void *newptr = (void *)tmp;
	void *oldptr = (void *)ptr;

	memcpy(newptr, oldptr, dw_min(nbytes, old_bytes));
	free(ptr);
    return tmp;
}

ALLRET&
Allocator::realloc(ALLRET& ptr, u_long nbytes)
{
	
	ALLRET& tmp = alloc(nbytes);

	void *newptr = (void *)tmp;
	void *oldptr = (void *)ptr;
	u_long old_bytes = ptr.size();

	memcpy(newptr, oldptr, dw_min(nbytes, old_bytes));
	free(ptr);
    return tmp;
}

ALLRET&
InitAlloc::alloc(u_long bytes)
{
	ALLRET& tmp = container->alloc(bytes);
	init_fun(tmp, bytes);
	return tmp;
}
