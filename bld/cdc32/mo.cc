
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//
// warning: most of the stuff in this file probably
// doesn't work very well, and probably isn't used anyway.

/*
 * $Header: g:/dwight/repo/cdc32/rcs/mo.cc 1.7 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "mo.h"
#include "pbmcfg.h"
#include "statfun.h"
#include "sqrs.h"
#define BLKSIZE 8
#define AREA 4
#define MAXSUM (10000000)

#define qqabs(ti) \
		(t = ti, \
		t2 = t, \
		t >>= 31, \
		t2 += t, \
        t ^ t2)

static int fast_subvec8(int *, int *);
#if 0
static inline int
subvec16(int *v1, int *v2)
{
    int t, t2;
#if 1
    return
        qqabs(v1[0] - v2[0]) +
        qqabs(v1[1] - v2[1]) +
        qqabs(v1[2] - v2[2]) +
        qqabs(v1[3] - v2[3]) +
        qqabs(v1[4] - v2[4]) +
        qqabs(v1[5] - v2[5]) +
        qqabs(v1[6] - v2[6]) +
        qqabs(v1[7] - v2[7]) +
        qqabs(v1[8] - v2[8]) +
        qqabs(v1[9] - v2[9]) +
        qqabs(v1[10] - v2[10]) +
        qqabs(v1[11] - v2[11]) +
        qqabs(v1[12] - v2[12]) +
        qqabs(v1[13] - v2[13]) +
        qqabs(v1[14] - v2[14]) +
        qqabs(v1[15] - v2[15]) ;
#else
    return
        qqabs(v1[0] - v2[0]) +
        qqabs(v1[2] - v2[2]) +
        qqabs(v1[4] - v2[4]) +
        qqabs(v1[6] - v2[6]) ;
#endif
}
static inline int
subvec(int *v1, int *v2)
{
    int t, t2;
#if 0
    return
        qqabs(v1[0] - v2[0]) +
        qqabs(v1[1] - v2[1]) +
        qqabs(v1[2] - v2[2]) +
        qqabs(v1[3] - v2[3]) +
        qqabs(v1[4] - v2[4]) +
        qqabs(v1[5] - v2[5]) +
        qqabs(v1[6] - v2[6]) +
        qqabs(v1[7] - v2[7]) ;
#else
    return
        qqabs(v1[0] - v2[0]) +
        qqabs(v1[2] - v2[2]) +
        qqabs(v1[4] - v2[4]) +
        qqabs(v1[6] - v2[6]) ;
#endif
}

static inline int
subvec_col(int *v1, int *v2)
{
    int t, t2;
    return
        qqabs(v1[0] - v2[0]) +
        qqabs(v1[1] - v2[8]) +
        qqabs(v1[2] - v2[2 * 8]) +
        qqabs(v1[3] - v2[3 * 8]) +
        qqabs(v1[4] - v2[4 * 8]) +
        qqabs(v1[5] - v2[5 * 8]) +
        qqabs(v1[6] - v2[6 * 8]) +
        qqabs(v1[7] - v2[7 * 8]);
}
#endif


int
subblock(int *v1, int *v2, int curmin)
{
    int sum = 0;
    for(int i = 0; i < 8; i += 2)
    {
        //sum += subvec(&v1[i * 8], &v2[16 * i]);
        sum += fast_subvec8(&v1[i * 8], &v2[16 * i]);
        if(sum > curmin)
        {
            return MAXSUM;
        }
    }
    return sum;
}

#if 0
int
subblock8_24(int *v1, int *v2, int curmin)
{
    int sum = 0;
    for(int i = 0; i < 8; i += 1)
    {
        //sum += subvec(&v1[i * 8], &v2[16 * i]);
        sum += fast_subvec8(&v1[i * 8], &v2[24 * i]);
        if(sum > curmin)
        {
            return MAXSUM;
        }
    }
    return sum;
}

int
subblock16(int *v1, int *v2, int curmin)
{
    int sum = 0;
    for(int i = 0; i < 8; i += 1)
    {
        sum += subvec(&v1[i * 16], &v2[24 * i]);
        if(sum > curmin)
        {
            return MAXSUM;
        }
    }
    return sum;
}

int
subblock16_32(int *v1, int *v2, int curmin)
{
    int sum = 0;
    for(int i = 0; i < 16; i += 1)
    {
        sum += subvec16(&v1[i * 16], &v2[32 * i]);
        if(sum > curmin)
        {
            return MAXSUM;
        }
    }
    return sum;
}
#endif

double
bm_motion_estimate(gray **last_frame, int *tmp, int fr, int fc,
                   int& vi, int& vj)
{
    int i, j;
    int *ptmp;

    int tmp2[16][16];
    ptmp = &tmp2[0][0];

    for(i = fr - 4; i < fr + 8 + 4; ++i)
        for(j = fc - 4; j < fc + 8 + 4; ++j)
            *ptmp++ = last_frame[i][j] - 128;

    int min = MAXSUM;
    for(i = 2; i < 8 + 2; ++i)
        for(j = 2; j < 8 + 2; ++j)
        {
            int t = subblock(tmp, &tmp2[i & 7][j & 7], min);
            if(t < min)
            {
                min = t;
                vi = i & 7;
                vj = j & 7;
            }
        }

    return min;
}

double
bm_motion_estimate(gray **last_frame, gray **frame, int fr, int fc,
                   int& vi, int& vj)
{
    int i, j;
    int tmp[8][8];
    int *ptmp = &tmp[0][0];
    int sum = 0;
    int sum2 = 0;

    for(i = fr; i < fr + 8; ++i)
        for(j = fc; j < fc + 8; ++j)
        {
            int t;
            t = *ptmp++ = frame[i][j];
            sum += t;
            sum2 += t * t;
        }
    double v = variance(64, sum, sum2);
    if(v < 64)
        return -1;

    int tmp2[16][16];
    ptmp = &tmp2[0][0];


    for(i = fr - 4; i < fr + 8 + 4; ++i)
        for(j = fc - 4; j < fc + 8 + 4; ++j)
            *ptmp++ = last_frame[i][j];

    int min = MAXSUM;
    for(i = 0; i < 8; ++i)
        for(j = 0; j < 8; ++j)
        {
            int t = subblock(&tmp[0][0], &tmp2[i][j], min);
            if(t < min)
            {
                min = t;
                vi = i;
                vj = j;
            }
        }

    return min;
}

#if 0
double
bm_motion_estimate16(gray **last_frame, gray **frame, int fr, int fc,
                     int& vi, int& vj)
{
    int i, j;
    int tmp[16][16];
    int *ptmp = &tmp[0][0];
    int sum = 0;
    int sum2 = 0;

    for(i = fr; i < fr + 16; ++i)
        for(j = fc; j < fc + 16; ++j)
        {
            int t;
            t = *ptmp++ = frame[i][j];
            sum += t;
            sum2 += t * t;
        }

    double v = variance(16 * 16, sum, sum2);
    if(v < 64)
        return -1;

    int tmp2[24][24];
    ptmp = &tmp2[0][0];

    for(i = fr - 4; i < fr + 16 + 4; ++i)
        for(j = fc - 4; j < fc + 16 + 4; ++j)
            *ptmp++ = last_frame[i][j];

    int min = MAXSUM;
    for(i = 0; i < 8; ++i)
        for(j = 0; j < 8; ++j)
        {
            int t = subblock16(&tmp[0][0], &tmp2[i][j], min);
            if(t < min)
            {
                min = t;
                vi = i;
                vj = j;
            }
        }

    return min;
}

double
motion_estimate(gray **last_frame, gray **frame, int fr, int fc,
                int& vi, int& vj)
{
    int i;
    int j;
    int p[8];
    int tmp[16][8];
    int *t = &tmp[0][0];
    int mins[8];
    int diffs[8];
    int d;
    int m;

    for(i = fr - 4; i < fr + 8 + 4; ++i)
        for(j = fc; j < fc + 8; ++j)
        {
            *t++ = last_frame[i][j];
        }

    for(i = 0; i < 8; ++i)
        p[i] = frame[fr + 3][fc + i];

    int min = MAXSUM;
    m = 0;
    d = 0;
    for(i = 4; i < 12; ++i)
    {
        int df;
        df = diffs[d++] = subvec(p, tmp[i]) ;
        if(df < min)
        {
            min = df;
            vi = i;
        }
    }
    for(i = 0; i < 8; ++i)
        if(diffs[i] == min)
            mins[m++] = i + 4;

    for(i = 0; i < 8; ++i)
        p[i] = frame[fr + i][fc + 3];

    for(j = 0; j < m; ++j)
    {
        min = MAXSUM;
        for(i = 0; i < 8; ++i)
        {
            int t = subvec_col(p, &tmp[mins[j] - 3][i]);
            if(t < min)
            {
                min = t;
                vj = i;
                vi = mins[j];
            }
        }
    }
    return min;
}

static void
lookplaces(int a[5], int b[5], int n)
{
    a[0] = b[0] = 0;
    a[1] = b[2] = n;
    a[3] = b[4] = -n;
    a[2] = a[4] = b[3] = b[1] = 0;
}

double
lbm_motion_estimate(gray **last_frame, gray **frame, int fr, int fc, int& vi, int &vj)
{
    int lftmp[32][32];
    int *lfp = &lftmp[0][0];
    int i, j;

    for(i = fr - 8; i < fr + 16 + 8; ++i)
        for(j = fc - 8; j < fc + 16 + 8; ++j)
            *lfp++ = last_frame[i][j];

    int ftmp[16][16];
    lfp = &ftmp[0][0];

    for(i = fr; i < fr + 16; ++i)
        for(j = fc; j < fc + 16; ++j)
            *lfp++ = frame[i][j];

    int q = 8;
    int l = 8;
    int n = 4;
    int mi, mj;
    int a[5], b[5], use[5];
    use[0] = use[1] = use[2] = use[3] = use[4] = 1;

    int min = MAXSUM;
    lookplaces(a, b, n);
    while(1)
    {
        int t;
        for(i = 0; i < 4; ++i)
        {
            if(!use[i])
                continue;
            t = subblock16_32(&ftmp[0][0], &lftmp[q + a[i]][l + b[i]], min);
            if(t <= min)
            {
                mi = a[i];
                mj = b[i];
                min = t;
            }
        }
        if(mi == 0 && mj == 0)
        {
            n /= 2;
            if(n == 1)
                break;
            lookplaces(a, b, n);
            use[0] = use[1] = use[2] = use[3] = use[4] = 1;
            continue;
        }
        if(q + mi < 0 || q + mi + 16 >= 32 ||
                l + mj < 0 || l + mj + 16 >= 32)
            break;
        q += mi;
        l += mj;
        for(i = 0; i < 4; ++i)
        {
            if(a[i] == -mi && b[i] == -mj)
                use[i] = 0;
            if(q + a[i] < 0 || q + a[i] + 16 >= 32)
                use[i] = 0;
            if(l + b[i] < 0 || l + b[i] + 16 >= 32)
                use[i] = 0;
        }
        if(!(use[0]|use[1]|use[2]|use[3]|use[4]))
        {
            printf("stumped\n");
            break;
        }
    }
    lookplaces(a, b, 1);
    for(i = 0; i < 4; ++i)
    {
        if(q + a[i] < 0 || q + a[i] + 16 >= 32
                || l + b[i] < 0 || l + b[i] + 16 >= 32)
            continue;

        int t = subblock16_32(&ftmp[0][0], &lftmp[q + a[i]][l + b[i]], min);
        if(t <= min)
        {
            mi = a[i];
            mj = b[i];
            min = t;
        }
    }
    vi = q + mi;
    vj = l + mj;
    return min;
}

#define SUBBLOCK subblock
double
lbm_motion_estimate8x8(gray **last_frame, int *frame, int fr, int fc, int& vi, int &vj)
{
    int lftmp[BLKSIZE + 2 * AREA][BLKSIZE + 2 * AREA];
    int *lfp = &lftmp[0][0];
    int i, j;

    for(i = fr - AREA; i < fr + BLKSIZE + AREA; ++i)
        for(j = fc - AREA; j < fc + BLKSIZE + AREA; ++j)
            *lfp++ = last_frame[i][j] - 128;

#if 0
    int ftmp[8][8];
    lfp = &ftmp[0][0];

    for(i = fr; i < fr + 16; ++i)
        for(j = fc; j < fc + 16; ++j)
            *lfp++ = frame[i][j];
#endif

    int q = MV_ZERO;
    int l = MV_ZERO;
    int n = 2;
    int mi, mj;
    int a[5], b[5], use[5];
    use[0] = use[1] = use[2] = use[3] = use[4] = 1;

    int min = MAXSUM;
    lookplaces(a, b, n);
    while(1)
    {
        int t;
        for(i = 0; i < 4; ++i)
        {
            if(!use[i])
                continue;
            t = SUBBLOCK(frame, &lftmp[q + a[i]][l + b[i]], min);
            if(t <= min)
            {
                mi = a[i];
                mj = b[i];
                min = t;
            }
        }
        if(mi == 0 && mj == 0)
        {
            n /= 2;
            if(n == 1)
                break;
            lookplaces(a, b, n);
            use[0] = use[1] = use[2] = use[3] = use[4] = 1;
            continue;
        }
        if(q + mi < 0 || q + mi + BLKSIZE >= BLKSIZE + 2 * AREA ||
                l + mj < 0 || l + mj + BLKSIZE >= BLKSIZE + 2 * AREA)
            break;
        q += mi;
        l += mj;
        for(i = 0; i < 4; ++i)
        {
            if(a[i] == -mi && b[i] == -mj)
                use[i] = 0;
            if(q + a[i] < 0 || q + a[i] + BLKSIZE >= BLKSIZE + 2 * AREA)
                use[i] = 0;
            if(l + b[i] < 0 || l + b[i] + BLKSIZE >= BLKSIZE + 2 * AREA)
                use[i] = 0;
        }
        if(!(use[0]|use[1]|use[2]|use[3]|use[4]))
        {
            printf("stumped\n");
            break;
        }
    }
    lookplaces(a, b, 1);
    for(i = 0; i < 4; ++i)
    {
        if(q + a[i] < 0 || q + a[i] + BLKSIZE >= BLKSIZE + 2 * AREA
                || l + b[i] < 0 || l + b[i] + BLKSIZE >= BLKSIZE + 2 * AREA)
            continue;

        int t = SUBBLOCK(frame, &lftmp[q + a[i]][l + b[i]], min);
        if(t <= min)
        {
            mi = a[i];
            mj = b[i];
            min = t;
        }
    }
    vi = q + mi;
    vj = l + mj;
    return min;
}
#endif

#pragma option -Od

static int
fast_subvec8(int *v1, int *v2)
{
#ifdef __BORLANDC__
#define two_pix(off) \
	mov eax, dword ptr off[ebp] ; \
	mov ebx, dword ptr off+4[ebp] ; \
 \
	sub eax, dword ptr off[esi] ; \
	sub ebx, dword ptr off+4[esi] ; \
 \
	add edx, dword ptr [edi + 4*eax] ; \
	add ecx, dword ptr [edi + 4*ebx] ;

    asm {
        xor edx, edx ;
        xor ecx, ecx ;

        mov edi, large dword ptr mo_abs_ptr ;
        mov esi, dword ptr v2 ;
        mov ebp, dword ptr v1 ;

        two_pix(0)
        two_pix(8)
        two_pix(16)
        two_pix(24)

        lea eax, dword ptr [edx + ecx] ;
    }
#else
    return
        mo_abs_ptr[v1[0] - v2[0]] +
        mo_abs_ptr[v1[1] - v2[1]] +
        mo_abs_ptr[v1[2] - v2[2]] +
        mo_abs_ptr[v1[3] - v2[3]] +
        mo_abs_ptr[v1[4] - v2[4]] +
        mo_abs_ptr[v1[5] - v2[5]] +
        mo_abs_ptr[v1[6] - v2[6]] +
        mo_abs_ptr[v1[7] - v2[7]] ;
#endif

}
#if 0
int
fast_subvec8_packed(int *v1, int *v2)
{
    asm {
        .486p ;
        push ebp ;
        xor ecx, ecx ;
        xor edx, edx ;
        xor edi, edi ;
        mov esi, v1 ;
        mov ebp, v2 ;

        mov eax, [esi] ;
        mov ebx, [ebp] ;

        or eax, 0x01010100 ;
        and ebx, 0xfefefeff ;

        shr eax, 1 ;
        shr ebx, 1 ;

        sub eax, ebx ;

        xor ebx, ebx ;

        mov cl, al ;
        mov bl, ah ;

        add edx, mo_abs[ecx] ;
        add edi, mo_abs[ebx] ;
        bswap eax ;
        mov cl, al ;
        mov bl, ah ;

        add edx, mo_abs[ecx] ;
        mov eax, 4[esi] ;
        add edi, mo_abs[ebx] ;
        mov ebx, 4[ebp] ;

        or eax, 0x01010100 ;
        and ebx, 0xfefefeff ;

        shr eax, 1 ;
        shr ebx, 1 ;

        sub eax, ebx ;

        xor ebx, ebx ;

        mov cl, al ;
        mov bl, ah ;

        add edx, mo_abs[ecx] ;
        add edi, mo_abs[ebx] ;
        bswap eax ;
        mov cl, al ;
        mov bl, ah ;

        add edx, mo_abs[ecx] ;
        add edi, mo_abs[ebx] ;

        lea eax, [edx + edi] ;
        pop ebp ;
    };

}
#endif
#pragma option -O.
#undef TEST_MO
#ifdef TEST_MO
main(int argc, char **argv)
{
    FILE *f = pm_openr("f1.pgm");
    FILE *f2 = pm_openr("f2.pgm");

    int cols, rows;
    gray maxval;
    gray **lf = pgm_readpgm(f, &cols, &rows, &maxval);
    gray **tf = pgm_readpgm(f2, &cols, &rows, &maxval);

    int i, j;

    for(i = 8; i < rows - 24; i += 16)
    {
        for(j = 8; j < cols - 24; j += 16)
        {
            int vi, vj;
            //double min = motion_estimate(lf, tf, i, j, vi, vj);
            //double min = bm_motion_estimate(lf, tf, i, j, vi, vj);
            double min = lbm_motion_estimate(lf, tf, i, j, vi, vj);
            printf("%d %d %f %d %d\n", i, j, min, vi, vj);
        }
    }

}
#endif
