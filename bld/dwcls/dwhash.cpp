#include "dwhash.h"

unsigned long
hash(int a)
{
	return a;
}

unsigned long
hash(long a)
{
	return a;
}

unsigned long
hash(double d)
{
	return (unsigned long)d;
}

unsigned long
hash(void *d)
{
	return (unsigned long)d;
}
