
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef QMSG_H
#define QMSG_H

// $Header: g:/dwight/repo/cdc32/rcs/qmsg.h 1.9 1999/01/10 16:11:01 dwight Checkpoint $

#include "dwstr.h"
#include "vc.h"
#include "mmchan.h"
#include "pval.h"

namespace dwyco {
extern int Rescan_msgs;
extern vc Cur_ignore;
extern vc No_direct_msgs;
extern vc No_direct_att;
extern vc Session_infos;
extern vc MsgFolders;
extern vc Pals;
}

//void load_users(int only_recent, int *total_out);
void load_users_from_files(int *total_out);
void load_users_from_index(int recent, int *total_out);
int remove_user(const vc &uid);
int clear_user(const vc &uid);
vc load_msgs(vc uid);
vc load_bodies(vc dir, int load_sent);
vc load_body_by_id(vc uid, vc msg_id);
vc load_qd_msgs(const vc &uid, int load_special);
vc load_qd_to_body(vc qid);
void query_messages();
MMChannel *fetch_attachment(vc fn, DestroyCallback, vc, void *, ValidPtr,
                            StatusCallback, void *, ValidPtr, vc server_ip = vcnil, vc server_port = vcnil);
vc save_body(vc msgid, vc from, vc text, vc attachment_id, vc date, vc rating, vc authvec,
             vc forwarded_body, vc new_text, vc no_forward, vc user_filename, vc logical_clock, vc special_type, vc from_group);
int uid_ignored(const vc &uid);
void delete_msg2(vc msgid);
void delete_body3(vc uid, vc msgid, int inhibit_indexing);

void delete_attachment2(vc user_id, vc msgid);
int q_message(vc recip, const char *attachment, DwString& fn_out,
              vc body_to_forward, const char *new_text, vc att_hash, vc special_type, vc st_arg1, int no_forward, vc user_filename, int save_sent);
void fetch_info(const vc &uid);
int qd_send_one();
int msg_outq_empty();
void qd_purge_outbox();
void move_back_to_outbox(const DwString&);
void move_to_outbox(const DwString&);
void move_in_progress(const DwString& fn, const DwString& dfn);
void remove_in_progress(const DwString& fn);
void remove_from_outbox(const DwString&);
void recover_inprogress();
void del_msg(vc vec, vc item);
vc uid_to_handle2(vc uid);
vc find_cur_msg(vc msg_id);
void got_ignore(vc m, void *, vc, ValidPtr);
void add_ignore(vc uid);
void del_ignore(vc uid);
void ack_all(vc);
//int save_to_inbox(vc m);
int store_direct(MMChannel *m, vc msg, void *);
vc direct_to_server(vc msgid);
void ack_all_direct();
void init_qmsg();
void exit_qmsg();
void suspend_qmsg();
void resume_qmsg();
int valid_qd_message(vc v);
vc get_local_ignore();
vc get_local_ignore_mapped();
vc get_local_pals();
void power_clean_safe();

// these are mainly for debugging now
// they move files to a "trash" folder instead of
// deleting them.
void trash_body(vc uid, vc msg_id, int inhibit_indexing);
void untrash_users();
int count_trashed_users();
int empty_trash();

vc uid_to_dir(const vc &uid);
vc dir_to_uid(DwString s);
void append_forwarded_text(DwString& s, vc body);
void append_forwarded_bodies(vc v, vc body);
vc get_body_text(vc body);
int verify_authentication(vc text, vc uid, vc att_filename, vc datevec, vc no_forward, vc mac);
int verify_chain(vc body, int top, vc att_hash, vc attachment_dir = vcnil);
vc strip_port(vc);
vc strip_chain(vc);
int check_profile_authenticator(vc m, vc uid);
vc gen_profile_authenticator(vc m, vc filehash);
vc gen_profile_checksum(vc m);
int check_profile_checksum(vc m, vc uid);
//int is_always_vis(vc uid);
//int is_never_vis(vc uid);
//void always_vis_add(vc uid);
//void always_vis_del(vc uid);
//void never_vis_add(vc uid);
//void never_vis_del(vc uid);
//void they_grant_add(vc who, vc cookie);
//void they_grant_del(vc who);
//vc they_grant_cookie(vc who);
//void i_grant_add(vc who, vc cookie);
//void i_grant_del(vc who);
//vc i_grant_cookie(vc who);
//void reset_i_grant();
//void reset_they_grant();
//void reset_always_vis();
//void reset_never_vis();
typedef DwVecP<WIN32_FIND_DATA> FindVec;
FindVec *find_to_vec(const char *pat);
void delete_findvec(FindVec *fv);
int creator_no_forward(vc body);
int any_no_forward(vc body);
int decode_no_forward_msg(vc m);
int encode_no_forward_msg(vc m);
int decode_no_forward_attachment(vc m);
int decode_no_forward_attachment_qqm(vc m);
int can_forward(vc body, vc att_dir);
DwString simple_diagnostics();
vc gen_hash(DwString filename);
vc direct_to_body(vc msgid, vc &uid_out);
vc direct_to_body2(vc dm);
int pal_add(vc u);
int pal_del(vc u, int norelogin = 0);
int pal_user(vc u);
int refile_attachment(vc filename, vc from_user);
void save_msg_idxs();
void save_qmsg_state();
// note: this returns the total count of messages,
// which is different than the number of entries in
// the current index.
vc do_local_store(vc filename, vc speced_mid);
vc make_best_local_info(const vc& uid, int *cant_resolve_now);
int init_msg_folder(vc uid);
int init_msg_folder(vc uid, DwString* fn_out);
vc encrypt_msg_qqm(vc msg_to_send, vc dhsf, vc ectx, vc key);
vc encrypt_msg_body(vc body, vc dhsf, vc ectx, vc key);
vc decrypt_msg_qqm(vc emsg);
vc decrypt_msg_body(vc body);
int can_decrypt_msg_body(vc body);
#ifdef DWYCO_CRYPTO_PIPELINE
vc decrypt_msg_body2(vc body, DwString& src, DwString& dst, DwString& key_out);
int decrypt_attachment2(const DwString& filename, const DwString& key, const DwString& filename_dst);
#endif
int encrypt_attachment(vc filename, vc key, vc filename_dst);
int decrypt_attachment(vc filename, vc key, vc filename_dst);

unsigned long uid_to_ip(vc uid, int& can_do_direct, int& prim, int& sec, int& pal);
int uid_online(vc uid);
int uid_online_display(const vc& uid);
void boost_clock(vc mi);
int move_replace(const DwString& s, const DwString& d);
vc pal_to_vector(int raw);
void clean_cruft();
void boost_logical_clock();
void update_global_logical_clock(int64_t lc);
int64_t diff_logical_clock(int64_t tm);

#define VERF_AUTH_NO_INFO 0x1
#define VERF_AUTH_FAILED 0x2
#define VERF_AUTH_OK 0x4
#define VERF_AUTH_THRU_OLD_FORWARD 0x8

#define AUTH_PROFILE_OK 0
#define AUTH_PROFILE_FAILED 1
#define AUTH_PROFILE_NO_INFO 2

// this is the summary info sent from the server
// use these to index a DWYCO_UNFETCHED_MSG_LIST

#define QM_FROM 0
#define QM_LEN 1
#define QM_ID 2
#define QM_DATE_SENT 3
// added locally:
//#define QM_PENDING_DEL 4
//#define QM_IS_DIRECT 5
// oops, server has to put this way out here because
// old software expects to see the above structure...
// 6: rating of sender
//#define QM_SENDER_RATING 6
//#define QM_ONLINE 7     // IP if they are online
//#define QM_FW_INFO 8	// firewall info vector(prim sec pal)
#define QM_SPECIAL_TYPE 9	// qm is a system special qm
// if the args are small enough, we can avoid a fetch by putting the
// the data for the special type into this part of the summary.
// used for delivery reports at the moment
#define QM_SPECIAL_TYPE_ARGS 10
// this can be used for ordering, it is available even for encrypted msgs
#define QM_LOGICAL_CLOCK 11

// message bodies as stored on client
// use these to index a DWYCO_SAVED_MSG_LIST

#define QM_BODY_ID 0
#define QM_BODY_FROM 1
#define QM_BODY_TEXT_do_not_use 2
#define QM_BODY_ATTACHMENT 3
#define QM_BODY_DATE 4

// locally set
#define QM_BODY_SENT 5

// from server
#define QM_BODY_RATING_do_not_use 6
// for 2.95 and above, the authentication
// vector
#define QM_BODY_AUTH_VEC 7
#define		QMBA_TEXT_HASH 0 // not used
#define		QMBA_TEXT_AUTH_HASH 1 // not used
#define		QMBA_ATTACHMENT_HASH 2 // not used
#define 	QMBA_ATTACHMENT_AUTH_HASH 3 // not used
#define		QMBA_COMB_AUTH_HASH 4
// this is used because we know old clients just store the
// entire authvec unmolested. note we also have to duplicate
// the flag in the main body because the authvec doesn't
// participate in the auth calculations, and we want the
// auth flag to enter into that calculation. this should
// be the same value as the other no_forward flag
#define		QMBA_NO_FORWARD 5
//#define QM_BODY_TIME_RECV 7 (defunct, don't think it is used now)
//#define QM_BODY_AUTOPLAY 8 (")

// new style forwarded
#define	QM_BODY_FORWARDED_BODY 8
#define QM_BODY_NEW_TEXT 9
#define QM_BODY_SPECIAL_TYPE 10
#define QM_BODY_NO_FORWARD 11
#define QM_BODY_FILE_ATTACHMENT 12
#define QM_BODY_DHSF 13
#define QM_BODY_EMSG 14 // recursive, when encrypted, this has the same structure
#define QM_BODY_ESTIMATED_SIZE 15 // estimated size of attachments
#define QM_BODY_NO_DELIVERY_REPORT 16 // t = don't give us a delivery report
#define QM_BODY_LOGICAL_CLOCK 17 // used for msg sorting since physical clocks are usually off
#define QM_BODY_FROM_GROUP 18

// message as q'd and ready to send to
// server or recipient (if direct)
#define QQM_RECIP_VEC 0
#define QQM_MSG_VEC 1
#define 	QQM_BODY_FROM 0
#define 	QQM_BODY_TEXT_do_not_use 1
#define 	QQM_BODY_ATTACHMENT 2
#define 	QQM_BODY_DATE 3
#define 	QQM_BODY_RATING_do_not_use 4
#define		QQM_BODY_AUTH_VEC 5
#define		QQM_BODY_FORWARDED_BODY 6 // another msg body
#define		QQM_BODY_NEW_TEXT 7		   // new style forwarded message use this for added text
#define		QQM_BODY_ATTACHMENT_LOCATION 8 // ip/port where attachment can be downloaded from.
#define		QQM_BODY_SPECIAL_TYPE 9 // for system messages, nil = normal
#define		QQM_BODY_NO_FORWARD 10
#define		QQM_BODY_FILE_ATTACHMENT 11 // nil = video file, otherwise, basename of file
#define 	QQM_BODY_DHSF 12
#define 	QQM_BODY_EMSG 13 // recursive, when encrypted, this has the same structure
#define     QQM_BODY_ESTIMATED_SIZE 14
#define     QQM_BODY_NO_DELIVERY_REPORT 15
#define     QQM_BODY_LOGICAL_CLOCK 16
#define     QQM_BODY_FROM_GROUP 17
#define QQM_LOCAL_ID 2 // added to direct messages locally
// for database-less operation, this info can be used
// when there is no other source of user info (ie, if
// the message is sent and the recipient cannot see the
// sender, possibly because of invisibility, being in the
// wrong server, etc.) this sucks because it is not
// authenticated in any way. should eventually be
// ignored. this info *is* ignored when using the database.
//#define QQM_INFO_VEC 3
// t is the message shoule be played automatically,
// used mostly for single-frame still messages
//#define QQM_AUTOPLAY 4
// reserved for storing info added by recipient (like time received)
//#define QQM_LOCAL_INFO_VEC 5
//#define		QQM_LIV_TIME_RECV 0
// t =  save it when the operation is successful
// nil = don't save it
// used for messages we want to discard as soon as sent, like auto-replies and stuff.
#define QQM_SAVE_SENT 6

// this is the format of the records in the msg index that is automatically
// generated by the dll. it is sorted on the logical clock.
// the logical clock should provide better ordering than the date if the
// two computers have dates that are off (which is almost always the case
// with android devices because apparently they don't handle leap seconds
// correctly and are off by some 15 seconds). if the logical clock is nil,
// this software falls-back to using the date

#define QM_IDX_DATE 0
#define QM_IDX_MID 1
#define QM_IDX_IS_SENT 2
#define QM_IDX_IS_FORWARDED 3
#define QM_IDX_IS_NO_FORWARD 4
#define QM_IDX_IS_FILE 5
#define QM_IDX_SPECIAL_TYPE 6
#define QM_IDX_HAS_ATTACHMENT 7
#define QM_IDX_ATT_HAS_VIDEO 8
#define QM_IDX_ATT_HAS_AUDIO 9
#define QM_IDX_ATT_IS_SHORT_VIDEO 10
#define QM_IDX_LOGICAL_CLOCK 11
#define QM_IDX_ASSOC_HUID 12
#define QM_IDX_FROM_GROUP 13
#define NUM_QM_IDX_FIELDS 14

// special message structure
//
// special messages are messages that
// contain something other than the usual
// text to be displayed to a user.
// they are sent/received in the normal
// manner as other messages, but they
// can contain extra information that
// clients can interpret in their own
// way. this allows clients to
// come up with thier own protocols outside
// the usual ones supported by the core.
// this also allows clients to be incompatible (sigh).
// it is crucial that clients ignore types they
// don't understand.
//
// special messages contain all the usual message
// fields, and they will be identical to a normal
// message in terms of what the fields mean (like the
// authentication, etc.) special messages cannot be
// forwarded at the moment.
//
// special messages have a field that includes
// all the items needed to process the message
// in this form:
// vector(type vector(type specific items))
//
// this special vector is inserted at a known location
// in a normal message.
// current uses of the special message structure are
// for processing pal authentication messages.
//
//

// max number of pals we process
// this is just here to avoid huge pal list from
// causing buffering problems in other parts of the
// system. this should really be fixed.
#define MAXPALS 300

#endif
