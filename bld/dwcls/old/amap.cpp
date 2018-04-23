
/*
 * $Header: g:/dwight/repo/dwcls/rcs/amap.cpp 1.11 1997/06/01 04:39:34 dwight Stable095 $
 */
#include <iostream.h>
#include <fstream.h>
#include "amap.h"


extern int beginning_of_world;

ALLRET&
AMapImp::alloc(u_long byte_count)
{
	ALLRET& tmp = container->alloc(byte_count);
    beginning_of_world = 1;
	map[(void *)tmp] = &tmp;
    beginning_of_world = 0;
	return tmp;
}

void
AMapImp::free(ALLRET& a)
{
	container->free(a);
}

void
AMapImp::free(void *a)
{
	ALLRET *ar = map[a];
	if(ar == NULL)
	{
		static ofstream os("bogus.fre");
		os << hex << a << endl;
		return;
	} // bogus void *

	container->free(*ar);
    beginning_of_world = 1;
	map.del(a);
    beginning_of_world = 0;
}

void *
AMapImp::map_to_phys(const ALLRET& a)
{
	return container->map_to_phys(a);
}


	
