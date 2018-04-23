
/*
 * $Header: g:/dwight/repo/dwcls/rcs/sysnew.h 1.10 1997/06/01 04:40:19 dwight Stable095 $
 */

//
//
// Use the global "new" operator to get storage.
// Can't be embedded in another allocator.
//
#include "ialloc.h"
#include "allret.h"

class SystemNewImp : public Allocator
{
private:
	static int is_created;

public:
	SystemNewImp();
	~SystemNewImp(); 
	virtual ALLRET& alloc(u_long bytes);
	virtual void free(ALLRET&);
	virtual void *map_to_phys(const ALLRET&);
};
