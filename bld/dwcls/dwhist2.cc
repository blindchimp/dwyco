
/*
 * $Header: g:/dwight/repo/cdc32/rcs/dwhist.cc 1.5 1997/11/25 20:41:03 dwight Stable095 $
 */
#if defined(LINUX) || defined(MINGW) || _MSC_VER >= 1500
#include <limits.h>
#include <float.h>
#define MINDOUBLE DBL_MIN
#define MAXDOUBLE DBL_MAX
#else
#include <values.h>
#endif
#include "dwhist2.h"

DwHistogram::DwHistogram(int nbins, double upr, double lwr)
	: bins(nbins, 1, 0) // no auto-resize, fixed
{
	upper = upr;
	lower = lwr;
	unit = (upr - lwr) / (double)nbins;
	clear();
}

DwHistogram::~DwHistogram()
{
}

void
DwHistogram::clear()
{
	int n = bins.num_elems();
	for(int i = 0; i < n; ++i)
		bins[i] = 0;
	total_samples = 0;
	over_max_samples = 0;
	under_min_samples = 0;
	max_val = MINDOUBLE;
	min_val = MAXDOUBLE;
	max_clipped = MINDOUBLE;
	min_clipped = MAXDOUBLE;
}

void
DwHistogram::add_sample(double s)
{
	if(s < lower)
	{
		++under_min_samples;
		if(s < min_clipped)
			min_clipped = s;
		return;
	} 
	else if(s >= upper)
	{
		++over_max_samples;
		if(s > max_clipped)
			max_clipped = s;
		return;
	}
	++total_samples;
	int bin = which_bin(s);
	if(bin == -1 || bin >= bins.num_elems()) return;
	if(s > max_val)
		max_val = s;
	else if(s < min_val)
		min_val = s;
	bins[bin]++;
}

int
DwHistogram::which_bin(double s)
{
	if(s < lower || s >= upper)
		return -1;
	s -= lower;
	return s / unit;
}

long
DwHistogram::get_bin(int b)
{
	return bins[b];
}

int
DwHistogram::num_bins()
{
	return bins.num_elems();
}
