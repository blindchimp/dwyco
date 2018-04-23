
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tstlsta.cpp 1.9 1997/06/01 04:40:26 dwight Stable095 $
 */
#include "dwlista.h"
#include "dwlist.h"
#include "dwlisto.h"

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

	DwListA<int> intl(0);
	int a,b,c;

	
	intl.append(1);
	intl.append(2);
	intl.append(3);
	intl.append(4);
	a = 5;
	b = 6;
	c = 7;
	intl.prepend(b);
	intl.prepend(c);

	dwlista_foreach(a, intl)
		printf("%d\n", a);
	intl.remove_first();
	intl.remove_last();
	intl.rewind();
	intl.remove();

	DwList<int> intpl;
	DwList<int> intpl2;

	intpl.append(&a);
	intpl2.append(&b);


	intpl = intpl2;

	DwListO<int> intplo;
	DwListO<int> intplo2;

	intplo.append(&a);
	intplo2.append(&b);

	intplo = intplo2;

	DwListO<int> aaa = intplo;

	int k = *aaa.get();

	DwListHC<int> bbb;
    DwListHC<int> ccc;


	

}
