
/*
 * $Header: g:/dwight/repo/dwcls/rcs/amap.h 1.10 1997/06/01 04:39:34 dwight Stable095 $
 */
#ifndef AMAP_H
#define AMAP_H

#include "dwmap.h"
#include "allret.h"
#include "ialloc.h"

// Allret map, can be used to retrofit old code into
// this framework. Keeps a map between regular pointers and
// ALLRET cookies, providing a means for code using normal
// pointers to take advantage of the allocator stuff.
//

class AMapImp : public Allocator
{
private:
	DwMap<ALLRET *, void *> map;

public:
	AMapImp(Allocator *map_holder = Default_alloc)
		: Allocator(map_holder), map((ALLRET *)0) {}

	virtual ALLRET& alloc(u_long byte_count);
	virtual void free(ALLRET&);
    virtual void free(void *);
	virtual void *map_to_phys(const ALLRET&);
};

inline unsigned long
hash(const void *& a)
{
	return (unsigned long)a;
}
#endif
