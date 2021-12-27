
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/bcastq.cc 1.16 1999/01/10 16:09:28 dwight Checkpoint $
 */
#include "bcastq.h"
// note: HACK alert: the only shared Q is the audio block q,
// so this is not quite right...
[[noreturn]] void oopanic(const char *);

int BroadcastQ::Dummy;

BroadcastQ::BroadcastQ() :
    idxs(0, 0, 1), bufs(0, 0, 1), lens(0, 0, 1), timecodes(0, 0, 1),
    payloads(0, 0, 1), cs(*new CRITICAL_SECTION)
{
    shared = 0;
    sema = 0;
}

BroadcastQ::BroadcastQ(CRITICAL_SECTION& c) :
    idxs(0, 0, 1), bufs(0, 0, 1), lens(0, 0, 1), timecodes(0, 0, 1),
    payloads(0, 0, 1), cs(c)
{
    shared = 1;
    sema = CreateEvent(0, 0, 0, 0);
    if(sema == 0)
        oopanic("bcast null sema");
}

BroadcastQ::~BroadcastQ()
{
    if(shared)
    {
        EnterCriticalSection(&cs);
    }

    int i = idxs.num_elems();
    while(i-- > 0)
    {
        delete [] bufs[i];
    }

    if(shared)
    {
        LeaveCriticalSection(&cs);
        //DeleteCriticalSection(&cs);
        CloseHandle(sema);
    }
    else
        delete &cs;
}

HANDLE
BroadcastQ::get_event()
{
    return sema;
}

void
BroadcastQ::add(DWBYTE *buf, int len, int timecode, int idx, int payload)
{
    if(shared)
    {
        EnterCriticalSection(&cs);
    }

    bufs.append(buf);
    lens.append(len);
    idxs.append(idx);
    timecodes.append(timecode);
    payloads.append(payload);

    if(shared)
    {
        SetEvent(sema);
        LeaveCriticalSection(&cs);
    }
}

void
BroadcastQ::remove(int idx, int dofree)
{
    if(shared)
    {
        EnterCriticalSection(&cs);
    }
    if(idxs.num_elems() == 0)
        goto out;
    if(idx != -1 && idxs[0] != idx)
        oopanic("bcast bad remove");
    if(dofree)
        delete [] bufs[0];
    bufs.del(0);
    lens.del(0);
    idxs.del(0);
    timecodes.del(0);
    payloads.del(0);
out:
    if(shared)
    {
        LeaveCriticalSection(&cs);
    }
}

int
BroadcastQ::buf_with_idx(int idx, DWBYTE*& buf, int& len, int& timecode,
                         int& payload)
{
    if(shared)
        EnterCriticalSection(&cs);

    int i;
    int ret = 1;
    if((i = idxs.index(idx)) == -1)
    {
        ret = 0;
        goto out;
    }
    buf = bufs[i];
    len = lens[i];
    timecode = timecodes[i];
    payload = payloads[i];
out:
    if(shared)
        LeaveCriticalSection(&cs);
    return ret;
}

int
BroadcastQ::fifo_buf(DWBYTE*& buf, int& len, int& timecode, int& idx,
                     int& payload)
{
    if(shared)
        EnterCriticalSection(&cs);
    int ret = 1;
    if(idxs.num_elems() == 0)
    {
        ret = 0;
        goto out;
    }
    buf = bufs[0];
    len = lens[0];
    timecode = timecodes[0];
    idx = idxs[0];
    payload = payloads[0];
out:
    if(shared)
        LeaveCriticalSection(&cs);
    return ret;
}

DwVecIter<int>
BroadcastQ::get_idx_iter()
{
    if(shared)
        oopanic("shared bcast iter");
    return DwVecIter<int>(&idxs);
}

int
BroadcastQ::num_elems()
{
    if(shared)
        EnterCriticalSection(&cs);

    int tmp = idxs.num_elems();

    if(shared)
        LeaveCriticalSection(&cs);

    return tmp;
}
