
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jqtab.cc 1.9 1997/11/25 20:41:03 dwight Stable095 $
 */
#include "jqtab.h"
#include "dwlog.h"
#include "doinit.h"
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"

static const char *Names[10] = {
    "lowest",
    "typical",
    "better",
    "better-yet",
    "best",
    "clowest",
    "ctypical",
    "cbetter",
    "cbetter-yet",
    "cbest"
};

void
JQTAB::jqtab_init()
{
    int i;
    for(i = 1; i <= 5; ++i)
    {
        JQTAB *qp = new JQTAB(i * 15, Names[i - 1], std_luminance_quant_tbl);
        qp->id = Q_JPEG_OFF + i - 1;
#if 0
        char s[100];
        sprintf(s, "jqtab %d", qp->id);
        Log->make_entry(s);
#endif

        QTables.add(qp->name, qp);
        QidTables.add(qp->id, qp);
        qp = new JQTAB(i * 15, Names[i - 1 + Q_JPEG_CHROMA_OFF], std_chrominance_quant_tbl);
        qp->id = Q_JPEG_OFF + i - 1 + Q_JPEG_CHROMA_OFF;
        QTables.add(qp->name, qp);
        QidTables.add(qp->id, qp);
    }

#ifdef JPEG3_CODEC
    // foveation is handled by decreasing the quality from
    // the middle of the pic to the edge. we use
    // 65 steps, because that will give us a smooth
    // quality in a 640x480 picture without too many
    // abrupt jumps in the worst case.
    for(i = 5; i <= 70; ++i)
    {
        char nm[100];
        sprintf(nm, "luma%d", i);
        JQTAB *qp = new JQTAB(i, nm, std_luminance_quant_tbl);
        qp->id = Q_JPEG3_OFF + i - 1;
        QTables.add(qp->name, qp);
        QidTables.add(qp->id, qp);
        sprintf(nm, "chroma%d", i);
        qp = new JQTAB(i, nm, std_chrominance_quant_tbl);
        qp->id = Q_JPEG_OFF + i - 1 + Q_JPEG3_CHROMA_OFF;
        QTables.add(qp->name, qp);
        QidTables.add(qp->id, qp);
    }
#endif
}

JQTAB::JQTAB(int quality, const char *nm, const unsigned int *basic_table)
{
    char s[100];
    if(nm == 0)
        sprintf(s, "jpeg%02d", quality);
    else
        strcpy(s, nm);
    name = s;
    jpeg_init_quant_table(basic_table,
                          jpeg_quality_scaling(quality), 0);
    init_dct_tables();
}

// this must be called *after* initializing the quantization
// tables.
void
JQTAB::init_dct_tables()
{
#ifdef DCT_SLOW
    for (int i = 0; i < DCTSIZE2; i++) {
        f_dct[i] = (*this)[jpeg_zigzag_order[i]] << 3;
        i_dct[i] = (*this)[jpeg_zigzag_order[i]];
    }
#else
    /* For AA&N IDCT method, multipliers are equal to quantization
     * coefficients scaled by scalefactor[row]*scalefactor[col], where
     *   scalefactor[0] = 1
     *   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
     * For integer operation, the multiplier table is to be scaled by
     * IFAST_SCALE_BITS.  The multipliers are stored in natural order.
     */
    //IFAST_MULT_TYPE * ifmtbl;
#define aan_CONST_BITS 14

    SHIFT_TEMPS

//DESCALE(MULTIPLY16V16((INT32) qtbl->quantval[jpeg_zigzag_order[i]],

    for (int i = 0; i < DCTSIZE2; i++) {
        f_dct[i] = (IFAST_MULT_TYPE)
                   DESCALE((INT32) aanscales[i] * (*this)[jpeg_zigzag_order[i]],
                           aan_CONST_BITS-3);
        f_dct_mul[i] = (1. / f_dct[i]) * (1 << 17);
        if(f_dct[i] == 0) f_dct[i] = 1;

        i_dct[i] = (IFAST_MULT_TYPE)
                   DESCALE((INT32) aanscales[i] * (*this)[jpeg_zigzag_order[i]],
                           aan_CONST_BITS-IFAST_SCALE_BITS);
    }
#endif
}

unsigned int JQTAB::std_luminance_quant_tbl[DCTSIZE2] = {
    16,  11,  12,  14,  12,  10,  16,  14,
    13,  14,  18,  17,  16,  19,  24,  40,
    26,  24,  22,  22,  24,  49,  35,  37,
    29,  40,  58,  51,  61,  60,  57,  51,
    56,  55,  64,  72,  92,  78,  64,  68,
    87,  69,  55,  56,  80, 109,  81,  87,
    95,  98, 103, 104, 103,  62,  77, 113,
    121, 112, 100, 120,  92, 101, 103,  99
};
unsigned int JQTAB::std_chrominance_quant_tbl[DCTSIZE2] = {
    17,  18,  18,  24,  21,  24,  47,  26,
    26,  47,  99,  66,  56,  66,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};

void
JQTAB::jpeg_init_quant_table (
    const unsigned int *basic_table,
    int scale_factor, int force_baseline)
/* Define a quantization table equal to the basic_table times
 * a scale factor (given as a percentage).
 * If force_baseline is TRUE, the computed quantization table entries
 * are limited to 1..255 for JPEG baseline compatibility.
 */
{
    int i;
    long temp;

    for (i = 0; i < DCTSIZE2; i++) {
        temp = ((long) basic_table[i] * scale_factor + 50L) / 100L;
        /* limit the values to the valid range */
        if (temp <= 0L) temp = 1L;
        if (temp > 32767L) temp = 32767L; /* max quantizer needed for 12 bits */
        if (force_baseline && temp > 255L)
            temp = 255L;		/* limit to baseline range if requested */
        (*this)[i] = (dwINT32)temp;
    }

}


