
/*
 * $Header: g:/dwight/repo/dwcls/rcs/tvec.cpp 1.9 1997/06/01 04:40:27 dwight Stable095 $
 */
#include "dwvec.h"
#include "dwbag.h"

//typedef DwBag<int> a;
//int i = int();

//typedef DwBag<DwVec<int> > b;
typedef DwVec<int> vi;
typedef DwVecIter<int> vii;

vi abc;
vii iabc(&abc);

DwVec<int> def;
DwVecP<int> foo;
DwVecP<float> floatbar;
DwVecPIter<int> mumble(&foo);
DwVecPIter<float> fi(&floatbar);

main(int, char **)
{
	mumble.rewind();
	int *a = mumble.getp();
}
