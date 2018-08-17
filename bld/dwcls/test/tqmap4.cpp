
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tqmap.cpp 1.9 1997/06/01 04:40:23 dwight Stable095 $
 */
#include <iostream>
#include <stdlib.h>

#include "dwqmap4.h"
using namespace std;

void oopanic(const char *) {::abort();}
#define TCOUNT 25

struct Int
{
    int i;
    Int(int ii) {i = ii;}
    Int() {i = 0;}
    operator int() const {return i;}
    void *operator new(size_t t, Int *v) {return v;}
    friend std::ostream& operator<<(std::ostream& os, const Int& ii);
    unsigned long hashValue() const {return i*31415;}
};

std::ostream&
operator<<(std::ostream& os, const Int& ii)
{
    os << ii.i;
    return os;
}


unsigned long
hash(const int& i)
{
    return i;
}

unsigned long
hash(const Int& ii)
{
    return ii.i;
}

unsigned long
hash(const DwAssocImp<Int, Int>& a)
{
    return hash(a.peek_key());
}

int
main(int, char **)
{
    DwQMap4<Int, Int, 32> a(0, 0);

    a.add(1, 100);
    if(a.contains(1) == 0)
        ::abort();
    if(a.contains(2) == 1)
        ::abort();

    int i;
    for(i = 0; i < TCOUNT; ++i)
    {
        a.add(i, i*100);
    }
    if(a.num_elems() != TCOUNT)
        ::abort();
    for(i = 0; i < TCOUNT; ++i)
    {
        if(a.get(i) != i * 100)
            ::abort();
    }
    for(i = 0; i < TCOUNT; ++i)
    {
        if(a.del(i) == 0)
            ::abort();
    }
    if(a.num_elems() != 0)
        ::abort();
    // generate some random numbers and see if
    // everything plays that way.

    srand(1);
    for(i = 0; i < TCOUNT; ++i)
    {
        a.add(rand(), i);
    }
    srand(1);
    for(i = 0; i < TCOUNT; ++i)
    {
        // may bomb if rand generates a dup
        if(a.get(rand()) != i)
            ::abort();
    }
    DwQMap4Iter<Int,Int,32> it(&a);
    for(; !it.eol(); it.forward())
    {
        DwAssocImp<Int, Int> as = it.get();
        cout << "key = " << as.peek_key() << " val = " << as.peek_value() << "\n";
    }
    srand(1);
    for(i = 0; i < TCOUNT; ++i)
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
