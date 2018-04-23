
/*
 * $Header: g:/dwight/repo/dwcls/rcs/mswin.cpp 1.10 1997/06/01 04:40:16 dwight Stable095 $
 */
#include "mswin.h"

MSWindowsGlobalImp::MSWindowsGlobalImp() : Allocator(0)
{
}

MSWindowsGlobalImp::~MSWindowsGlobalImp()
{
// release all handles
}

ALLRET&
MSWindowsGlobalImp::alloc(u_long bytes)
{
	MSWIN_POINTER *tmp = new MSWIN_POINTER(GlobalAlloc(GMEM_FIXED, bytes));

	return *tmp;
}

void
MSWindowsGlobalImp::free(ALLRET& a)
{
	MSWIN_POINTER& m = (MSWIN_POINTER&)a;

	if(GlobalUnlock(m.h) == 0)
		GlobalFree(m.h);
	//may want to warn in this case...

}
