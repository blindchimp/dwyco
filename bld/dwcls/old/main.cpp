
/*
 * $Header: g:/dwight/repo/dwcls/rcs/main.cpp 1.11 1997/06/01 04:40:14 dwight Stable095 $
 */

#include <stdio.h>
#include "ialloc.h"
#include "allret.h"
#include "mswin.h"
#include "fmal.h"
#include "offa.h"
#include "chall.h"
#include "inital.h"
#include "fixall.h"
#include "dwlist.h"
#include "exmap.h"
#include "leak.h"


Allocator *Default_alloc;

main(int argc, char *argv[])
{
	MSWIN_POINTER::init_pool();
	POINTER::init_pool();
	OA_POINTER::init_pool();
    LEAK_POINTER::init_pool();

	MSWindowsGlobalImp Root;
	//InitAllocConstImp RootZero(&Root, 0);
	Default_alloc = &Root;
	
	FineMallocImp a1(32767);
	/*
	FineMallocImp a2(&a1, 8192);
	FineMallocImp a3(&a2, 1024);
	OffsetAllocImp oa1(&a3, 512);
	LeakImp la(&oa1);

	ALLRET& x = la.alloc(1);
	la.free(x);

	ChainAllocImp<OffsetAllocImp> foo(Default_alloc, 1024);

    */
	FixAlloc<(u_long)sizeof(FineMallocImp)> bar;

	bar.alloc(10);


	ALLRET& mem = a1.alloc(200);

	void *ptr = mem;

	a1.free(mem);

    return 0;
}
