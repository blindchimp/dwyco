
/*
 * $Header: g:/dwight/repo/dwcls/rcs/fmal.h 1.11 1997/06/01 04:40:06 dwight Stable095 $
 */
//
// This allocator is useful for situations where allocations tend to be
// frequent and small. This allocator supports variable sized allocations
// up to a fixed maximum determined at object creation time. The lifetime
// of the allocations is assumed to be small as well. There is no
// free-list searching used to satisfy a request, however, frees are
// recorded and when an allocation block contains no active allocations
// it is freed to the containing allocator.
// This is useful for applications that create large numbers of small
// allocations and free them after a short period of time.
// Fragmentation and memory
// utilization will be bad if there are even a few long-lived allocations.
//

#ifndef FMAL_H
#define FMAL_H

#include "ialloc.h"
#define DEFAULT_BLOCKSIZE (8192)

class FineMallocImp : public Allocator
{
private:

#ifdef KEEPLIST
	CList blk_list;
	void dump_list(void);
	void dump_a_bit(char  *, int);
	void check_ptr(char  *p);
#endif
	struct fm_blk
	{
		fm_blk(Allocator *, u_long chunk_size);
		void *operator new(size_t, ALLRET *);
		
		long size;
		long align;
		long num_allocs;
        char *alloc_ptr;
		long space_left;
		ALLRET *mem;
	#ifdef KEEPLIST
		void *backptr;
	#endif
	};

	ALLRET *cur_blk;
	long blocks_out;
	long fm_blocksize;

public:
    FineMallocImp(u_long chunk_size = DEFAULT_BLOCKSIZE);
	FineMallocImp(Allocator *, u_long chunk_size = DEFAULT_BLOCKSIZE);
	~FineMallocImp();

	virtual ALLRET& alloc(unsigned long bytes);
	virtual void free(ALLRET&);
	virtual void *map_to_phys(const ALLRET&);
	Allocator *make_one_like() const { return new FineMallocImp(container, fm_blocksize); }
};


#endif
