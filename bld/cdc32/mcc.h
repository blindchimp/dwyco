
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//---------------------------------------------------------------------------
#ifndef MCC_H
#define MCC_H
//---------------------------------------------------------------------------
#include "dlli.h"
#include "pval.h"
#include "dwstr.h"
#include "vc.h"
class MMChannel;
//---------------------------------------------------------------------------
class TMsgCompose
{
public:
    ValidPtr vp;
    int cancel_button_enabled;
    int record_button_enabled;
    int record_video;
    int record_audio;
    int record_pic;
    int stop_button_enabled;
    int play_button_enabled;
    int send_button_enabled;
    int start_over_button_enabled;

    void Close() ;
    void  cancel_buttonClick();
    int  record_buttonClick();
    void  stop_buttonClick();
    void  play_buttonClick(int no_audio = 0);
    void  start_over_buttonClick();
    void  FormCloseQuery(bool &CanClose);
    void  send_buttonClick();
    void  FormShow();

    int recording_audio;
    int recording;
    int stop_id;
    int stop_audio_id;
    int do_append;
    int last_audio_timecode;
    int view_id;
    DwString file_basename;
    DwString actual_filename;
    int changed;
    vc rid_list;
    vc recipient_list;
    int forward;
    DwString qfn;

    void reset_dialog();
    void init_av_buttons();
    void disable_most();
    void enable_most();
    vc msg_to_send;
    void forceClose();
    int force_close;
    vc filehash;
    int inhibit_hashing;
    int composer;
    int pal_auth_mode;
    int pal_auth_ok_mode;
    int pal_auth_rej_mode;
    vc body_to_forward;
    int no_forward;
    int limited_forward;
    vc user_filename;
    int hiq;
    // this is used in cases like auto-reply messages
    // where you don't want to save as a sent message
    int dont_save_sent;
    int special_type;
    vc special_payload;

    TMsgCompose();
    TMsgCompose(const TMsgCompose&);
    ~TMsgCompose();
    int do_record_pic();
    void step_done(MMChannel *);

    DwycoChannelDestroyCallback destroy_callback;
    void *dcb_arg1;
    void *dcb_arg2;
    DwString msg_text;

    DwycoStatusCallback status_callback;
    void *scb_arg1;

    int max_frames;
    int max_bytes;

    void  preview_play();
    int reset_composer();

};
#endif
