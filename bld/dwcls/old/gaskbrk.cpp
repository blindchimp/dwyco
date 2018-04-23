
/*
 * $Header: g:/dwight/repo/dwcls/rcs/gaskbrk.cpp 1.9 1997/06/01 04:40:06 dwight Stable095 $
 */

#include "gaskbrk.h"

GaskBrkImp::GaskBrkImp(u_long poolsize)
	: Allocator(Default_alloc), arena(container->alloc(poolsize))
{
	c_arena = (char  *)container->map_to_phys(arena);
	cbrk = c_arena;
    size = poolsize;
}

GaskBrkImp::GaskBrkImp(Allocator *a, u_long poolsize)
	: Allocator(a), arena(container->alloc(poolsize))
{
	c_arena = (char  *)container->map_to_phys(arena);
	cbrk = c_arena;
    size = poolsize;
}

GaskBrkImp::~GaskBrkImp()
{
	container->free(arena);
	c_arena = cbrk = 0;
}


ALLRET&
GaskBrkImp::alloc(u_long bytes)
{
	if(cbrk - c_arena + bytes >= size)
		return *new POINTER(0);
	char  *ret = cbrk;
	cbrk += bytes;
	return *new POINTER(ret);
}

void
GaskBrkImp::free(ALLRET& p)
{
::abort();
}

void *
GaskBrkImp::map_to_phys(const ALLRET& p)
{
	return container->map_to_phys(p);
}


void *
GaskBrkImp::sbrk(long incr)
{
	if(incr > 0)
    {
		ALLRET& p = alloc(incr);
		if(p.no_space())
		{
			delete &p;
			return (void *)-1;
		}
		void *ret = (void *)p;
		delete &p;
		return ret;
	}
	else if(incr == 0)
		return cbrk;
	// incr < 0
    void *ret = cbrk;
	cbrk -= incr;
	if(cbrk < c_arena)
		cbrk = c_arena;
	return ret;

}
