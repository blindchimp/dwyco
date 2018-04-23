
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/imgmisc.h 1.7 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef IMGMISC_H
#define IMGMISC_H
#include "matcom.h"
#include "pbmcfg.h"
gray **subsample(gray **img, int *rows, int *cols, int factor);
//gray **subsample2(gray **img, int *rows, int *cols);
template <class T>
T ** subsample2(T **img, int *rows, int *cols);
template<class T>
T **crop(T **img, int *cols, int *rows, int x0, int y0, int w, int h, int pad, int to_be_subsampled);

template<class T>
T **
cond_pad(T **img, int cols, int rows, int ncols, int nrows);

gray **subsample2_1(gray **img, int *rows, int *cols);
gray **upsample0_2(gray **img, int *cols, int *rows);
gray **upsample1_2(gray **img, int *cols, int *rows);
gray **upsample2_2(gray **img, int *cols, int *rows);
double stdev_vec(ELTYPE *in, int n, double& mean_out);
double mse_vec(ELTYPE *in, int n, double& mean_out);
double mae_vec(ELTYPE *in, int n, double& mean_out);
void printvec(ELTYPE *in, int n);
//gray **crop(gray **img, int *cols, int *rows, int x, int y, int w, int h, int pad, int to_be_subsampled);
void flip_in_place(gray **img, int cols, int rows);
void compute_subsampled_region(int& cols, int& rows, int pad, int sample_factor);
pixel **copy_ppm_frame(pixel **p, int cols, int rows);

template<class T>
void
flip_in_place(T **img, int cols, int rows)
{
    T *tmp = (T *)pm_allocrow(cols, sizeof(T));
    int lim = rows / 2;

    for(int i = 0; i < lim; ++i)
    {
        bcopy(&img[i][0], &tmp[0], cols * sizeof(T));
        bcopy(&img[rows - i - 1][0], &img[i][0], cols * sizeof(T));
        bcopy(&tmp[0], &img[rows - i - 1][0], cols * sizeof(T));
    }
    pm_freerow((char *)tmp);
}

#endif
