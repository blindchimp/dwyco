
/*
 * $Header: g:/dwight/repo/dwcls/rcs/ttree.cpp 1.4 1997/06/01 04:40:32 dwight Stable095 $
 */
#include <iostream>
#include <stdlib.h>

#include "dwtree.h"

class DwAllocator;
DwAllocator *Default_alloc;

void
printtree(DwTree<int, int> *a)
{
cout << "TREE-IN-ORDER\n";
	DwTreeIter<int, int> t(a);
	while(!t.eol())
	{
		DwAssocImp<int, int> v = t.get();
		cout << v.get_key() << " ";
		t.forward();
	}
cout << "\n";
cout << "TREE-PRE-ORDER\n";
	DwTreeIterPre<int, int> t2(a);
	while(!t2.eol())
	{
		DwAssocImp<int, int> v = t2.get();
		cout << v.get_key() << " ";
		t2.forward();
	}
cout << "\n";
}

int
main(int, char **)
{

	DwTree<int, int> a(0);

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
	printtree(&a);
	a.del(2);
	printtree(&a);
	a.del(4);
	printtree(&a);
	a.del(5);
	printtree(&a);
	a.del(-1);
	printtree(&a);
	a.del(6);
	printtree(&a);
	exit(0);

	DwTree<int, int> foo(0);
	foo = a;
	printtree(&foo);

	DwTree<int, int> bar(foo);
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
		//cout << k << "\n";

		//cout.flush();
	}
	printtree(&a);

	for(i = 0; i < 1000000; ++i)
	{
		int k = rand() % 1000;
		int del = rand() % 2;
		if(del)
			a.del(k);
		else
			a.add(k, k);
	}
	printtree(&a);

	srand(1);
	for(i = 0; i < 1000000; ++i)
	{
		int k = rand() % 1000000;
		a.add(k, k);
	}
	for(i = 0; i < 1000000; ++i)
	{
    	int k = rand() % 1000000;
		a.del(k);

	}
    printtree(&a);

}
