
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tstmap.cpp 1.11 1997/06/01 04:40:26 dwight Stable095 $
 */
#include <stdio.h>
#include "dwmap.h"
#include "allret.h"
#include "sysnew.h"


u_int
hash(const long& l)
{
	return l;
}

Allocator *Default_alloc;

main()
{
	SystemNewImp Root;
	POINTER::init_pool();

    Default_alloc = &Root;

	DwMap<int, long> baz;

	for(int i = 0; i < 100; ++i)
		baz[i] = i * 20;

	for(i = 0; i < 100; ++i)
		if(baz[i] != i * 20)
			::abort();

	printf("v k\n");

	const int *vp;
	const long *kp;

	for(baz.rewind(); baz.peek_get(vp, kp); baz.next())
		printf("%d %ld\n", *vp, *kp);

	for(i = 0; i < 100; ++i)
		if(!baz.contains(i))
			::abort();
	for(i = 100; i < 200; ++i)
		if(baz.contains(i))
			::abort();
	for(i = 0; i < 100; ++i)
		baz.del(i);

	for(i = 0; i < 100; ++i)
		if(baz.contains(i))
			::abort();

}
