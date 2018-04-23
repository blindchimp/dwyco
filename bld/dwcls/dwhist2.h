
/*
 * $Header: g:/dwight/repo/cdc32/rcs/dwhist.h 1.4 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef DWHIST2_H
#define DWHIST2_H
#include "dwvec.h"
// class that encapsulates a histogram

class DwHistogram
{
public:
	DwHistogram(int bins, double upper, double lower = 0);
	virtual ~DwHistogram();

	void add_sample(double d);
	int which_bin(double);
	long get_bin(int);
	int num_bins();
	void clear();

// leave this public so whoever wants to print it
// doesn't have to use whatever io crap is compiled in here.
	DwVec<long> bins;
	double upper;
	double lower;
	double unit;
	long total_samples; // total samples plotted, clipped samples not included in this number
	long over_max_samples;
	long under_min_samples;
	double max_val;
	double min_val;
	double max_clipped; // max value that was thrown away
	double min_clipped; // min value that was thrown away
};

#endif
