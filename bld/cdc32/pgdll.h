
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
//---------------------------------------------------------------------------

#ifndef pgdllH
#define pgdllH
//---------------------------------------------------------------------------
#include "vc.h"
#include "pval.h"

namespace dwyco {
class ProfileGrid
{
public:
    ProfileGrid();
    ~ProfileGrid();

    void add_user(const vc &name, const vc &uid);
    void remove_user(vc uid);
    void update_ah(vc uid);
    //void display_video_in_listbox(pixel **img, int cols, int rows, vc uid);
    ValidPtr vp;
    //TPopupMenu dir_menu;
    void start_update();
    void end_update();

    // audio q related stuff
    void addq(vc q, vc uid, vc pos);
    void delq(vc q, vc uid);
    void data(vc q, vc uid, vc data);
    void grant(vc q, vc uid, vc tm);

    // attribute updates

    void sys_attr(const vc& uid, const vc& name, const vc& val);
    void update_attr(const vc &uid, const vc &name, const vc &val);

    // user lobbies
    void add_user_lobby(vc info);
    void del_user_lobby(vc id);

    // god tracking
    void god_online(vc uid, vc info);
    void god_offline(vc uid);

    // simple data from a channel
    void simple_data(vc qid, vc display_name, vc uid, vc data);
};

// this is a little bogus making this a static global, but
// it suits our needs at the moment (there is a minor problem
// in that you really want to be able to access the user lobby
// list even when a chat context isn't active... needs to be
// resolved somehow.)
extern vc Current_user_lobbies;
#define SL_ULOBBY_ID 0
#define SL_ULOBBY_HOSTNAME 1
#define SL_ULOBBY_IP 2
#define SL_ULOBBY_PORT 3
#define SL_ULOBBY_RATING 4
#define SL_ULOBBY_DISPLAY_NAME 5
#define SL_ULOBBY_CATEGORY 6
#define SL_ULOBBY_BACKUP 7
#define SL_ULOBBY_internal_pvt_name 8
#define SL_ULOBBY_MAX_USERS 9
#define SL_ULOBBY_SUBGOD_UID 10
#define SL_ULOBBY_PW 11

extern vc Current_gods;
#define GT_NAME 0
#define GT_WHERE 1
#define GT_GOD 2
#define GT_DEMIGOD 3
#define GT_SUBGOD 4
#define GT_SERVER_ID 5

int dllify(vc v, const char*& str_out, int& len_out);
}

#endif
