
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tstlst.cpp 1.9 1997/06/01 04:40:25 dwight Stable095 $
 */
#include "dwlisto.h"
#include "dwmap.h"
#include <stdio.h>
#include <iostream.h>
#include "sysnew.h"

Allocator *Default_alloc;
class Int;
struct Int : public InHeap
	 {int i; Int(int j) : InHeap(Default_alloc) {i = j;} operator int() const {return i;} };
long
hash(const Int& i)
{
	return (int)i;
}

main()
{
	POINTER::init_pool();

	SystemNewImp alloc;
	Default_alloc = &alloc;

	DwListO<int> intl;
	int a,b,c;
	
	intl.append(&a);
	intl.append(&b);
	intl.append(&c);
	intl.append(&a);
	a = 5;
	b = 6;
	c = 7;
	intl.prepend(&b);
	intl.prepend(&c);

	Int *ip;

	dwlist_foreach(ip, intl)
		printf("%d\n", (int)*ip);
	intl.remove_first();
	intl.remove_last();
	intl.rewind();
	intl.remove();

}
