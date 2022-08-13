
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/bcastq.h 1.16 1999/01/10 16:10:43 dwight Checkpoint $
 */
#ifndef BCASTQ_H
#define BCASTQ_H
#include <windows.h>
#include "matcom.h"
#include "dwvec.h"
#include "dwvecp.h"

class BroadcastQ
{
private:
    static int Dummy;

public:
    BroadcastQ();
    BroadcastQ(CRITICAL_SECTION& c);
    ~BroadcastQ();

    void add(DWBYTE *buf, int len, int timecode, int idx, int payload = 0);
    void remove(int = -1, int dofree = 1);
    int buf_with_idx(int idx, DWBYTE*& buf, int& len, int& timecode,
                     int& payload = BroadcastQ::Dummy);
    int fifo_buf(DWBYTE*& buf, int& len, int& timecode, int& idx,
                 int& payload = BroadcastQ::Dummy);
    DwVecIter<int> get_idx_iter();
    int num_elems();

    HANDLE get_event();

private:
    DwVec<int> idxs;
    DwVecP<DWBYTE> bufs;
    DwVec<int> lens;
    DwVec<int> timecodes;
    DwVec<int> payloads;

    int shared;
    CRITICAL_SECTION& cs;
    HANDLE sema;
};

#endif
