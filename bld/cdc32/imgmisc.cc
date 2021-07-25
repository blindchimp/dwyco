
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/imgmisc.cc 1.10 1997/11/25 20:41:03 dwight Stable095 $
 */
#include <math.h>
#include "macs.h"
#include "sqrs.h"
#include "pbmcfg.h"
#include "matcom.h"
#include "imgmisc.h"
#include "statfun.h"

[[noreturn]] void oopanic(const char *);

static gray **
allocarray(int c, int r, gray **dummy)
{
    return pgm_allocarray(c, r);
}

static pixel **
allocarray(int c, int r, pixel **dummy)
{
    return ppm_allocarray(c, r);
}


void
compute_subsampled_region(int& cols, int& rows, int pad, int sample_factor)
{
    int cw;
    int ch;
    int w = cols / sample_factor;
    int h = rows / sample_factor;
    int msz = MSZ;
    int strunc = STRUNC;

    if(pad)
    {
        cw = w + ((w & strunc) ? (msz - (w & strunc)) : 0);
        ch = h + ((h & strunc) ? (msz - (h & strunc)) : 0);
    }
    else
    {
        cw = w & ~strunc;
        ch = h & ~strunc;
    }
    cols = cw;
    rows = ch;
}

// function to produce a codeable frame.
// user wants an active area x0, y0, w, h.
// if pad == 1, then the area is extracted into
// a frame that is padded to the next higher
// multiple of the coding block size.
// if pad == 0, the area is extracted and it is
// truncated to the next lower multiple of the block size.
// *cols and *rows should be set to the size of the input frame.
// on return, cols and rows contain the actual size of the
// returned frame, which may be different than
// the user's request (ie. the frame is ready for
// coding.) if crop returns 0, then no adjustments
// are needed to the given frame.

static void
pad_right(gray **g, gray **img, int x0, int y0, int w, int h, int cw)
{
    for(int i = 0; i < h; ++i)
        memset(&g[i][w], img[i + y0][x0 + w - 1], cw - w);
}

static void
pad_right(pixel **g, pixel **img, int x0, int y0, int w, int h, int cw)
{
    for(int i = 0; i < h; ++i)
        memset(&g[i][w], 0, (cw - w) * sizeof(pixel));
}

template<class T>
T **
crop(T **img, int *cols, int *rows, int x0, int y0, int w, int h, int pad, int to_be_subsampled)
{
    int i;
    int cw;
    int ch;
    int msz = MSZ;
    int strunc = STRUNC;
    if(to_be_subsampled)
    {
        msz = 2 * MSZ;
        strunc = (1 << log2(msz)) - 1;
    }
    if(w < 0)
        w = 0;
    if(h < 0)
        h = 0;
    if(x0 >= *cols)
        x0 = *cols - 1;
    if(y0 >= *rows)
        y0 = *rows - 1;
    if(y0 < 0)
        y0 = 0;
    if(x0 < 0)
        x0 = 0;
    if(x0 + w > *cols)
        w = *cols - x0;
    if(y0 + h > *rows)
        h = *rows - y0;
    if(pad)
    {
        cw = w + ((w & strunc) ? (msz - (w & strunc)) : 0);
        ch = h + ((h & strunc) ? (msz - (h & strunc)) : 0);
    }
    else
    {
        cw = w & ~strunc;
        ch = h & ~strunc;
    }
    if(x0 == 0 && y0 == 0 && cw == w && ch == h)
        return 0; // no processing necessary
    T **dummy;
    T **g = allocarray(cw, ch, dummy);
    if(pad)
    {
        if(cw != w)
            pad_right(g, img, x0, y0, w, h, cw);
        //for(i = 0; i < h; ++i)
        //	memset(&g[i][w], img[i + y0][x0 + w - 1], (cw - w) * sizeof(T));
        for(i = h; i < ch; ++i)
            bcopy(&img[y0 + h - 1][0], &g[i][0], w * sizeof(T));
    }
    int copy_h = dw_min(h, ch);
    int copy_w = dw_min(w, cw);
    for(i = y0; i < copy_h + y0; ++i)
        bcopy(&img[i][x0], &g[i - y0][0], copy_w * sizeof(T));
    *cols = cw;
    *rows = ch;
    return g;
}

// conditionally pad a frame out to a given size.
// the frame is put in the upper-left of the resulting
// image. returns 0 if there is no need to pad the frame.
template<class T>
T **
cond_pad(T **img, int cols, int rows, int ncols, int nrows)
{
    if(cols == ncols && rows == nrows)
        return 0;
    if(cols > ncols || rows > nrows)
        oopanic("bogus cond_pad call");
    int cw = ncols;
    int ch = nrows;
    int w = cols;
    int h = rows;
    T **dummy;
    T **g = allocarray(cw, ch, dummy);
    if(cw != w)
        pad_right(g, img, 0, 0, w, h, cw);
    int i;
    for(i = h; i < ch; ++i)
        memcpy(&g[i][0], &img[h - 1][0],  w * sizeof(T));
    int copy_h = dw_min(h, ch);
    int copy_w = dw_min(w, cw);
    for(i = 0; i < copy_h; ++i)
        memcpy(&g[i][0], &img[i][0], copy_w * sizeof(T));
    return g;
}


gray **
subsample(gray **img, int *rows, int *cols, int factor)
{
    int nrows = *rows / factor;
    int ncols = *cols / factor;
    int i;
    int j;
    gray **nimg = pgm_allocarray(ncols, nrows);
    for(i = 0; i < nrows; ++i)
        for(j = 0; j < ncols; ++j)
            nimg[i][j] = img[i * factor][j * factor];
    *rows = nrows;
    *cols = ncols;
    return nimg;
}



template <class T>
T **
subsample2(T **img, int *rows, int *cols)
{
    int nrows = *rows / 2;
    int ncols = *cols / 2;
    int i;
    int j;
    T **dummy;
    T **nimg = allocarray(ncols, nrows, dummy);
    int c = ncols & ~0x3;
#define loop_body(k) \
		g[j + (k)] = g2[j * 2 + (k) * 2];

    for(i = 0; i < nrows; ++i)
    {
        T *g = nimg[i];
        T *g2 = img[i * 2];
        for(j = 0; j < c; j += 4)
        {
            int t;
            loop_body(0)
            loop_body(1)
            loop_body(2)
            loop_body(3)
        }
        for(j = c; j < ncols; ++j)
        {
            int t;
            loop_body(0)
        }
    }
    *rows = nrows;
    *cols = ncols;
    return nimg;
#undef loop_body
}

template gray  **subsample2(gray **, int*, int*);
template pixel **subsample2(pixel **, int *, int*);
template gray **crop(gray **img, int *cols, int *rows, int x0, int y0, int w, int h, int pad, int to_be_subsampled);
template pixel **crop(pixel **img, int *cols, int *rows, int x0, int y0, int w, int h, int pad, int to_be_subsampled);

template gray** cond_pad(gray **img, int cols, int rows, int ncols, int nrows);
template pixel ** cond_pad(pixel **img, int cols, int rows, int ncols, int nrows);

#if 0
gray **
subsample2(gray **img, int *rows, int *cols)
{
    int nrows = *rows / 2;
    int ncols = *cols / 2;
    int i;
    int j;
    gray **nimg = pgm_allocarray(ncols, nrows);
    int c = ncols & ~0x3;
#define loop_body(k) \
		g[j + (k)] = g2[j * 2 + (k) * 2];

    for(i = 0; i < nrows; ++i)
    {
        gray *g = nimg[i];
        gray *g2 = img[i * 2];
        for(j = 0; j < c; j += 4)
        {
            int t;
            loop_body(0)
            loop_body(1)
            loop_body(2)
            loop_body(3)
        }
        for(j = c; j < ncols; ++j)
        {
            int t;
            loop_body(0)
        }
    }
    *rows = nrows;
    *cols = ncols;
    return nimg;
#undef loop_body
}
#endif

// subsample by 2 with boxcar filtering
gray **
subsample2_1(gray **img, int *rows, int *cols)
{
    int nrows = *rows / 2;
    int ncols = *cols / 2;
    int i;
    int j;
    gray **nimg = pgm_allocarray(ncols, nrows);
    int c = ncols & ~0x3;
#define loop_body(k) \
		g[j + (k)] = (g2[j * 2 + (k) * 2] + g2[j * 2 + (k) * 2 + 1] + \
			g3[j * 2 + (k) * 2] + g3[j * 2 + (k) * 2 + 1]) / 4;

    for(i = 0; i < nrows; ++i)
    {
        gray *g = nimg[i];
        gray *g2 = img[i * 2];
        gray *g3 = img[i * 2 + 1];
        for(j = 0; j < c; j += 4)
        {
            int t;
            loop_body(0)
            loop_body(1)
            loop_body(2)
            loop_body(3)
        }
        for(j = c; j < ncols; ++j)
        {
            int t;
            loop_body(0)
        }
    }
    *rows = nrows;
    *cols = ncols;
    return nimg;
#undef loop_body
}

// 0-order upsample
gray **
upsample0_2(gray **img, int *cols, int *rows)
{
    int nrows = *rows * 2;
    int ncols = *cols * 2;
    int i;
    int j;
    int ii;
    int jj;
    gray **nimg = pgm_allocarray(ncols, nrows);
    int c = (*cols) & ~0x3;
#define loop_body(k) \
			t = img[i][j + (k)]; \
			g = nimg[ii]; \
			g[jj + (2 * (k))] = t; \
			g[jj + (2 * (k)) + 1] = t; \
			g = nimg[ii + 1]; \
			g[jj + (2 * (k))] = t; \
			g[jj + (2 * (k)) + 1] = t;

    for(i = 0, ii = 0; i < *rows; ++i, ii += 2)
    {
        for(j = 0, jj = 0; j < c; j += 4, jj += 8)
        {
            gray t;
            gray *g;
            loop_body(0)
            loop_body(1)
            loop_body(2)
            loop_body(3)
        }
        for(j = c, jj = 2 * c; j < *cols; ++j, jj += 2)
        {
            gray t;
            gray *g;
            loop_body(0)
        }
    }
    *rows = nrows;
    *cols = ncols;
    return nimg;
#undef loop_body
}

// 1-order upsample
gray **
upsample1_2(gray **img, int *cols, int *rows)
{
// until we fix this, just do the simple upsampling
// there is a buffer overrun below that can cause problems
    return upsample0_2(img, cols, rows);
    int nrows = *rows * 2;
    int ncols = *cols * 2;
    int i;
    int j;
    int ii;
    int jj;
    gray **nimg = pgm_allocarray(ncols, nrows);

#if 0
    for(i = 1; i < nrows; i += 2)
                memset(&nimg[i][0], 0, ncols);
#endif

    int c = (*cols) & ~0x3;
#define loop_body(k) \
			g2[jj + (2 * (k))] = g[j + (k)]; \
			g2[jj + (2 * (k)) + 1] = (g[j + (k)] + g[j + (k) + 1]) / 2;

    for(i = 0, ii = 0; i < *rows; ++i, ii += 2)
    {
        gray *g = img[i];
        gray *g2 = nimg[ii];
        for(j = 0, jj = 0; j < c; j += 4, jj += 8)
        {
            loop_body(0)
            loop_body(1)
            loop_body(2)
            loop_body(3)
        }
        for(j = c, jj = 2 * c; j < *cols; ++j, jj += 2)
        {
            loop_body(0)
        }
    }
#undef loop_body
    c = ncols & ~0x3;
#define loop_body(k) \
			g1[j + (k)] = (g0[j + (k)] + g2[j + (k)]) / 2;
    for(i = 1; i < nrows - 1; i += 2)
    {
        gray *g0 = nimg[i - 1];
        gray *g1 = nimg[i];
        gray *g2 = nimg[i + 1];
        for(j = 0; j < c; j += 4)
        {
            loop_body(0)
            loop_body(1)
            loop_body(2)
            loop_body(3)
        }
        for(j = c; j < ncols; ++j)
        {
            loop_body(0)
        }
    }
    // assume image is padded with zeros
    for(j = 0; j < ncols; ++j)
                nimg[nrows - 1][j] = nimg[nrows - 2][j] / 2;
#undef loop_body
#if 0
    for(i = 1; i < nrows - 1; i += 2)
    {
        gray *g0 = nimg[i - 1];
        gray *g1 = nimg[i];
        gray *g2 = nimg[i + 1];
        for(j = 0; j < ncols; ++j)
        {
            g1[j] = (g0[j] + g2[j]) / 2;
            //nimg[i][j ] = (nimg[i - 1][j ] + nimg[i + 1][j ]) / 2;
        }
    }
#endif
#if 0
    for(i = 1; i < nrows - 1; ++i)
    {
        if(i & 1)
        {
            for(j = 1; j < ncols - 1; j += 2)
                        nimg2[i][j] = (nimg[i - 1][j - 1] + nimg[i + 1][j - 1] + nimg[i + 1][j + 1] + nimg[i - 1][j + 1]) / 4;
            for(j = 2; j < ncols - 1; j += 2)
                        nimg2[i][j] = (nimg[i - 1][j] + nimg[i + 1][j]) / 2;
        }
        else
        {
            for(j = 1; j < ncols - 1; j += 2)
                        nimg2[i][j] = (nimg[i][j + 1] + nimg[i][j - 1]) / 2;
            for(j = 0; j < ncols; j += 2)
                        nimg2[i][j] = nimg[i][j];
        }
    }
    pgm_freearray(nimg, nrows);
#endif
    *rows = nrows;
    *cols = ncols;
    return nimg;
}

// 1-order upsample by 4, assuming samples are in the
// middle of a 4x4 grid
gray **
upsample2_2(gray **img, int *cols, int *rows)
{
    // need extra 4 to get pel offset right
    int nrows = *rows * 4 + 4;
    int ncols = *cols * 4 + 4;
    int i;
    int j;
    int ii;
    int jj;
    gray **nimg = pgm_allocarray(ncols, nrows);

    memset(&nimg[0][0], 0, ncols * nrows);

    int c = ((*cols) & ~0x3) - 4;

#define loop_body(k) \
			g2[jj + (4 * (k))] = (7 * g[j + (k)] + g[j + (k) + 1]) / 8; \
			g2[jj + (4 * (k)) + 1] = (5 * g[j + (k)] + 3 * g[j + (k) + 1]) / 8; \
			g2[jj + (4 * (k)) + 2] = (3 * g[j + (k)] + 5 * g[j + (k) + 1]) / 8; \
			g2[jj + (4 * (k)) + 3] = (g[j + (k)] + 7 * g[j + (k) + 1]) / 8;

    for(i = 0, ii = 2; i < *rows; ++i, ii += 4)
    {
        gray *g = img[i];
        gray *g2 = &nimg[ii][2];
        for(j = 0, jj = 0; j < c; j += 4, jj += 16)
        {
            loop_body(0)
            loop_body(1)
            loop_body(2)
            loop_body(3)
        }
        for(j = c, jj = 4 * c; j < *cols - 1; ++j, jj += 4)
        {
            loop_body(0)
        }
        // get first two pixels
        nimg[ii][0] = g[0];
        nimg[ii][1] = g[0];
    }

#undef loop_body
    c = ncols & ~0x3;
#define loop_body(k) \
			g1[j + (k)] = (5 * g0[j + (k)] + 3 * g4[j + (k)]) / 8; \
			g2[j + (k)] = (3 * g0[j + (k)] + 5 * g4[j + (k)]) / 8; \
			g3[j + (k)] = (g0[j + (k)] + 7 * g4[j + (k)]) / 8;
    for(i = 0; i < 2; ++i)
    {
        for(j = 0; j < ncols; ++j)
                    nimg[i][j] = nimg[2][j];
        //nimg[i][j] = ((5 + 2 * i) * nimg[2][j]) / 8;
    }
    for(i = 2; i < nrows - 2; i += 4)
    {
        gray *g0 = nimg[i];
        gray *g1 = nimg[i + 1];
        gray *g2 = nimg[i + 2];
        gray *g3 = nimg[i + 3];
        gray *g4 = nimg[i + 4];
        for(j = 0; j < c; j += 4)
        {
            loop_body(0)
            loop_body(1)
            loop_body(2)
            loop_body(3)
        }
        for(j = c; j < ncols; ++j)
        {
            loop_body(0)
        }
    }
    // assume image is padded with zeros
    for(j = 0; j < ncols; ++j)
                nimg[nrows - 1][j] = nimg[nrows - 2][j];
#undef loop_body
    *rows = nrows;
    *cols = ncols;
    return nimg;
}

#if 0
void
flip_in_place(gray **img, int cols, int rows)
{
    gray *tmp = pgm_allocrow(cols);
    int lim = rows / 2;

    for(int i = 0; i < lim; ++i)
    {
        bcopy(&img[i][0], &tmp[0], cols);
        bcopy(&img[rows - i - 1][0], &img[i][0], cols);
        bcopy(&tmp[0], &img[rows - i - 1][0], cols);
    }
    pgm_freerow(tmp);
}
#endif

double
stdev_vec(ELTYPE *in, int n, double& mean_out)
{
    long sum = 0;
    long sum2 = 0;

    for(int i = 0; i < n; ++i)
    {
        sum += in[i];
        sum2 += in[i] * in[i];
    }
    mean_out = (double)sum / n;
    return stdev(n, sum, sum2);
}

// mean squared error
double
mse_vec(ELTYPE *in, int n, double& mean_out)
{
    long sum = 0;
    long sum2 = 0;

    for(int i = 0; i < n; ++i)
    {
        //sum += in[i];
        //int t = qabs(in[i]);
        //sum2 += sqrs[t]; // something wrong here... mean probably overflows?
        long t = in[i];
        sum2 += t * t;
    }
    mean_out = (double)sum / n;
    return sqrt((double)sum2 / n);
}

// mean absolute error
double
mae_vec(ELTYPE *in, int n, double& mean_out)
{
    long sum = 0;
#define qqabs(p) \
		t = in[p]; \
		t2 = t; \
		t >>= 31; \
		t2 += t; \
		t ^= t2; \
		sum += t;
    int iter = n >> 3;
    int i;
    for(i = 0; i < iter; ++i, in += 8)
    {
        long t;
        long t2;
        qqabs(0);
        qqabs(1);
        qqabs(2);
        qqabs(3);
        qqabs(4);
        qqabs(5);
        qqabs(6);
        qqabs(7);
    }
    if(n & 0x7)
    {
        iter = n & 0x7;
        for(i = 0; i < iter; ++i, ++in)
        {
            long t;
            long t2;
            qqabs(0);
        }
    }
    //mean_out = (double)sum / n;
    return (double)sum / n;
}

void
printvec(ELTYPE *in, int n)
{
    //printf("---\n");
    for(int i = 0; i < n; ++i)
                printf("%d ", in[i]);
    printf("\n");
}

pixel **
copy_ppm_frame(pixel **img, int cols, int rows)
{
    pixel **p = ppm_allocarray(cols, rows);
    for(int i = 0; i < rows; ++i)
    {
        memcpy(p[i], img[i], cols * sizeof(pixel));
    }
    return p;
}

