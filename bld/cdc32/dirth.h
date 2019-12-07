
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/

/*
 * $Header: g:/dwight/repo/cdc32/rcs/dirth.h 1.14 1999/01/10 16:10:46 dwight Checkpoint $
 */
#ifndef DIRTH_H
#define DIRTH_H

#include "vc.h"
#include "mmchan.h"

class MessageDisplay;

void start_database_thread();
void end_database_thread();
void init_dirth();
void init_home_server();
vc build_directory_entry();
vc build_directory_entry2();
long get_disk_serial();
//vc system_info();
vc get_server_ip_by_uid(vc, vc&);
vc get_server_name_by_uid(vc, vc&);
vc get_random_xfer_server_ip(vc& port);
vc get_random_server_ip(vc server_list, vc& port);
bool contains_xfer_ip(vc ip);
int dirth_switch_to_server(int, MessageDisplay * = 0);
//int dirth_switch_to_server2(const char *id, MessageDisplay *md = 0);
int dirth_switch_to_chat_server(int, const char *pw);
vc make_local_ports();
vc make_fw_setup();

extern int Database_online;
extern vc My_server_port;
extern vc My_server_ip;
extern vc My_server_name;
extern vc STUN_server_list;
extern vc BW_server_list;


#define SL_SERVER_HOSTNAME 0
#define SL_SERVER_IP 1
#define SL_SERVER_PORT 2
#define SL_SERVER_RATING 3
#define SL_SERVER_NAME 4
#define SL_SERVER_CLASS 5
#define SL_SERVER_BACKUP 6
#define SL_SERVER_CATEGORY 5

#endif
