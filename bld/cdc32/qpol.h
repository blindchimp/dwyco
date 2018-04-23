
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/qpol.h 1.5 1997/11/25 20:41:03 dwight Stable095 $
 */
#ifndef QPOL_H
#define QPOL_H
#include "dwvec.h"
#include "dwmapr.h"
#include "dwstr.h"

// vanilla coding policy style
#define MSE_CLOSE 0
#define STOPPED_REFINING_NO_MOTION 1
#define STOPPED_REFINING_MOTION 2
#define REFINING_MOTION 3
#define REFINING_NO_MOTION 4
#define REFERENCE 5
#define Q_CHROMA_OFFSET 6
// these are high-variance quantizers
#define HV_OFFSET 6
#define HV_MSE_CLOSE 6
#define HV_STOPPED_REFINING_NO_MOTION 7
#define HV_STOPPED_REFINING_MOTION 8
#define HV_REFINING_MOTION 9
#define HV_REFINING_NO_MOTION 10
#define HV_REFERENCE 11
// here are entropy models
#define EM_OFFSET 12

// these are policies for codec that does
// foveation. the center val is the value
// that is the max quality. the codec
// then computes a line from
// FOV_CENTER to FOV_CENTER + FOV_MAX_OFFSET
// and uses the quantization tables
// in that range radially out from the center
// of the picture.
#define FOV_CENTER_VAL 0
#define FOV_MAX_OFFSET 1

// a policy is a group of quantizers
// each policy can have a different set of quantizers
// and they may be used in totally different ways, so
// there is no attempt to structure them...
typedef DwVec<DwString> QGROUP;


class QPolicy
{
public:
    QGROUP names; // normally quantizer names, but not necessarily so
};

typedef DwMapR<QPolicy *, DwString> PGROUP;
typedef DwMapRIter<QPolicy *, DwString> PolicyIter;
extern PGROUP Policy;

void qpol_init();

#endif
