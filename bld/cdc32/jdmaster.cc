
/*
 * $Header: g:/dwight/repo/cdc32/rcs/jdmaster.cc 1.2 1997/11/25 20:41:03 dwight Stable095 $
 */
/*
 * jdmaster.c
 *
 * Copyright (C) 1991-1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains master control logic for the JPEG decompressor.
 * These routines are concerned with selecting the modules to be executed
 * and with determining the number of passes and the work to be done in each
 * pass.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

/*
 * Several decompression processes need to range-limit values to the range
 * 0..MAXJSAMPLE; the input value may fall somewhat outside this range
 * due to noise introduced by quantization, roundoff error, etc.  These
 * processes are inner loops and need to be as fast as possible.  On most
 * machines, particularly CPUs with pipelines or instruction prefetch,
 * a (subscript-check-less) C table lookup
 *		x = sample_range_limit[x];
 * is faster than explicit tests
 *		if (x < 0)  x = 0;
 *		else if (x > MAXJSAMPLE)  x = MAXJSAMPLE;
 * These processes all use a common table prepared by the routine below.
 *
 * For most steps we can mathematically guarantee that the initial value
 * of x is within MAXJSAMPLE+1 of the legal range, so a table running from
 * -(MAXJSAMPLE+1) to 2*MAXJSAMPLE+1 is sufficient.  But for the initial
 * limiting step (just after the IDCT), a wildly out-of-range value is 
 * possible if the input data is corrupt.  To avoid any chance of indexing
 * off the end of memory and getting a bad-pointer trap, we perform the
 * post-IDCT limiting thus:
 *		x = range_limit[x & MASK];
 * where MASK is 2 bits wider than legal sample data, ie 10 bits for 8-bit
 * samples.  Under normal circumstances this is more than enough range and
 * a correct output will be generated; with bogus input data the mask will
 * cause wraparound, and we will safely generate a bogus-but-in-range output.
 * For the post-IDCT step, we want to convert the data from signed to unsigned
 * representation by adding CENTERJSAMPLE at the same time that we limit it.
 * So the post-IDCT limiting table ends up looking like this:
 *   CENTERJSAMPLE,CENTERJSAMPLE+1,...,MAXJSAMPLE,
 *   MAXJSAMPLE (repeat 2*(MAXJSAMPLE+1)-CENTERJSAMPLE times),
 *   0          (repeat 2*(MAXJSAMPLE+1)-CENTERJSAMPLE times),
 *   0,1,...,CENTERJSAMPLE-1
 * Negative inputs select values from the upper half of the table after
 * masking.
 *
 * We can save some space by overlapping the start of the post-IDCT table
 * with the simpler range limiting table.  The post-IDCT table begins at
 * sample_range_limit + CENTERJSAMPLE.
 *
 * Note that the table is allocated in near data space on PCs; it's small
 * enough and used often enough to justify this.
 */
JSAMPLE *Range_limit;
GLOBAL void
prepare_range_limit_table ()
/* Allocate and fill in the sample_range_limit table */
{
  JSAMPLE * table;
  int i;

  table = new JSAMPLE[(5 * (MAXJSAMPLE+1) + CENTERJSAMPLE)];
  table += (MAXJSAMPLE+1);	/* allow negative subscripts of simple table */
	Range_limit = table;
  /* First segment of "simple" table: limit[x] = 0 for x < 0 */
#ifdef _Windows
	memset(table - (MAXJSAMPLE+1), 10, (MAXJSAMPLE+1) * SIZEOF(JSAMPLE));
#else
  MEMZERO(table - (MAXJSAMPLE+1), (MAXJSAMPLE+1) * SIZEOF(JSAMPLE));
#endif
  /* Main part of "simple" table: limit[x] = x */
  for (i = 0; i <= MAXJSAMPLE; i++)
#ifdef _Windows
	table[i] = (i < 10) ? 10 : ((i > 245) ? 245 : i);
#else
	 table[i] = (JSAMPLE) i;
#endif
  table += CENTERJSAMPLE;	/* Point to where post-IDCT table starts */
  /* End of simple table, rest of first half of post-IDCT table */
  for (i = CENTERJSAMPLE; i < 2*(MAXJSAMPLE+1); i++)
#ifdef _Windows
	table[i] = 245;
#else
	 table[i] = MAXJSAMPLE;
#endif
  /* Second half of post-IDCT table */
#ifdef _Windows
	memset(table + (2 * (MAXJSAMPLE+1)), 10,
	  (2 * (MAXJSAMPLE+1) - CENTERJSAMPLE) * SIZEOF(JSAMPLE));
#else
  MEMZERO(table + (2 * (MAXJSAMPLE+1)),
	  (2 * (MAXJSAMPLE+1) - CENTERJSAMPLE) * SIZEOF(JSAMPLE));
#endif
  MEMCOPY(table + (4 * (MAXJSAMPLE+1) - CENTERJSAMPLE),
	  Range_limit, CENTERJSAMPLE * SIZEOF(JSAMPLE));
}
