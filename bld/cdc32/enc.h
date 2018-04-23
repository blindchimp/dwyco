
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
/******************************************************************************
* file: enc.h
* prototypes for enc.c (simple encryption)
*
******************************************************************************/

#ifndef ENC_H
#define ENC_H

/*
 * $Header: g:/dwight/repo/cdc32/rcs/enc.h 1.1 1996/11/02 11:58:06 dwight Stable095 $
 */


class Enc
{
public:
    Enc();
    void munge(char *buf, int count);
    void mungeback(char *buf, int count);
private:
    int keychar;

};


#endif
