
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tstbag.cpp 1.9 1997/06/01 04:40:24 dwight Stable095 $
 */
#include <iostream>

#include "dwbag.h"
#include "dwset.h"
#include "dwmapr.h"
#include "dwstr.h"
using namespace std;

void
oopanic(const char *)
{
    ::abort();
}

class Barf
{
public:
	Barf() {}
	int operator==(const Barf&) const {return 1;}
    unsigned long hashValue() const {
        return (unsigned long)this;
    }
};

class Baz
{
public:
	Baz() {}
	int operator==(const Baz&) const {return 1;}
    unsigned long hashValue() const {
        return (unsigned long)this;
    }
};


unsigned long
hash(const Barf& b)
{
	return 1;
}

unsigned long
hash(const Baz& b)
{
	return 1;
}

unsigned long
hash(const int& i)
{
	return i;
}

int
main(int argc, char *argv[])
{
    DwBag<int> foo(0, 0);
    DwSet<int> bar(0, 0);
	DwMapR<Barf, Barf> baz;
	DwMapR<Baz, Barf> jj;
	DwMapR<Barf, Baz> kk;
	DwMapR<Baz, Baz> ll;
    DwMapR<int, int> mumble;

    int i;
    for(i = 0; i < 20; ++i)
		foo.add(i / 2);

    DwBag<int> foofoo(0, 0);

    foofoo = foo;

	if(foo.num_elems() != 20)
		::abort();

	for(i = 0; i < 20; ++i)
		if(!foo.contains(i / 2))
			::abort();

    for(i = 0; i < 20; ++i)
        if(!foofoo.contains(i / 2))
            ::abort();

    DwBag<int> foofoofoo(foo);

    for(i = 0; i < 20; ++i)
        if(!foofoofoo.contains(i / 2))
            ::abort();

	DwBagIter<int> iter(&foo);
	for(;!iter.eol();iter.forward())
        std::cout << iter.get() << "\n";


	for(i = 0; i < 20; ++i)
		if(!foo.del(i / 2))
			::abort();

    for(i = 0; i < 20; ++i)
        if(!foofoofoo.del(i / 2))
            ::abort();

	if(foo.num_elems() != 0)
		::abort();

    if(foofoofoo.num_elems() != 0)
        ::abort();

	for(i = 0; i < 20; ++i)
		bar.add(i / 2);

	if(bar.num_elems() != 10)
		::abort();

	for(i = 0; i < 20; ++i)
		if(!bar.contains(i / 2))
			::abort();

	DwSetIter<int> iter2(&bar);
	for(;!iter2.eol();iter2.forward())
		cout << iter2.get() << "\n";


	for(i = 0; i < 10; ++i)
		if(!bar.del(i))
			::abort();

	if(bar.num_elems() != 0)
		::abort();

	for(i = 0; i < 20; ++i)
		mumble.add(i, i * 10);
	if(mumble.num_elems() != 20)
		::abort();
	DwMapRIter<int, int> iter3(&mumble);
	for(;!iter3.eol();iter3.forward())
	{
		DwAssocImp<int, int> a = iter3.get();
		cout << a.peek_key() << "," << a.peek_value() << "\n";
	}
	for(i = 0; i < 20; ++i)
	{
    	int out;
		if(!mumble.find(i, out))
			::abort();
		if(mumble.get(i) != i * 10)
			::abort();
		if(out != i * 10)
			::abort();
	}

	for(i = 0; i < 20; ++i)
	{
		if(!mumble.contains(i))
			::abort();
		if(!mumble.del(i))
			::abort();
    }

}



	
