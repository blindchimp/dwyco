
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dwlog.h 1.3 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef DWLOG_H
#define DWLOG_H

#include "vc.h"
#include "dwstr.h"

#define DWLOG_DEFAULT "dwyco.log"

class DwLog
{
private:
    DwString filename;

public:
#ifdef DWYCO_DO_USER_LOG
    DwLog(const char *filename = DWLOG_DEFAULT);
    ~DwLog();
    void make_entry(const char *str = 0, const char * = 0, const char * = 0);
    //void make_entry(vc v);
#else
    DwLog(const char *filename = DWLOG_DEFAULT) {
        this->filename = 0;
    }
    ~DwLog() {}
    void make_entry(const char *str = 0, const char * = 0, const char * = 0) {}
    void make_entry(vc v) {}
#endif
};
#endif
