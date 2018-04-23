
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef QTAB_H
#define QTAB_H
// quantization table handler
// $Header: g:/dwight/repo/cdc32/rcs/qtab.h 1.4 1996/05/22 04:11:32 dwight Stable095 $
#include <stdio.h>
#include <string.h>
#include "dwmapr.h"
#include "matcom.h"
#include "tcode.h"
#include "dwstr.h"
#define JPEG_INTERNALS
#include "jpeglib.h"

typedef ELTYPE (TCoder::*QTAB_MAXFUNP)();
typedef void (TCoder::*QTAB_RECONFUNP)(int, int);

class QTAB
{
public:
    static void qtab_init();
    static int Id;
    static INT16 aanscales[DCTSIZE2];
    static QTAB JustDctScales;

    DwString name;
    int id;
    dwINT32 q[MSZ][MSZ];
    dwINT32 f_dct[DCTSIZE2];
    dwINT32 i_dct[DCTSIZE2];
    dwINT32 f_dct_mul[DCTSIZE2];

    QTAB_MAXFUNP maxfun;
    QTAB_RECONFUNP reconfun;

    int operator()(int i, int j) {
        return q[i][j];
    }
    int& operator[](int i) {
        return ((dwINT32 *)q)[i];
    }

    QTAB(FILE *);
    int read(FILE *);
    void write(FILE *);
    void init_dct_tables();
protected:
    QTAB();
};

typedef DwMapR<QTAB *, DwString> QTABTAB;
typedef DwMapR<QTAB *, int> QIDTAB;
extern QTABTAB QTables;
extern QIDTAB QidTables;


#endif
