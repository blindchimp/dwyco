
/*
 * $Header: g:/dwight/repo/dwcls/rcs/gaskbrk.h 1.10 1997/06/01 04:40:07 dwight Stable095 $
 */
#ifndef GASKBRK_H
#define GASKBRK_H

#include "allret.h"
#include "ialloc.h"

//
// This class implements a unix "brk"-like functionality on top of
// it's containing allocator. It simply does one giant allocation
// at the beginning of the world and doles it out until it is all gone.
// This is mainly used to retrofit unix-style allocators that use
// sbrk, and expect it to return contiguous memory.
//
// The operations get a bit warped:
//
// alloc -> extending the break, so sbrk(x>=0) should map to alloc
// free -> decreasing the break, so sbrk(x<0) maps to free plus a special
// call to get the current break;
//


#define DEFAULT_GASKBRK_SIZE (1L << 20) // 1 MEG

class GaskBrkImp : public Allocator
{
private:
	ALLRET& arena;
	char  *c_arena; // cached arena pointer, should never change
	char  *cbrk;
    u_long size;

public:
	GaskBrkImp(u_long pool_size = DEFAULT_GASKBRK_SIZE);
	GaskBrkImp(Allocator *a, u_long pool_size = DEFAULT_GASKBRK_SIZE);
    ~GaskBrkImp();

	virtual ALLRET& alloc(u_long byte_count);
	virtual void free(ALLRET&);
	virtual void *map_to_phys(const ALLRET&);

    // functionality unique to this class
	void *sbrk(long);
};


#endif
