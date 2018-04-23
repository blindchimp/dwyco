
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef RCT_H
#define RCT_H
#ifdef USE_RCT
#define RCBUFSIZE (4*1024*1024)
#define RCMASK (RCBUFSIZE - 1)
#define RCQINC(p) {*Inc_bp++ = (void*)(p); if(!((unsigned long)Inc_bp & RCMASK)) rcgc2();}
#define RCQDEC(p) {*Dec_bp++ = (void*)(p); if(!((unsigned long)Dec_bp & RCMASK)) rcgc2();}
extern void **Inc_bp;
extern void **Dec_bp;
void rcgc1();
void rcgc2();

#else
#define RCQINC(p)
#define RCQDEC(p)
#endif
#endif
