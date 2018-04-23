
/*
 * $Header: g:/dwight/repo/dwcls/rcs/leak.cpp 1.10 1997/06/01 04:40:12 dwight Stable095 $
 */

#include "leak.h"

ALLRET_alloc<LEAK_POINTER> *LEAK_POINTER::pool;

void
LEAK_POINTER::init_pool(int nhandles)
{
	if(pool == 0)
		pool = new ALLRET_alloc<LEAK_POINTER>(nhandles);
}

LeakImp::LeakImp(int mapsize) : Allocator(Default_alloc), ptrmap(mapsize)
{
	mapparm = mapsize;
}

LeakImp::LeakImp(Allocator *a, int mapsize) : Allocator(a), ptrmap(mapsize)
{
     mapparm = mapsize;
}

LeakImp::~LeakImp()
{

}

ALLRET&
LeakImp::alloc(u_long bytes)
{
	ALLRET& tmp = container->alloc(bytes);
	LEAK_POINTER& lp = *new LEAK_POINTER(this, tmp);

	ptrmap.add(&lp);

    return lp;
}

void
LeakImp::free(ALLRET& p)
{
	LEAK_POINTER& lp = (LEAK_POINTER&)p;

	if(ptrmap.del(&lp))
	{
		container->free(lp.user_ptr);

		delete &lp;
	}
	else // unsafe free, we may not ever make it here without dropping core
	{
        bad_frees.append(&lp);
    }
}

Allocator *
LeakImp::make_one_like() const
{
	return new LeakImp(container->make_one_like(), mapparm);
}


u_char *
LeakImp::start_addr()
{
	return container->start_addr();
}

void *
LeakImp::map_to_phys(const ALLRET& p)
{
	return container->map_to_phys(p);
}
