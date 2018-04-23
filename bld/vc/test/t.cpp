
// test to see where the hash tables go faster than
// vectors.
#include "vc.h"
#include "dwamap.h"
#include "dwvec.h"

template<class R, class D> class DwAMap;
template<class R, class D> class DwAMapIter;
typedef DwAMap<vc,vc> VMAP;
typedef DwAMapIter<vc,vc> VMAPIter;

;DwVec<vc> v;

int Continue_on_alarm;
class Allocator;
Allocator *Default_alloc;

int T;

void
b()
{
	++T;
}

main(int, char **)
{
#ifdef VEC
	vc v[10];
	v[0] = "abc";
	v[1] = "i";
	v[2] = "n";
	v[3] = "one_two";
	v[4] = "a_b";
	v[5] = "a_b2";
	v[6] = "a_b3";
	v[7] = "a_b6";
	v[8] = "a_b4";
	v[9] = "a_b5";

	int n = 10;
	vc f("a_b");
	for(int i = 0; i < 10000000; ++i)
	{
		for(int j = 0; j < n; ++j)
			if(v[j] == f)
			{
				b();
				break;
			}
	}
#else
	VMAP v;
	v.add(vc("abc"), vcnil);
	v.add(vc("i"), vcnil);
	v.add(vc("n"), vcnil);
	v.add(vc("one_two"), vcnil);
	v.add(vc("a_b"), vcnil);
	v.add(vc("a_b2"), vcnil);
	v.add(vc("a_b3"), vcnil);
	v.add(vc("a_b4"), vcnil);
	v.add(vc("a_b5"), vcnil);
	v.add(vc("a_b6"), vcnil);
	v.add(vc("a_b7"), vcnil);
	v.add(vc("a_b8"), vcnil);
	v.add(vc("a_b9"), vcnil);
	v.add(vc("a_b10"), vcnil);
	v.add(vc("a_b11"), vcnil);
	v.add(vc("a_b12"), vcnil);
	v.add(vc("a_b13"), vcnil);
	vc f("i");
	for(int i = 0; i < 1000000; ++i)
		if(v.contains(f))
			++T;
#endif
}

void
oopanic(char *)
{
}

