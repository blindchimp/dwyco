
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef ESDAUDIN_H
void DWYCOCALLCONV esd_new(void *, int buflen, int nbufs);
void DWYCOCALLCONV esd_delete(void *);
int DWYCOCALLCONV esd_init(void *);
int DWYCOCALLCONV esd_has_data(void *);
void DWYCOCALLCONV esd_need(void *);
void DWYCOCALLCONV esd_pass(void *);
void DWYCOCALLCONV esd_stop(void *);
void DWYCOCALLCONV esd_on(void *);
void DWYCOCALLCONV esd_off(void *);
void DWYCOCALLCONV esd_reset(void *);
void DWYCOCALLCONV esd_need(void *);
int DWYCOCALLCONV esd_status(void *);
void * DWYCOCALLCONV esd_get_data(void *, int *r, int *c);

#endif
