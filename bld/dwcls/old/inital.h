
/*
 * $Header: g:/dwight/repo/dwcls/rcs/inital.h 1.10 1997/06/01 04:40:11 dwight Stable095 $
 */

#ifndef INITAL_H
#define INITAL_H
#include "ialloc.h"

//
// This allocator provides initialization of the chunks allocated
// in the containing allocator. The initialization is with a constant
// u_char specified when the allocator is created.
//

class InitAllocConstImp : public InitAlloc
{
private:
	const u_char init_val;

public:
	InitAllocConstImp(u_char val = 0);
	InitAllocConstImp(Allocator *, u_char val = 0);

	void init_fun(ALLRET&, u_long) const;
	Allocator *make_one_like() const;
};

#endif
