
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/jqtab.h 1.8 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef JQTAB_H
#define JQTAB_H
#include "qtab.h"
#include "jinclude.h"
#include "jpeglib.h"

class JQTAB : public QTAB
{
public:
    static void jqtab_init();
    static unsigned int std_luminance_quant_tbl[DCTSIZE2];
    static unsigned int std_chrominance_quant_tbl[DCTSIZE2];

    JQTAB(int quality = 75, const char *name = 0, const unsigned int *table = 0);
    void jpeg_init_quant_table (
        const unsigned int *basic_table,
        int scale_factor, int force_baseline);
    void init_dct_tables();


};

// note: qid must be stored in 5 bits for xmission over
// channel, and there are 10 id's that are needed now
// which makes this work (10+16 < 31). if you change the
// offset or number of tables, make sure everything
// still fits in 5 bits.
// NOTE NOTE NOTE
// this is very rickety, really need to have fixed
// id's because the dynamic allocation of id's
// is bad because the compatibility of codecs is based
// on the id's being the same between the sender
// and receiver.
// XXX FIX THIS XXX, the JPEG offset is gotten
// empirically and will change depending on
// how many ctors get called in QTAB, etc.
// very bad. this means that global vars
// can change the id allocation, which is *very*
// bad.

// offset for base of all JPEG tables
#define Q_JPEG_OFF 3
// additional offset for chroma jpeg tables
#define Q_JPEG_CHROMA_OFF 5

// the old codec uses 3-7 for luma quant tables,
// and 8-12 for chroma tables... you add Q_JPEG_CHROMA_OFF
// to the luma id to get the chroma id.
//
// the new codec uses a lot more tables, mainly to support
// foveation, so it's offset is Q_JPEG3_OFF .. QJPEG3_OFF + NUM_JPEG3_QTABS - 1
// are the luma quantization tables, and the chroma
// are Q_JPEG3_CHROMA_OFF..QJPEG3_CHROMA_OFF + NUM_JPEG3_QTABS - 1
#undef JPEG3_CODEC
#define NUM_JPEG3_QTABS 65
#define Q_JPEG3_OFF 16
#define Q_JPEG3_CHROMA_OFF (Q_JPEG3_OFF + NUM_JPEG3_QTABS)



#endif
