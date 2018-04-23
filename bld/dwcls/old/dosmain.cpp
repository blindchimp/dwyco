
/*
 * $Header: g:/dwight/repo/dwcls/rcs/dosmain.cpp 1.10 1997/06/01 04:39:48 dwight Stable095 $
 */

#include <stdio.h>
#include "ialloc.h"
#include "allret.h"
#include "ubrk.h"
#include "fmal.h"
#include "offa.h"
#include "chall.h"
#include "inital.h"
#include "fixall.h"
//#include "leak.h"

Allocator *Default_alloc;

main(int argc, char *argv[])
{
	POINTER::init_pool();
	OA_POINTER::init_pool();
	//LEAK_POINTER::init_pool();

	UNIXBrkImp Root;
	InitAllocConstImp ZeroRoot(&Root, 0);
	Default_alloc = &ZeroRoot;
	FineMallocImp a1(32767);
	OffsetAllocImp oa2(&a1, 8192);
#if 0
	FineMallocImp a2(&a1, 8192);
#endif

	FineMallocImp a3(&oa2, 1024);
	OffsetAllocImp oa1(&a3, 512);
	//LeakImp la(&oa1);
	//ALLRET& x = la.alloc(1);
	//la.free(x);

	ChainAllocImp<OffsetAllocImp> foo(Default_alloc, 1024);

	FixAlloc<10> bar;

	bar.alloc(3);

	foo.alloc(100);


	ALLRET& omem = oa1.alloc(20);
	void *ptr2 = omem;

	ALLRET& mem = a3.alloc(200);

	void *ptr = mem;

	a3.free(mem);
}
