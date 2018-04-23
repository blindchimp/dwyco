
/*
 * $Header: g:/dwight/repo/dwcls/rcs/offa.h 1.10 1997/06/01 04:40:17 dwight Stable095 $
 */
#ifndef OFFA_H
#define OFFA_H

#include "ialloc.h"
#include "allret.h"
#include "aalloc.h"

#define DEFAULT_OFFA_SIZE (1024)

//
// This allocator provides variable size allocations inside a fixed size
// pool. The pointers returned are position independent, so the allocator
// can be moved in memory, and the extant pointers re-bound to the new
// pool by simply recreating the pointer with a new allocator.
//

class OffsetAllocImp : public Allocator
{
private:
	ALLRET& start;
    u_long next_alloc;
    u_long len;

public:
	OffsetAllocImp(u_long blk_size = DEFAULT_OFFA_SIZE);
	OffsetAllocImp(Allocator *, u_long blk_size = DEFAULT_OFFA_SIZE); 
	~OffsetAllocImp();

	virtual ALLRET& alloc(u_long byte_count);
	virtual void free(ALLRET&);
	virtual unsigned char *start_addr();
	virtual void *map_to_phys(const ALLRET&) { return 0;}
	Allocator *make_one_like() const {return new OffsetAllocImp(container, len);}
};


class OA_POINTER : public ALLRET
{
friend class OffsetAllocImp;
private:
	long index;
    static ALLRET_alloc<OA_POINTER> *pool;

public:
	static void init_pool(int nhandles = 100);
	OA_POINTER() : ALLRET(0) {index = -1;}
	OA_POINTER(long i) : ALLRET(Default_alloc) {index = i;}
	OA_POINTER(Allocator *f, long i) : ALLRET(f) {index = i;}
	~OA_POINTER() {}
	operator void *() const {return (void *)(from->start_addr() + index); }
    int no_space() const {return index == -1;}

};


#endif
