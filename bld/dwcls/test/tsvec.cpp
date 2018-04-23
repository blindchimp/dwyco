
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "dwsvec.h"
#include "dwvec.h"

void
oopanic(const char *a)
{
	::abort();
}

void *
operator new(size_t, void *p)
{
	return p;
}

struct foo
{
	int i;
	foo() {i = 0;}
	foo(int b) {i = b;}
	operator int() const {return i;}
};

struct bar
{
	int *i;
	bar() {i = 0;}
	bar(int *b) {i = b;}
	operator int *() const {return i;}
};

//typedef DwSVec<bar> barvec;
//typedef DwSVec<foo> foovec;

typedef DwVec<bar> barvec;
typedef DwVec<foo> foovec;

static void
baz(barvec *pl)
{
}

int
main(int, char **)
{
	foovec a;
	a.set_size(100);
	a.append(1);
	a.append(2);
	a.append(3);

	printf("%d %d %d\n", (int)a.get(0), (int)a.get(1), (int)a.get(2));

	a.set_size(101);

	printf("%d %d %d\n", (int)a.get(0), (int)a.get(1), (int)a.get(2));

	int k[3];
	barvec pr;
	pr.append(&k[0]);
	pr.append(&k[1]);
	pr.append(&k[2]);

	for(int i = 0; i < 1000000 * 1000; ++i)
	{
		barvec pl;
		pl.append(pr.ref(0));
		pl.append(pr.ref(1));
		pl.append(pr.ref(2));
		baz(&pl);
	}
}
