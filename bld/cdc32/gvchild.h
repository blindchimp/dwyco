
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/gvchild.h 1.21 1999/01/10 16:10:47 dwight Checkpoint $
 */
#ifndef GVCHILD_H
#define GVCHILD_H

int gen_new_grayview(int chan_id = -1);
void set_status_text_by_id(int id, const char *txt);
void set_caption_by_id(int id, const char *txt);
void set_indicator_by_id(int id, int, int val);
void set_audio_status(int);
void reset_audio_menus();
void set_squelch(int);

#define IND_SEND 0
#define IND_RECV 1

#endif
