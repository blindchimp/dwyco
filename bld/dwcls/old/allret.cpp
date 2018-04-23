
/*
 * $Header: g:/dwight/repo/dwcls/rcs/allret.cpp 1.10 1997/06/01 04:39:33 dwight Stable095 $
 */

#include "ialloc.h"
#include "allret.h"
#include "aalloc.h"

ALLRET::ALLRET(Allocator *a)
{
	from = a;
}

ALLRET::operator void *() const
{
	return from->map_to_phys(*this);
}

ALLRET_alloc<POINTER> *POINTER::pool;

void
POINTER::init_pool(int nhandles)
{
	if(pool == 0)
		pool = new ALLRET_alloc<POINTER>(nhandles);
}
