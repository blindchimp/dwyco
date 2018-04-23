
/*
 * $Header: g:/dwight/repo/dwcls/rcs/offa.cpp 1.10 1997/06/01 04:40:16 dwight Stable095 $
 */
#include "offa.h"
#include "aalloc.h"

ALLRET_alloc<OA_POINTER> *OA_POINTER::pool;

void
OA_POINTER::init_pool(int nhandles)
{
	if(pool == 0)
		pool = new ALLRET_alloc<OA_POINTER>(nhandles);
}

OffsetAllocImp::OffsetAllocImp(u_long blk_size)
	: Allocator(Default_alloc),
	start(container->alloc(blk_size))
{
	len = blk_size;
    next_alloc = 0;
}

OffsetAllocImp::OffsetAllocImp(Allocator *a, u_long blk_size)
	: Allocator(a),
	start(container->alloc(blk_size))
{
	len = blk_size;
	next_alloc = 0;
}

OffsetAllocImp::~OffsetAllocImp()
{
	container->free(start);
}


ALLRET&
OffsetAllocImp::alloc(u_long bytes)
{
	if(next_alloc + bytes > len)
		return *new OA_POINTER(this, -1);
	long tmp = next_alloc;
    next_alloc += bytes;
	return *new OA_POINTER(this, tmp);
}    	

void
OffsetAllocImp::free(ALLRET& ptr)
{
	delete &ptr;
}

u_char *
OffsetAllocImp::start_addr()
{
	return (u_char *)(void *)start;
}
