
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tqmap.cpp 1.9 1997/06/01 04:40:23 dwight Stable095 $
 */
#include <iostream>
#include "dwqmap3.h"
#include "dwamap.h"
using namespace std;

void oopanic(const char *) {::abort();}

struct Int
{
	int i;
	Int(int ii) {i = ii;}
	Int() {i = 0;}
	operator int() const {return i;}
	void *operator new(size_t t, Int *v) {return v;}
	friend ostream& operator<<(ostream& os, const Int& ii);
    unsigned long hashValue() const {
        return i;
    }
};

ostream&
operator<<(ostream& os, const Int& ii)
{
	os << ii.i;
	return os;
}


unsigned long
hash(const int& i)
{
	return i;
}

//unsigned long
//hash(const Int& ii)
//{
//	return ii.i;
//}

unsigned long
hash(const DwAssocImp<Int, Int>& a)
{
	return hash(a.peek_key());
}

int
main()
{
	DwAMap<Int, Int> a(0, 0);


	a.add(1, 100);
	if(a.contains(1) == 0)
		::abort();
	if(a.contains(2) == 1)
		::abort();

	int i;
	for(i = 0; i < 200; ++i)
	{
		a.add(i, i*100);
	}
	if(a.num_elems() != 200)
		::abort();
	for(i = 0; i < 200; ++i)
	{
		if(a.get(i) != i * 100)
			::abort();
	}
	for(i = 0; i < 200; ++i)
	{
		if(a.del(i) == 0)
			::abort();
	}
	if(a.num_elems() != 0)
		::abort();
	// generate some random numbers and see if
	// everything plays that way.

	srand(1);
	for(i = 0; i < 200; ++i)
	{
		a.add(rand(), i);
	}
	srand(1);
	for(i = 0; i < 200; ++i)
	{
		// may bomb if rand generates a dup
		if(a.get(rand()) != i)
			::abort();
	}
	DwAMapIter<Int,Int> it(&a);
	for(; !it.eol(); it.forward())
	{
		DwAssocImp<Int, Int> as = it.get();
		cout << "key = " << as.peek_key() << " val = " << as.peek_value() << "\n";
	}
	srand(1);
	for(i = 0; i < 200; ++i)
	{
		// might bomb is rand returned dup in first 30...
		if(a.del(rand()) == 0)
			::abort();
	}
	cout << "should have nothing\n";
	for(it.rewind(); !it.eol(); it.forward())
	{
		::abort();
		DwAssocImp<Int, Int> as = it.get();
		cout << "key = " << as.peek_key() << " val = " << as.peek_value() << "\n";
	}
	
	if(a.num_elems() != 0)
		::abort();

	a.add(12, 10);
	a.replace(12, 15);
	if(a.get(12) != 15)
		::abort();
	if(a.del(12) == 0)
		::abort();
	Int k;
	if(a.find(12, k))
		::abort();
	
	

}
