
/*
 * $Header: g:/dwight/repo/dwcls/rcs/ttree.cpp 1.4 1997/06/01 04:40:32 dwight Stable095 $
 */
#include <iostream.h>
#include <stdlib.h>

#include "dwtree2.h"
class DwAllocator;
DwAllocator *Default_alloc;

printtree(DwTreeKaz<int, int> *a)
{
cout << "TREE-IN-ORDER\n";
	DwTreeKazIter<int, int> t(a);
	while(!t.eol())
	{
		DwAssocImp<int, int> v = t.get();
		cout << "(" << v.get_key() << "," << v.get_value() << ") ";
		t.forward();
	}
cout << "\n";
}

main()
{

	DwTreeKaz<int, int> a(0);

	a.add(1, 1);

	a.add(3, 3);
	a.add(2, 2);
	a.add(4, 4);
	a.add(6, 6);
	a.add(5, 5);
	a.add(-1, -1);
	a.add(-2, -2);
	a.add(-4, -4);
	a.add(-3, -3);
	//a.del(1);
	printtree(&a);
	cout << "\n";
	a.del(3);
	a.del(2);
	a.del(4);
	a.del(5);
	a.del(-1);
	a.del(6);
	printtree(&a);

	DwTreeKaz<int, int> foo(0);
	foo = a;
	printtree(&foo);

	DwTreeKaz<int, int> bar(foo);
	printtree(&bar);
	
	cout << a[2] << "\n";
	a[2] = 2;
	cout << a[2] << "\n";

	int vom = 0;
	cout << "find " << a.find(20, vom) << " exists " << a.exists(30) << "\n";
	cout << "find " << a.find(-4, vom) << " exists " << a.exists(-4) << "\n";
	cout << "vom " << vom << "\n";

	cout << a.num_elems() << "\n";
	cout.flush();
	while(a.num_elems() > 0)
	{
		cout << a.delmin() << "\n";;
		cout.flush();
	}

	for(int i = 0; i < 10000; ++i)
	{
		a.add(rand(), i);
	}
	printtree(&a);

	while(a.num_elems() > 0)
	{
		int k;
		cout << a.delmin(&k) << " ";
		cout << k << "\n";
		
		cout.flush();
	}
	cout << "should be EMPTY\n";
	printtree(&a);
	cout << "END\n";

	for(int i = 0; i < 1000000; ++i)
	//for(int i = 0; i < 1000; ++i)
	{
		int k = rand() % 1000;
		int del = rand() % 2;
		if(del)
			a.del(k);
		else
			a.add(k, k);
	}
	printtree(&a);

}
