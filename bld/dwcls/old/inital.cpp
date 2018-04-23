
/*
 * $Header: g:/dwight/repo/dwcls/rcs/inital.cpp 1.10 1997/06/01 04:40:10 dwight Stable095 $
 */
#include "inital.h"
#include <string.h>

InitAllocConstImp::InitAllocConstImp(u_char val)
	: init_val(val),
    InitAlloc(Default_alloc)
{
}

InitAllocConstImp::InitAllocConstImp(Allocator *a, u_char val)
	: init_val(val),
	InitAlloc(a)
{
}

void
InitAllocConstImp::init_fun(ALLRET& tmp, u_long bytes) const
{
	void *p = tmp;
	memset(p, init_val, bytes);
}

Allocator *
InitAllocConstImp::make_one_like() const
{
	return new InitAllocConstImp(container, init_val);
}
