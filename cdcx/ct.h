
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef CT_H
#define CT_H

#include "dwstr.h"

#define DWYCO_CT_ORIG 0
#define DWYCO_CT_RECV 1
#define DWYCO_CT_ANY 2
struct dwyco_call_type
{
    dwyco_call_type(int cid, const char *acall_type, int len_call_type,
                    const char *auid, int len_uid, int adir = DWYCO_CT_RECV) {
        chan_id = cid;
        this->call_type = DwOString(acall_type, 0, len_call_type);
        this->uid = DwOString(auid, 0, len_uid);
        this->dir = adir;
    }
    dwyco_call_type() {
        chan_id = -1;
    }

    int chan_id;
    int remote_wants_to_recv_your_video;
    int remote_wants_to_send_you_video;
    int remote_wants_to_recv_your_audio;
    int remote_wants_to_send_you_audio;
    int remote_wants_to_exchange_pubchat;
    int remote_wants_to_exchange_privchat;
    DwOString call_type;
    DwOString uid;
    int dir;
};

void call_add(const dwyco_call_type& c);
void call_del(int chan_id);
void call_del_by_uid(const DwOString& uid);
int call_destroy_by_uid(const DwOString& uid);
int call_destroy_by_chan_id(int chan_id);
int call_destroy_by_type(const char *type, int call_dir);
int call_destroy_all();
int call_exists(int chan_id);
int call_exists_by_type(const char *type, int call_dir);
void call_del(int chan_id);
int call_find(int chan_id, dwyco_call_type& call_out);
int call_destroy_by_mask(const char *type, int dir,
                         int sending_video, int receiving_video,
                         int sending_audio, int receiving_audio,
                         int pubchat, int privchat);

int call_exists_by_type_to_uid(const char *type, const DwOString& uid, int dir);
int call_exists_by_uid(const DwOString& uid);
int is_video_streaming_out();
int is_audio_streaming_out();
#endif
