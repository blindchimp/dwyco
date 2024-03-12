
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/qtab.cc 1.9 1997/11/25 20:41:03 dwight Stable095 $
 */
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "qtab.h"
#include "matcom.h"
#include "dwmapr.h"
#include "tcode.h"
#include "dwlog.h"
#include "jdct.h"
#include "fnmod.h"
#include "doinit.h"

using namespace dwyco;

QTABTAB QTables;
QIDTAB QidTables;
#ifdef DWYCO_DCT_CODER
#define fastfun(name) {#name, TCoder::max_##name, TCoder::recon_##name}
static struct nfp {
    char *name;
    QTAB_MAXFUNP maxfun;
    QTAB_RECONFUNP reconfun;
#ifdef DWYCO_CODEC
} funs[] = {

    fastfun(dct75_33),
    fastfun(dct75_64),
    fastfun(dct85_16),
    fastfun(dct85_32),
    fastfun(dct95_16),
    fastfun(dct95_32),
    fastfun(dct95_64),
    fastfun(dct95_128),
    fastfun(dct95_256),
    fastfun(no_bits),
    fastfun(space2bits),
    fastfun(space1bits),
    fastfun(space3bits),
    fastfun(mean_only),
    fastfun(dct99_32),
    fastfun(dct99_64),
    fastfun(dct99_128),
    fastfun(dct99_256)

};
#define NFUNS (sizeof(funs) / sizeof(funs[0]))
#else
} funs[1];
#define NFUNS 0
#endif
#endif

int QTAB::Id;

QTAB QTAB::JustDctScales;

void
QTAB::qtab_init()
{
// note: this is a hack, we know we have been distributing
// this stuff with a qtabs file with one table, but since we
// want to get rid of the file and just keep the offsets all the
// same, we make one qtab here with the right name and nothing else
// in it. sigh. what a piece of junk... i'll rip this out eventually.
// this is only used by the old CODER (not decoder). theora doesn't use this.
    QTAB *qp = new QTAB;
    qp->name = "dct99_512";
    QTables.add(qp->name, qp);
    QidTables.add(qp->id, qp);
    return;
#if 0
    FILE *f = fopen(newfn("qtabs").c_str(), "rt");
    if(f == 0)
    {
        Log_make_entry("no quantization tables?");
        return;
    }

    int i = 0;
    for(;;)
    {
        QTAB *qp = new QTAB;
        if(!qp->read(f))
            break;
        QTables.add(qp->name, qp);
        QidTables.add(qp->id, qp);
        for(int j = 0; j < NFUNS; ++j)
            if(qp->name.eq(funs[j].name) == 0)
            {
                qp->maxfun = funs[j].maxfun;
                qp->reconfun = funs[j].reconfun;
            }
        ++i;
    }
    char s[100];
    sprintf(s, "%d dwyco quantization tables", i);
    Log_make_entry(s);
    fclose(f);
#endif
}

QTAB::QTAB()
{
    name = "unmapped";
    memset(q, 0, sizeof(q));
    id = Id++;
#ifdef DWYCO_DCT_CODER
    maxfun = 0;
    reconfun = 0;
#endif
    init_dct_tables();
}

#if 0
QTAB::QTAB(FILE *f)
{
    read(f);
    id = Id++;
    maxfun = 0;
    reconfun = 0;
    init_dct_tables();
}

int
QTAB::read(FILE *f)
{
    // first line is id
    char s[255];
    if(fscanf(f, "%s", s) != 1)
        return 0;
    for(int i = 0; i < MSZ; ++i)
        for(int j = 0; j < MSZ; ++j)
        {
            int inp;
            if(fscanf(f, "%d", &inp) != 1)
            {
                return 0;
            }
            q[i][j] = inp;
        }
    name = s;
    return 1;
}

void
QTAB::write(FILE *f)
{
    fprintf(f, "%s\n", name.c_str());
    for(int i = 0; i < MSZ; ++i)
    {
        for(int j = 0; j < MSZ; ++j)
        {
            fprintf(f, "%d ", q[i][j]);
        }
        fputc('\n', f);
    }

}
#endif

INT16 QTAB::aanscales[DCTSIZE2] = {
    /* precomputed values scaled up by 14 bits */
    16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
    22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
    21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
    19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
    16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
    12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
    8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
    4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
};

void
QTAB::init_dct_tables()
{
    /* For AA&N IDCT method, multipliers are equal to quantization
     * coefficients scaled by scalefactor[row]*scalefactor[col], where
     *   scalefactor[0] = 1
     *   scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
     * For integer operation, the multiplier table is to be scaled by
     * IFAST_SCALE_BITS.  The multipliers are stored in natural order.
     */
    IFAST_MULT_TYPE * ifmtbl;
#define aan_CONST_BITS 14

    SHIFT_TEMPS

//DESCALE(MULTIPLY16V16((INT32) qtbl->quantval[jpeg_zigzag_order[i]],

    for (int i = 0; i < DCTSIZE2; i++) {
        f_dct[i] = (IFAST_MULT_TYPE)
                   DESCALE((INT32) aanscales[i],
                           aan_CONST_BITS-3);
        if(f_dct[i] == 0) f_dct[i] = 1;
        i_dct[i] = (IFAST_MULT_TYPE)
                   DESCALE((INT32) aanscales[i],
                           aan_CONST_BITS-IFAST_SCALE_BITS);
    }
}

#ifdef TEST
main()
{
    fwt_init();
    QTAB q;
    QTAB qr;
    QTAB *qp;
    FILE *f = fopen("qtabs", "r");
    for(int i = 0; i < 25; ++i)
    {
        qp = new QTAB(f);
        QTables.add(qp->name, qp);
        qp->write(stdout);
        qp->reorder(qr);
        qr.write(stdout);
        printf("\n");
    }
    if(QTables.find("wal50_128", qp))
        qp->write(stdout);
}
#endif
