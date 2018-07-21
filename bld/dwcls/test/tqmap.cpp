
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tqmap.cpp 1.9 1997/06/01 04:40:23 dwight Stable095 $
 */
#include <iostream>
#include "dwqmap3.h"
#include "dwamap.h"
#include <memory.h>
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
  // generate a lot of collisions
    unsigned long hashValue() const {return i & 0x3;}
};

ostream&
operator<<(ostream& os, const Int& ii)
{
	os << ii.i;
	return os;
}

#if 0

unsigned long
hash(const int& i)
{
	return i;
}

template<>
unsigned long
hash(const Int& ii)
{
	return ii.i;
}

template<>
unsigned long
hash(const DwAssocImp<Int, Int>& a)
{
	return hash(a.peek_key());
}
#endif

int
main(int, char **)
{
    DwQMapLazyC<Int, Int, 32> a(0, 0);


	a.add(1, 100);
	if(a.contains(1) == 0)
		::abort();
	if(a.contains(2) == 1)
		::abort();
    int i;
    Int r;
    a.add(5, 200);
    a.del(1);
    if(a.contains(1) == 1)
        ::abort();
    a.add(1, 300);
    if(a.get(5) != 200)
        ::abort();
    if(a.get(1) != 300)
        ::abort();
    for(int j = 0; j < 2; ++j)
    {
    for(i = 0; i < 15; ++i)
	{
        a.replace(i, i*100);
	}
    if(a.num_elems() != 15)
		::abort();
    for(i = 0; i < 15; ++i)
	{

        if(!a.find(i, r) || r != i * 100)
			::abort();
	}
    for(i = 0; i < 7; ++i)
	{
		if(a.del(i) == 0)
			::abort();
	}
    if(a.num_elems() != 8)
		::abort();


    for(i = 7; i < 15; ++i)
    {
        Int r;
        if(!a.find(i, r) || r != i * 100)
            ::abort();
    }
    }
    for(int tst = 0; tst < 10; ++tst)
    {
        DwQMapLazyC<Int,Int,32> &a = *new DwQMapLazyC<Int, Int, 32>(0, 0);
    cout << "TEST " << tst << "\n";
    DwQMapIter<Int,Int> it(&a);
    int m[64];
    memset(m, 0xff, sizeof(m));
    for(it.rewind(); !it.eol(); it.forward())
    {
        DwAssocImp<Int, Int> as = it.get();
        cout << "key = " << as.peek_key() << " val = " << as.peek_value() << "\n";
        m[as.peek_key()] = as.peek_value();
    }

    for(int p = 0; p < 200; ++p)
    {
    srand(p * tst);
    int n = rand() % 28;
    for(i = 0; i < n; ++i)
	{
        int k = rand() % 30;
        a.replace(k, i);
        m[k] = i;

	}
    srand(p * tst);
    for(i = 0; i < n / 2; ++i)
	{
        int k = rand() % 30;
        a.del(k);
        m[k] = -1;
	}
    }


    for(it.rewind(); !it.eol(); it.forward())
	{
		DwAssocImp<Int, Int> as = it.get();
		cout << "key = " << as.peek_key() << " val = " << as.peek_value() << "\n";
        if(m[as.peek_key()] != as.peek_value())
            ::abort();
    }
    for(int mi = 0; mi < 64; ++mi)
    {
        if(m[mi] == -1)
        {
            if(a.contains(mi))
                ::abort();
        }
        else
        {
            if(!a.find(mi, r) || r != m[mi])
                ::abort();
        }

    }
    delete &a;
    }

    exit(0);

	
	

}
