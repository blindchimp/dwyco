
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CDCVER_H
#define CDCVER_H
#include "vc.h"
#if defined(_Windows)
#define CDC_PLATFORM "win"
#elif defined(MACOSX)
#define CDC_PLATFORM "mac"
#elif defined(LINUX)

#if defined(ANDROID)
#define CDC_PLATFORM "linux-android"
#elif defined(DWYCO_IOS)
#define CDC_PLATFORM "linux-ios"
#else
#define CDC_PLATFORM "linux"
#endif

#else
#error "can't figure out platform, define _Windows, MACOSX, or LINUX"
#endif

#define IVERSION "CDCX-" CDC_PLATFORM "-" __DATE__ "-" __TIME__

vc dwyco_get_version_string();

#endif
