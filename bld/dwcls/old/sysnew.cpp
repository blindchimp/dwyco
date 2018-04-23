
/*
 * $Header: g:/dwight/repo/dwcls/rcs/sysnew.cpp 1.9 1997/06/01 04:40:18 dwight Stable095 $
 */
//
// Use system "new" operator get storage. Can't be embedded
// in another allocator. Can't be destroyed in-toto.
//
#include "sysnew.h"

SystemNewImp::SystemNewImp() : Allocator(0)
{
}

SystemNewImp::~SystemNewImp()
{
}

ALLRET&
SystemNewImp::alloc(u_long bytes)
{
	return *new POINTER(new char [bytes]);
}

void
SystemNewImp::free(ALLRET& space)
{
	POINTER *p = (POINTER*)&space;
	delete [] p->ptr;
	
	delete &space;
}

void *
SystemNewImp::map_to_phys(const ALLRET& a)
{
	return (void *)a;
}
