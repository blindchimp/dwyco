
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef DWYCO_RAND_H
#define DWYCO_RAND_H
#include <stdlib.h>
#if defined(ANDROID) || defined(_Windows) || defined(WIN32)
#define dwyco_rand() rand()
#define dwyco_srand(x) srand((x))
#else

extern unsigned int dwyco_rand_state;
#define dwyco_rand() rand_r(&dwyco_rand_state)
#define dwyco_srand(x) dwyco_rand_state = (x);
#endif

#endif
