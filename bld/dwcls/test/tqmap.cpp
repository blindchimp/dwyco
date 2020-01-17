
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tqmap.cpp 1.9 1997/06/01 04:40:23 dwight Stable095 $
 */
#include <iostream>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include "dwqmap3.h"
#include "dwqmap4.h"
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
        return i * 314;
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

static volatile int Gen;
void
incr(int)
{
    ++Gen;
}

#define RUNTIME 5
void
start_timer()
{
    signal(SIGVTALRM, incr);
    struct itimerval val;
    timerclear(&val.it_value);
    timerclear(&val.it_interval);
    val.it_value.tv_sec = RUNTIME;
    val.it_interval.tv_sec = 0;
    setitimer(ITIMER_VIRTUAL, &val, 0);
}

void
perf1()
{
    DwAMap<Int, Int> a(0, 0);
    int n = 0;
    Gen = 0;
    srand(0);
    for(int i = 0; i < 4; ++i)
        a.add(rand(), 0);
    start_timer();
    while(Gen == 0)
    {
        a.add(n % 4,0);
        ++n;
    }
    printf("%d\n", n / RUNTIME);
}

void
perf2()
{
    //DwAMap<Int, Int> a(0, 0);
    int n = 0;
    Gen = 0;
    srand(0);
    //for(int i = 0; i < 4; ++i)
    //    a.add(rand(), 0);
    start_timer();
    while(Gen == 0)
    {
        DwAMap<Int, Int> a(0, 0);
        //a.add(1, 0);
        //a.add(1, 0);
//        a.add(1, 0);
//        a.add(1, 0);
        ++n;
    }
    printf("%d\n", n / RUNTIME);
}

void
perf3()
{
    DwAMap<Int, Int> a(0, 0);
    int n = 0;
    Gen = 0;
    srand(0);
//    for(int i = 0; i < 20; ++i)
//        a.add(rand(), 0);
    a.add(0, 0);
    a.add(1, 0);
    a.add(2, 0);
    a.add(3, 0);
    a.add(4, 0);
    start_timer();
    while(Gen == 0)
    {
        a.get(n % 4);
        ++n;
    }
    printf("%d\n", n / RUNTIME);
}

void
perf4()
{
    DwQMapLazyC<Int, Int, 32> a;
    int n = 0;
    Gen = 0;
    srand(0);
//    for(int i = 0; i < 20; ++i)
//        a.add(rand(), 0);
    a.add(0, 0);
    a.add(1, 0);
    a.add(2, 0);
    a.add(3, 0);
    a.add(4, 0);
    start_timer();
    while(Gen == 0)
    {
        a.get(n % 4);
        ++n;
    }
    printf("%d\n", n / RUNTIME);
}

void
perf5()
{
    DwQMap4<Int, Int, 32> a;
    int n = 0;
    Gen = 0;
    srand(0);
//    for(int i = 0; i < 20; ++i)
//        a.add(rand(), 0);
    a.add(0, 0);
    a.add(1, 0);
    a.add(2, 0);
    a.add(3, 0);
    a.add(4, 0);
    start_timer();
    while(Gen == 0)
    {
        a.get(n % 4);
        ++n;
    }
    printf("%d\n", n / RUNTIME);
}



int
main()
{
    perf3();
    perf4();
    perf5();
    exit(0);
    perf1();
    perf2();
    exit(0);

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
