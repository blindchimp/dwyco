
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef AUDI_QT_H
#define AUDI_QT_H
#include "dlli.h"

void DWYCOCALLCONV audi_qt_new(void *, int buflen, int nbufs);
void DWYCOCALLCONV audi_qt_delete(void *);
int DWYCOCALLCONV audi_qt_init(void *);
int DWYCOCALLCONV audi_qt_has_data(void *);
void DWYCOCALLCONV audi_qt_need(void *);
void DWYCOCALLCONV audi_qt_pass(void *);
void DWYCOCALLCONV audi_qt_stop(void *);
void DWYCOCALLCONV audi_qt_on(void *);
void DWYCOCALLCONV audi_qt_off(void *);
void DWYCOCALLCONV audi_qt_reset(void *);
int DWYCOCALLCONV audi_qt_status(void *);
void * DWYCOCALLCONV audi_qt_get_data(void *, int *r, int *c);


#endif // AUDI_QT_H
