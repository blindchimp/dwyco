static DwycoChatCtxCallback  user_dwyco_set_chat_ctx_callback;
static DwycoChatCtxCallback2  user_dwyco_set_chat_ctx_callback2;
static DwycoStatusCallback  user_dwyco_set_debug_message_callback;
static DwycoVideoDisplayCallback  user_dwyco_set_video_display_callback;
static DwycoCallAppearanceCallback  user_dwyco_set_call_acceptance_callback;
static DwycoEmergencyCallback  user_dwyco_set_emergency_callback;
static DwycoUserControlCallback  user_dwyco_set_user_control_callback;
static DwycoCallScreeningCallback  user_dwyco_set_call_screening_callback;
static DwycoStatusCallback  user_dwyco_set_call_bandwidth_callback;
static DwycoServerLoginCallback  user_dwyco_set_login_result_callback;
static DwycoPublicChatInitCallback  user_dwyco_set_public_chat_init_callback;
static DwycoPrivateChatInitCallback  user_dwyco_set_private_chat_init_callback;
static DwycoPrivateChatDisplayCallback  user_dwyco_set_private_chat_display_callback;
static DwycoPublicChatDisplayCallback  user_dwyco_set_public_chat_display_callback;
static DwycoPublicChatDisplayCallback  user_dwyco_set_bgapp_msg_callback;
static DwycoCallAppearanceCallback  user_dwyco_set_call_appearance_callback;
static DwycoCallAppearanceDeathCallback  user_dwyco_set_call_appearance_death_callback;
static DwycoZapAppearanceCallback  user_dwyco_set_zap_appearance_callback;
static DwycoAutoUpdateStatusCallback  user_dwyco_set_autoupdate_status_callback;
extern "C" {
DWYCOEXPORT void _real_dwyco_field_debug(const char *var, int num);
DWYCOEXPORT void _real_dwyco_debug_dump();
DWYCOEXPORT void _real_dwyco_random_string2(char **str_out, int len);
DWYCOEXPORT void _real_dwyco_eze2(const char *str, int len_str, char **str_out, int *len_out);
DWYCOEXPORT void _real_dwyco_ezd2(const char *str, int len_str, char **str_out, int *len_out);
DWYCOEXPORT void _real_dwyco_set_fn_prefixes( const char *sys_pfx, const char *user_pfx, const char *tmp_pfx);
DWYCOEXPORT int _real_dwyco_get_suspend_state();
DWYCOEXPORT void _real_dwyco_suspend();
DWYCOEXPORT void _real_dwyco_resume();
DWYCOEXPORT void _real_dwyco_set_chat_ctx_callback(DwycoChatCtxCallback cb);
DWYCOEXPORT void _real_dwyco_set_chat_ctx_callback2(DwycoChatCtxCallback2 cb);
DWYCOEXPORT void _real_dwyco_set_debug_message_callback(DwycoStatusCallback cb);
DWYCOEXPORT void _real_dwyco_set_video_display_callback(DwycoVideoDisplayCallback cb);
DWYCOEXPORT void _real_dwyco_set_call_acceptance_callback(DwycoCallAppearanceCallback cb);
DWYCOEXPORT void _real_dwyco_set_emergency_callback(DwycoEmergencyCallback cb);
DWYCOEXPORT void _real_dwyco_set_user_control_callback(DwycoUserControlCallback cb);
DWYCOEXPORT void _real_dwyco_set_call_screening_callback(DwycoCallScreeningCallback cb);
DWYCOEXPORT void _real_dwyco_set_call_bandwidth_callback(DwycoStatusCallback cb);
DWYCOEXPORT void _real_dwyco_finish_startup();
DWYCOEXPORT void _real_dwyco_set_local_auth(int a);
DWYCOEXPORT void _real_dwyco_gen_pass(const char *pw, int len_pw, char **salt_in_out, int *len_salt_in_out, char **hash_out, int *len_hash_out);
DWYCOEXPORT int _real_dwyco_get_create_new_account();
DWYCOEXPORT void _real_dwyco_free(char *p_elide);
DWYCOEXPORT void _real_dwyco_free_array(char *p_elide);
DWYCOEXPORT void _real_dwyco_free_image(char *p_elide, int rows);
DWYCOEXPORT void _real_dwyco_enable_activity_checking(int on, int timeout, DwycoActivityCallback cb);
DWYCOEXPORT void _real_dwyco_set_inactivity_time(int secs);
DWYCOEXPORT int _real_dwyco_chat_set_activity_state(int active, const char *state, int len_state);
DWYCOEXPORT void _real_dwyco_set_client_version(const char *str, int len_str);
DWYCOEXPORT int _real_dwyco_init();
DWYCOEXPORT int _real_dwyco_bg_init();
DWYCOEXPORT int _real_dwyco_bg_exit();
DWYCOEXPORT int _real_dwyco_update_server_list(const char *lhxfer_str, int lhxfer_str_len);
DWYCOEXPORT void _real_dwyco_power_clean_safe();
DWYCOEXPORT int _real_dwyco_get_authenticator(const char **a_out, int *len_a_out);
DWYCOEXPORT void _real_dwyco_set_login_result_callback(DwycoServerLoginCallback cb);
DWYCOEXPORT void _real_dwyco_database_login();
DWYCOEXPORT int _real_dwyco_database_online();
DWYCOEXPORT int _real_dwyco_chat_online();
DWYCOEXPORT int _real_dwyco_database_auth_remote();
DWYCOEXPORT void _real_dwyco_inhibit_database(int i);
DWYCOEXPORT void _real_dwyco_inhibit_pal(int i);
DWYCOEXPORT void _real_dwyco_inhibit_sac(int i);
DWYCOEXPORT void _real_dwyco_inhibit_incoming_sac(int i);
DWYCOEXPORT void _real_dwyco_inhibit_outgoing_sac(int i);
DWYCOEXPORT void _real_dwyco_inhibit_all_incoming(int i);
DWYCOEXPORT void _real_dwyco_set_disposition(const char *str, int len_str);
DWYCOEXPORT void _real_dwyco_fetch_info(const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_service_channels(int *spin_out);
DWYCOEXPORT void _real_dwyco_add_entropy_timer(const char *crap, int len_crap);
DWYCOEXPORT void _real_dwyco_get_my_uid(const char **uid_out, int *len_out);
DWYCOEXPORT int _real_dwyco_enable_video_capture_preview(int on);
DWYCOEXPORT void _real_dwyco_destroy_channel(int chan_id);
DWYCOEXPORT void _real_dwyco_hangup_all_calls();
DWYCOEXPORT void _real_dwyco_cancel_call(int call_id);
DWYCOEXPORT void _real_dwyco_pause_all_channels(int p);
DWYCOEXPORT int _real_dwyco_pause_channel_media_set(int chan_id, int pause_video, int pause_audio);
DWYCOEXPORT int _real_dwyco_pause_channel_media_get(int chan_id, int *pause_video_out, int *pause_audio_out);
DWYCOEXPORT int _real_dwyco_remote_pause_channel_media_get(int chan_id, int *pause_video_out, int *pause_audio_out);
DWYCOEXPORT void _real_dwyco_set_channel_destroy_callback(int chan_id, DwycoChannelDestroyCallback cb, void *user_arg);
DWYCOEXPORT int _real_dwyco_get_audio_hw(int *has_audio_input_out, int *has_audio_output_out, int *audio_hw_full_duplex_out);
DWYCOEXPORT int _real_dwyco_set_all_mute(int a);
DWYCOEXPORT int _real_dwyco_get_all_mute();
DWYCOEXPORT int _real_dwyco_set_exclusive_audio(int a, int recv_chan_id);
DWYCOEXPORT int _real_dwyco_get_exclusive_audio(int *state_out, int *chan_id_out);
DWYCOEXPORT int _real_dwyco_set_auto_squelch(int a);
DWYCOEXPORT int _real_dwyco_get_auto_squelch();
DWYCOEXPORT int _real_dwyco_get_squelched();
DWYCOEXPORT void _real_dwyco_set_full_duplex(int a);
DWYCOEXPORT int _real_dwyco_get_full_duplex();
DWYCOEXPORT int _real_dwyco_get_audio_output_in_progress();
DWYCOEXPORT int _real_dwyco_chat_addq(int q);
DWYCOEXPORT int _real_dwyco_chat_delq(int q, const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_chat_talk(int q);
DWYCOEXPORT int _real_dwyco_chat_mute(int q, int on);
DWYCOEXPORT int _real_dwyco_chat_set_filter(int q, int gods_only,  int demigods_only, const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_chat_set_demigod(const char *uid, int len_uid, int on);
DWYCOEXPORT int _real_dwyco_chat_clear_all_demigods();
DWYCOEXPORT int _real_dwyco_chat_set_unblock_time2(int q, const char *uid, int len_uid, int tm, const char *reason);
DWYCOEXPORT int _real_dwyco_chat_set_unblock_time(int q, const char *uid, int len_uid, int tm);
DWYCOEXPORT int _real_dwyco_chat_get_admin_info();
DWYCOEXPORT int _real_dwyco_chat_send_popup(const char *text, int len_text, int global);
DWYCOEXPORT int _real_dwyco_chat_set_sys_attr(const char *name, int name_len, int dwyco_type, const char *val, int val_len, int int_val);
DWYCOEXPORT void _real_dwyco_chat_create_user_lobby(const char *dispname,  const char *category, const char *sub_god_uid, int len_sub_god_uid, const char *pw, int user_limit, DwycoCommandCallback cb, void *user_arg);
DWYCOEXPORT void _real_dwyco_chat_remove_user_lobby(const char *lobby_id, DwycoCommandCallback cb, void *user_arg);
DWYCOEXPORT void _real_dwyco_chat_send_data(const char *txt, int txt_len, int pic_type, const char *pic_data, int pic_data_len);
DWYCOEXPORT void _real_dwyco_set_moron_dork_mode(int m);
DWYCOEXPORT int _real_dwyco_get_moron_dork_mode();
DWYCOEXPORT void _real_dwyco_set_public_chat_init_callback(DwycoPublicChatInitCallback cb);
DWYCOEXPORT void _real_dwyco_set_private_chat_init_callback(DwycoPrivateChatInitCallback cb);
DWYCOEXPORT void _real_dwyco_set_private_chat_display_callback(DwycoPrivateChatDisplayCallback cb);
DWYCOEXPORT void _real_dwyco_set_public_chat_display_callback(DwycoPublicChatDisplayCallback cb);
DWYCOEXPORT void _real_dwyco_set_bgapp_msg_callback(DwycoPublicChatDisplayCallback cb);
DWYCOEXPORT void _real_dwyco_command_from_keyboard(int chan_id, int com, int arg1, int arg2, const char *str, int len);
DWYCOEXPORT void _real_dwyco_line_from_keyboard(int id, const char *line, int len);
DWYCOEXPORT int _real_dwyco_selective_chat_recipient_enable(const char *uid, int len_uid, int enable);
DWYCOEXPORT int _real_dwyco_selective_chat_enable(int e);
DWYCOEXPORT void _real_dwyco_reset_selective_chat_recipients();
DWYCOEXPORT int _real_dwyco_is_selective_chat_recipient(const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_set_max_established_originated_calls(int n);
DWYCOEXPORT int _real_dwyco_channel_create(const char *uid, int len_uid, DwycoCallDispositionCallback cdc, void *cdc_arg1, DwycoStatusCallback scb, void *scb_arg1, const char *pw, const char *call_type, int len_call_type, int q_call);
DWYCOEXPORT int _real_dwyco_channel_send_video(int chan_id, int vid_dev);
DWYCOEXPORT int _real_dwyco_channel_stop_send_video(int chan_id);
DWYCOEXPORT int _real_dwyco_channel_send_audio(int chan_id, int aud_dev);
DWYCOEXPORT int _real_dwyco_channel_stop_send_audio(int chan_id);
DWYCOEXPORT int _real_dwyco_connect_uid(const char *uid, int len_uid, DwycoCallDispositionCallback cdc, void *cdc_arg1, DwycoStatusCallback scb, void *scb_arg1, int send_video, int recv_video, int send_audio, int recv_audio, int private_chat, int public_chat, const char *pw, int len_pw, const char *call_type, int len_call_type, int q_call);
DWYCOEXPORT int _real_dwyco_chan_to_call(int chan_id);
DWYCOEXPORT int _real_dwyco_channel_streams(int chan_id, int *send_video_out, int *recv_video_out, int *send_audio_out, int *recv_audio_out, int *pubchat_out, int *privchat_out);
DWYCOEXPORT int _real_dwyco_connect_msg_chan(const char *uid, int len_uid, DwycoCallDispositionCallback cdc, void *cdc_arg1, DwycoStatusCallback scb, void *scb_arg1);
DWYCOEXPORT int _real_dwyco_send_user_control(const char *uid, int len_uid, const char *data, int len_data);
DWYCOEXPORT void _real_dwyco_set_call_appearance_callback(DwycoCallAppearanceCallback cb);
DWYCOEXPORT void _real_dwyco_set_call_appearance_death_callback(DwycoCallAppearanceDeathCallback cb);
DWYCOEXPORT int _real_dwyco_call_accept(int id);
DWYCOEXPORT int _real_dwyco_call_reject(int id, int session_ignore);
DWYCOEXPORT void _real_dwyco_set_zap_appearance_callback(DwycoZapAppearanceCallback cb);
DWYCOEXPORT int _real_dwyco_zap_accept(int id, int always_accept);
DWYCOEXPORT int _real_dwyco_zap_reject(int id, int session_ignore);
DWYCOEXPORT int _real_dwyco_get_server_list(DWYCO_SERVER_LIST *list_out, int *numlines_out);
DWYCOEXPORT int _real_dwyco_get_lobby_name_by_id2(const char *id, DWYCO_LIST *list_out);
DWYCOEXPORT int _real_dwyco_switch_to_chat_server(int i);
DWYCOEXPORT int _real_dwyco_check_chat_server_pw(const char *cid, const char *pw);
DWYCOEXPORT int _real_dwyco_chat_server_has_pw(const char *cid);
DWYCOEXPORT int _real_dwyco_switch_to_chat_server2(const char *cid, const char *pw);
DWYCOEXPORT int _real_dwyco_disconnect_chat_server();
DWYCOEXPORT void _real_dwyco_set_initial_invis(int invis);
DWYCOEXPORT void _real_dwyco_set_invisible_state(int invis);
DWYCOEXPORT int _real_dwyco_get_invisible_state();
DWYCOEXPORT int _real_dwyco_delete_user(const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_clear_user(const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_clear_user_unfav(const char *uid, int len_uid);
DWYCOEXPORT void _real_dwyco_pal_relogin();
DWYCOEXPORT int _real_dwyco_get_pal_logged_in();
DWYCOEXPORT int _real_dwyco_empty_trash();
DWYCOEXPORT int _real_dwyco_count_trashed_users();
DWYCOEXPORT void _real_dwyco_untrash_users();
DWYCOEXPORT int _real_dwyco_set_profile_from_composer(int compid, const char *txt, int txt_len, DwycoProfileCallback cb, void *arg);
DWYCOEXPORT int _real_dwyco_get_profile_to_viewer(const char *uid, int len_uid, DwycoProfileCallback cb, void *arg);
DWYCOEXPORT int _real_dwyco_get_profile_to_viewer_sync(const char *uid, int len_uid, char **fn_out, int *len_fn_out);
DWYCOEXPORT void _real_dwyco_name_to_uid(const char *handle, int len_handle);
DWYCOEXPORT int _real_dwyco_map_uid_to_representative(const char *uid, int len_uid, DWYCO_LIST *list_out);
DWYCOEXPORT int _real_dwyco_create_bootstrap_profile(const char *handle, int len_handle, const char *desc, int len_desc, const char *loc, int len_loc, const char *email, int len_email);
DWYCOEXPORT int _real_dwyco_make_profile_pack(const char *handle, int len_handle, const char *desc, int len_desc, const char *loc, int len_loc, const char *email, int len_email, const char **str_out, int *len_str_out);
DWYCOEXPORT int _real_dwyco_set_setting(const char *name, const char *value);
DWYCOEXPORT int _real_dwyco_get_setting(const char *name, const char **value_out, int *len_out, int *dwyco_type_out);
DWYCOEXPORT int _real_dwyco_make_zap_composition( char *dum);
DWYCOEXPORT int _real_dwyco_make_zap_composition_raw(const char *filename, const char *possible_extension);
DWYCOEXPORT int _real_dwyco_dup_zap_composition(int compid);
DWYCOEXPORT int _real_dwyco_make_forward_zap_composition2(const char *msg_id, int strip_forward_text);
DWYCOEXPORT int _real_dwyco_is_forward_composition(int compid);
DWYCOEXPORT int _real_dwyco_flim(int compid);
DWYCOEXPORT int _real_dwyco_is_file_zap(int compid);
DWYCOEXPORT int _real_dwyco_make_special_zap_composition( int special_type, const char *user_block, int len_user_block);
DWYCOEXPORT int _real_dwyco_set_special_zap(int compid, int special_type);
DWYCOEXPORT int _real_dwyco_make_file_zap_composition( const char *filename, int len_filename);
DWYCOEXPORT int _real_dwyco_copy_out_qd_file_zap(DWYCO_SAVED_MSG_LIST m, const char *dst_filename);
DWYCOEXPORT int _real_dwyco_copy_out_file_zap2(const char *msg_id, const char *dst_filename);
DWYCOEXPORT int _real_dwyco_copy_out_file_zap_buf2(const char *msg_id, const char **buf_out, int *buf_len_out, int max);
DWYCOEXPORT int _real_dwyco_delete_zap_composition(int compid);
DWYCOEXPORT int _real_dwyco_zap_record2(int compid, int video, int audio, int max_frames, int max_bytes, int hi_quality, DwycoStatusCallback scb, void *scb_arg1, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *chan_id_out);
DWYCOEXPORT int _real_dwyco_zap_record(int compid, int video, int audio, int pic, int frames, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out);
DWYCOEXPORT int _real_dwyco_zap_composition_chan_id(int compid);
DWYCOEXPORT int _real_dwyco_zap_stop(int compid);
DWYCOEXPORT int _real_dwyco_zap_play(int compid, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out);
DWYCOEXPORT int _real_dwyco_zap_send4(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, const char **pers_id_out, int *len_pers_id_out);
DWYCOEXPORT int _real_dwyco_zap_send5(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, int save_sent, const char **pers_id_out, int *len_pers_id_out);
DWYCOEXPORT int _real_dwyco_zap_send6(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, int save_sent, int defer, const char **pers_id_out, int *len_pers_id_out);
DWYCOEXPORT int _real_dwyco_zap_cancel(int compid);
DWYCOEXPORT int _real_dwyco_zap_still_active(int compid);
DWYCOEXPORT int _real_dwyco_kill_message(const char *pers_id, int len_pers_id);
DWYCOEXPORT int _real_dwyco_make_zap_view2(DWYCO_SAVED_MSG_LIST list, int qd);
DWYCOEXPORT int _real_dwyco_make_zap_view_file(const char *filename);
DWYCOEXPORT int _real_dwyco_make_zap_view_file_raw(const char *filename);
DWYCOEXPORT int _real_dwyco_delete_zap_view(int compid);
DWYCOEXPORT int _real_dwyco_zap_play_view(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out);
DWYCOEXPORT int _real_dwyco_zap_play_view_no_audio(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out);
DWYCOEXPORT int _real_dwyco_zap_play_preview(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out);
DWYCOEXPORT int _real_dwyco_zap_stop_view(int viewid);
DWYCOEXPORT int _real_dwyco_zap_quick_stats_view(int viewid, int *has_video_out, int *has_audio_out, int *short_video_out);
DWYCOEXPORT int _real_dwyco_zap_create_preview_buf(int viewid, const char **buf_out_elide, int *len_out, int *cols_out, int *rows_out);
DWYCOEXPORT int _real_dwyco_zap_create_preview(int viewid, const char *filename, int len_filename);
DWYCOEXPORT int _real_dwyco_get_refresh_users();
DWYCOEXPORT void _real_dwyco_set_refresh_users(int i);
DWYCOEXPORT int _real_dwyco_get_rescan_messages();
DWYCOEXPORT void _real_dwyco_set_rescan_messages(int i);
DWYCOEXPORT int _real_dwyco_uid_online(const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_uid_status(const char *uid, int len_uid);
DWYCOEXPORT void _real_dwyco_uid_to_ip(const char *uid, int len_uid, int *can_do_direct_out, char **str_out);
DWYCOEXPORT int _real_dwyco_uid_to_ip2(const char *uid, int len_uid, int *can_do_direct_out, char **str_out);
DWYCOEXPORT int _real_dwyco_uid_g(const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_load_users_internal();
DWYCOEXPORT int _real_dwyco_load_users2(int recent, int *total_out);
DWYCOEXPORT int _real_dwyco_get_user_list2(DWYCO_USER_LIST *list_out, int *nelems_out);
DWYCOEXPORT int _real_dwyco_get_updated_uids(DWYCO_USER_LIST *list_out, long time);
DWYCOEXPORT int _real_dwyco_get_message_index(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_get_message_index2(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid, int *available_count_out, int load_count);
DWYCOEXPORT int _real_dwyco_get_new_message_index(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid, long logical_clock);
DWYCOEXPORT void _real_dwyco_get_qd_messages(DWYCO_QD_MSG_LIST *list_out, const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_qd_message_to_body(DWYCO_SAVED_MSG_LIST *list_out, const char *pers_id, int len_pers_id);
DWYCOEXPORT int _real_dwyco_get_message_bodies(DWYCO_SAVED_MSG_LIST *list_out, const char *uid, int len_uid, int load_sent);
DWYCOEXPORT void _real_dwyco_start_bulk_update();
DWYCOEXPORT void _real_dwyco_end_bulk_update();
DWYCOEXPORT int _real_dwyco_get_sync_model(DWYCO_SYNC_MODEL *list_out);
DWYCOEXPORT int _real_dwyco_get_join_log_model(DWYCO_JOIN_LOG_MODEL *list_out);
DWYCOEXPORT int _real_dwyco_get_group_status(DWYCO_LIST *list_out);
DWYCOEXPORT int _real_dwyco_get_saved_message3(DWYCO_SAVED_MSG_LIST *list_out, const char *a_msg_id);
DWYCOEXPORT int _real_dwyco_get_saved_message(DWYCO_SAVED_MSG_LIST *list_out, const char *uid, int len_uid, const char *msg_id);
DWYCOEXPORT int _real_dwyco_get_unfetched_messages(DWYCO_UNFETCHED_MSG_LIST *list_out, const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_get_unfetched_message(DWYCO_UNFETCHED_MSG_LIST *list_out, const char *msg_id);
DWYCOEXPORT int _real_dwyco_mid_disposition(const char *mid);
DWYCOEXPORT int _real_dwyco_is_special_message2(DWYCO_UNFETCHED_MSG_LIST ml, int *what_out);
DWYCOEXPORT int _real_dwyco_get_user_payload(DWYCO_SAVED_MSG_LIST ml, const char **str_out, int *len_out);
DWYCOEXPORT int _real_dwyco_start_gj2(const char *gname, const char *password);
DWYCOEXPORT int _real_dwyco_is_special_message(const char *msg_id, int *what_out);
DWYCOEXPORT int _real_dwyco_is_delivery_report(const char *mid, const char **uid_out, int *len_uid_out, const char **dlv_mid_out, int *what_out);
DWYCOEXPORT DWYCO_LIST _real_dwyco_get_body_text(DWYCO_SAVED_MSG_LIST m);
DWYCOEXPORT DWYCO_LIST _real_dwyco_get_body_array(DWYCO_SAVED_MSG_LIST m);
DWYCOEXPORT int _real_dwyco_authenticate_body(DWYCO_SAVED_MSG_LIST m, const char *recip_uid, int len_uid, int unsaved);
DWYCOEXPORT int _real_dwyco_save_message(const char *msg_id);
DWYCOEXPORT int _real_dwyco_fetch_server_message(const char *msg_id, DwycoMessageDownloadCallback dcb, void *mdc_arg1, DwycoStatusCallback scb, void *scb_arg1);
DWYCOEXPORT void _real_dwyco_cancel_message_fetch(int fetch_id);
DWYCOEXPORT int _real_dwyco_delete_unfetched_message(const char *msg_id);
DWYCOEXPORT int _real_dwyco_delete_saved_message(const char *user_id, int len_uid, const char *msg_id);
DWYCOEXPORT void _real_dwyco_pal_add(const char *uid, int len_uid);
DWYCOEXPORT void _real_dwyco_pal_delete(const char *uid, int len_uid);
DWYCOEXPORT int _real_dwyco_is_pal(const char *uid, int len_uid);
DWYCOEXPORT DWYCO_LIST _real_dwyco_pal_get_list();
DWYCOEXPORT void _real_dwyco_set_msg_tag(const char *mid, const char *tag);
DWYCOEXPORT void _real_dwyco_unset_msg_tag(const char *mid, const char *tag);
DWYCOEXPORT void _real_dwyco_unset_all_msg_tag(const char *tag);
DWYCOEXPORT int _real_dwyco_get_tagged_mids(DWYCO_LIST *list_out, const char *tag);
DWYCOEXPORT int _real_dwyco_get_tagged_mids2(DWYCO_LIST *list_out, const char *tag);
DWYCOEXPORT int _real_dwyco_get_tagged_mids_older_than(DWYCO_LIST *list_out, const char *tag, int days);
DWYCOEXPORT int _real_dwyco_get_tagged_idx(DWYCO_MSG_IDX *list_out, const char *tag, int order_by_tag_time);
DWYCOEXPORT int _real_dwyco_mid_has_tag(const char *mid, const char *tag);
DWYCOEXPORT int _real_dwyco_uid_has_tag(const char *uid, int len_uid, const char *tag);
DWYCOEXPORT int _real_dwyco_uid_count_tag(const char *uid, int len_uid, const char *tag);
DWYCOEXPORT int _real_dwyco_count_tag(const char *tag);
DWYCOEXPORT int _real_dwyco_valid_tag_exists(const char *tag);
DWYCOEXPORT int _real_dwyco_all_messages_tagged(const char *uid, int len_uid, const char *tag);
DWYCOEXPORT void _real_dwyco_set_fav_msg(const char *mid, int fav);
DWYCOEXPORT int _real_dwyco_get_fav_msg(const char *mid);
DWYCOEXPORT int _real_dwyco_run_sql(const char *stmt, const char *a1, const char *a2, const char *a3);
DWYCOEXPORT int _real_dwyco_is_ignored(const char *uid, int len_uid);
DWYCOEXPORT void _real_dwyco_ignore(const char *uid, int len_uid);
DWYCOEXPORT void _real_dwyco_unignore(const char *uid, int len_uid);
DWYCOEXPORT DWYCO_LIST _real_dwyco_ignore_list_get();
DWYCOEXPORT void _real_dwyco_set_pals_only(int on);
DWYCOEXPORT int _real_dwyco_get_pals_only();
DWYCOEXPORT DWYCO_LIST _real_dwyco_uid_to_info(const char *uid, int len_uid, int* cant_resolve_now_out);
DWYCOEXPORT void _real_dwyco_list_release(DWYCO_LIST l);
DWYCOEXPORT int _real_dwyco_list_numelems(DWYCO_LIST l, int *rows_out, int *cols_out);
DWYCOEXPORT int _real_dwyco_list_get(DWYCO_LIST l, int row, const char *col, const char **val_out, int *len_out, int *type_out);
DWYCOEXPORT int _real_dwyco_list_print(DWYCO_LIST l);
DWYCOEXPORT DWYCO_LIST _real_dwyco_list_copy(DWYCO_LIST l);
DWYCOEXPORT DWYCO_LIST _real_dwyco_list_new();
DWYCOEXPORT void _real_dwyco_list_append(DWYCO_LIST l, const char *val, int len, int type);
DWYCOEXPORT void _real_dwyco_list_append_int(DWYCO_LIST l, int i);
DWYCOEXPORT void _real_dwyco_list_to_string(DWYCO_LIST l, const char **str_out, int *len_out);
DWYCOEXPORT int _real_dwyco_list_from_string(DWYCO_LIST *list_out, const char *str, int len_str);
DWYCOEXPORT DWYCO_LIST _real_dwyco_get_vfw_drivers();
DWYCOEXPORT int _real_dwyco_start_vfw(int idx, void *main_hwnd, void *client_hwnd);
DWYCOEXPORT int _real_dwyco_shutdown_vfw();
DWYCOEXPORT int _real_dwyco_change_driver(int new_idx);
DWYCOEXPORT int _real_dwyco_is_preview_on();
DWYCOEXPORT int _real_dwyco_preview_on(void *display_window);
DWYCOEXPORT int _real_dwyco_preview_off();
DWYCOEXPORT int _real_dwyco_vfw_format();
DWYCOEXPORT int _real_dwyco_vfw_source();
DWYCOEXPORT int _real_dwyco_set_external_video(int v);
DWYCOEXPORT void _real_dwyco_set_external_video_capture_callbacks( DwycoVVCallback nw, DwycoVVCallback del, DwycoIVICallback init, DwycoIVCallback has_data, DwycoVVCallback need, DwycoVVCallback pass, DwycoVVCallback stop, DwycoVidGetDataCallback get_data, DwycoVVCallback free_data, DwycoCACallback get_vid_devices, DwycoFCACallback free_vid_list, DwycoVICallback set_vid_device, DwycoVCallback stop_vid_device, DwycoVCallback show_source_dialog, DwycoVVCallback hw_preview_on, DwycoVCallback hw_preview_off, DwycoVVCallback set_app_data);
DWYCOEXPORT void _real_dwyco_set_external_audio_capture_callbacks( DwycoVVIICallback nw, DwycoVVCallback del, DwycoIVCallback init, DwycoIVCallback has_data, DwycoVVCallback need, DwycoVVCallback pass, DwycoVVCallback stop, DwycoVVCallback on, DwycoVVCallback off, DwycoVVCallback reset, DwycoIVCallback status, DwycoAudGetDataCallback get_data);
DWYCOEXPORT void _real_dwyco_set_external_audio_output_callbacks( DwycoVVCallback nw, DwycoVVCallback del, DwycoIVCallback init, DwycoDevOutputCallback output, DwycoDevDoneCallback done, DwycoIVCallback stop, DwycoIVCallback reset, DwycoIVCallback status, DwycoIVCallback close, DwycoIICallback buffer_time, DwycoIVCallback play_silence, DwycoIVCallback bufs_playing);
DWYCOEXPORT void _real_dwyco_force_autoupdate_check();
DWYCOEXPORT void _real_dwyco_set_autoupdate_status_callback(DwycoAutoUpdateStatusCallback sb);
DWYCOEXPORT int _real_dwyco_start_autoupdate_download(DwycoStatusCallback cb, void *arg1, DwycoAutoUpdateDownloadCallback dcb);
DWYCOEXPORT int _real_dwyco_start_autoupdate_download_bg();
DWYCOEXPORT int _real_dwyco_run_autoupdate();
DWYCOEXPORT void _real_dwyco_abort_autoupdate_download();
DWYCOEXPORT void _real_dwyco_network_diagnostics2(char **report_out, int *len_out);
DWYCOEXPORT void _real_dwyco_estimate_bandwidth2(int *out_bw_out, int *in_bw_out);
DWYCOEXPORT int _real_dwyco_create_backup(int days_to_run, int days_to_rebuild);
DWYCOEXPORT int _real_dwyco_copy_out_backup(const char *dir, int force);
DWYCOEXPORT void _real_dwyco_remove_backup();
DWYCOEXPORT int _real_dwyco_restore_from_backup(const char *bu_fn, int msgs_only);
DWYCOEXPORT int _real_dwyco_get_android_backup_state();
DWYCOEXPORT int _real_dwyco_set_android_backup_state(int i);
DWYCOEXPORT int _real_dwyco_restore_android_backup();
DWYCOEXPORT void _real_dwyco_set_aux_string(const char *str);
DWYCOEXPORT void _real_dwyco_write_token(const char *token);
DWYCOEXPORT void _real_dwyco_clear_contact_list();
DWYCOEXPORT int _real_dwyco_add_contact(const char *name, const char *phone, const char *email);
DWYCOEXPORT int _real_dwyco_get_contact_list(DWYCO_LIST *list_out);
DWYCOEXPORT int _real_dwyco_get_aux_string(const char **str_out, int *len_str_out);
DWYCOEXPORT void _real_dwyco_signal_msg_cond();
DWYCOEXPORT void _real_dwyco_wait_msg_cond(int ms);
DWYCOEXPORT int _real_dwyco_test_funny_mutex(int port);
DWYCOEXPORT int _real_dwyco_background_sync(int port, const char *sys_pfx, const char *user_pfx, const char *tmp_pfx, const char *token, const char *grpname, const char *grppw);
}
void
DWYCOCALLCONV
wrapped_cb_dwyco_set_chat_ctx_callback(int cmd, int ctx_id, const char *uid, int len_uid, const char *name, int len_name, int type, const char *value_elide, int val_len, int qid, int extra_arg)
{
printcbfunname("dwyco_set_chat_ctx_callback" "DwycoChatCtxCallback");
printarg("int ", "cmd",cmd);
printarg(" int ", "ctx_id",ctx_id);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "name",name, " int ", "len_name", len_name);
printarg(" int ", "type",type);
printarg(" int ", "val_len",val_len);
printarg(" int ", "qid",qid);
printarg(" int ", "extra_arg",extra_arg);
(*user_dwyco_set_chat_ctx_callback)(cmd,ctx_id,uid,len_uid,name,len_name,type,value_elide,val_len,qid,extra_arg);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_chat_ctx_callback2(int cmd, int ctx_id, const char *uid, int len_uid, const char *name, int len_name, DWYCO_LIST lst, int qid, int extra_arg)
{
printcbfunname("dwyco_set_chat_ctx_callback2" "DwycoChatCtxCallback2");
printarg("int ", "cmd",cmd);
printarg(" int ", "ctx_id",ctx_id);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "name",name, " int ", "len_name", len_name);
printarg(" DWYCO_LIST ", "lst",lst);
printarg(" int ", "qid",qid);
printarg(" int ", "extra_arg",extra_arg);
(*user_dwyco_set_chat_ctx_callback2)(cmd,ctx_id,uid,len_uid,name,len_name,lst,qid,extra_arg);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_debug_message_callback(int id, const char *msg, int percent_done, void *user_arg)
{
printcbfunname("dwyco_set_debug_message_callback" "DwycoStatusCallback");
printarg("int ", "id",id);
printarg(" const char *", "msg",msg);
printarg(" int ", "percent_done",percent_done);
printarg(" void *", "user_arg",user_arg);
(*user_dwyco_set_debug_message_callback)(id,msg,percent_done,user_arg);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_video_display_callback(int chan_id, void *img, int cols, int rows, int depth)
{
printcbfunname("dwyco_set_video_display_callback" "DwycoVideoDisplayCallback");
printarg("int ", "chan_id",chan_id);
printarg(" void *", "img",img);
printarg(" int ", "cols",cols);
printarg(" int ", "rows",rows);
printarg(" int ", "depth",depth);
(*user_dwyco_set_video_display_callback)(chan_id,img,cols,rows,depth);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_call_acceptance_callback(int chan_id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type)
{
printcbfunname("dwyco_set_call_acceptance_callback" "DwycoCallAppearanceCallback");
printarg("int ", "chan_id",chan_id);
printarg(" const char *", "name",name);
printarg(" const char *", "location",location);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "call_type",call_type, " int ", "len_call_type", len_call_type);
(*user_dwyco_set_call_acceptance_callback)(chan_id,name,location,uid,len_uid,call_type,len_call_type);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_emergency_callback(int whats_the_problem, int must_exit, const char *dll_msg)
{
printcbfunname("dwyco_set_emergency_callback" "DwycoEmergencyCallback");
printarg("int ", "whats_the_problem",whats_the_problem);
printarg(" int ", "must_exit",must_exit);
printarg(" const char *", "dll_msg",dll_msg);
(*user_dwyco_set_emergency_callback)(whats_the_problem,must_exit,dll_msg);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_user_control_callback(int chan_id, const char *uid, int len_uid, const char *data, int len_data)
{
printcbfunname("dwyco_set_user_control_callback" "DwycoUserControlCallback");
printarg("int ", "chan_id",chan_id);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "data",data, " int ", "len_data", len_data);
(*user_dwyco_set_user_control_callback)(chan_id,uid,len_uid,data,len_data);
/*++Reentered;
*/printcbret();
}

int
DWYCOCALLCONV
wrapped_cb_dwyco_set_call_screening_callback(int chan_id, int remote_wants_to_recv_your_video, int remote_wants_to_send_you_video, int remote_wants_to_recv_your_audio, int remote_wants_to_send_you_audio, int remote_wants_to_exchange_pubchat, int remote_wants_to_exchange_privchat, const char *call_type, int len_call_type, const char *uid, int len_uid, int *accept_call_style_out, char **error_msg_out)
{
printcbfunname("dwyco_set_call_screening_callback" "DwycoCallScreeningCallback");
printarg("int ", "chan_id",chan_id);
printarg(" int ", "remote_wants_to_recv_your_video",remote_wants_to_recv_your_video);
printarg(" int ", "remote_wants_to_send_you_video",remote_wants_to_send_you_video);
printarg(" int ", "remote_wants_to_recv_your_audio",remote_wants_to_recv_your_audio);
printarg(" int ", "remote_wants_to_send_you_audio",remote_wants_to_send_you_audio);
printarg(" int ", "remote_wants_to_exchange_pubchat",remote_wants_to_exchange_pubchat);
printarg(" int ", "remote_wants_to_exchange_privchat",remote_wants_to_exchange_privchat);
printarg(" const char *", "call_type",call_type, " int ", "len_call_type", len_call_type);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int *", "accept_call_style_out",accept_call_style_out);
printarg(" char **", "error_msg_out",error_msg_out);
int _ret = (*user_dwyco_set_call_screening_callback)(chan_id,remote_wants_to_recv_your_video,remote_wants_to_send_you_video,remote_wants_to_recv_your_audio,remote_wants_to_send_you_audio,remote_wants_to_exchange_pubchat,remote_wants_to_exchange_privchat,call_type,len_call_type,uid,len_uid,accept_call_style_out,error_msg_out);
printargout(" int *", "accept_call_style_out",accept_call_style_out);
printargout(" char **", "error_msg_out",error_msg_out);
/*++Reentered;
*/printcbretval(_ret);
return(_ret);
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_call_bandwidth_callback(int id, const char *msg, int percent_done, void *user_arg)
{
printcbfunname("dwyco_set_call_bandwidth_callback" "DwycoStatusCallback");
printarg("int ", "id",id);
printarg(" const char *", "msg",msg);
printarg(" int ", "percent_done",percent_done);
printarg(" void *", "user_arg",user_arg);
(*user_dwyco_set_call_bandwidth_callback)(id,msg,percent_done,user_arg);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_login_result_callback(const char *str, int what)
{
printcbfunname("dwyco_set_login_result_callback" "DwycoServerLoginCallback");
printarg("const char *", "str",str);
printarg(" int ", "what",what);
(*user_dwyco_set_login_result_callback)(str,what);
/*++Reentered;
*/printcbret();
}

int
DWYCOCALLCONV
wrapped_cb_dwyco_set_public_chat_init_callback(int chan_id)
{
printcbfunname("dwyco_set_public_chat_init_callback" "DwycoPublicChatInitCallback");
printarg("int ", "chan_id",chan_id);
int _ret = (*user_dwyco_set_public_chat_init_callback)(chan_id);
/*++Reentered;
*/printcbretval(_ret);
return(_ret);
}

int
DWYCOCALLCONV
wrapped_cb_dwyco_set_private_chat_init_callback(int chan_id, const char *not_used)
{
printcbfunname("dwyco_set_private_chat_init_callback" "DwycoPrivateChatInitCallback");
printarg("int ", "chan_id",chan_id);
printarg(" const char *", "not_used",not_used);
int _ret = (*user_dwyco_set_private_chat_init_callback)(chan_id,not_used);
/*++Reentered;
*/printcbretval(_ret);
return(_ret);
}

int
DWYCOCALLCONV
wrapped_cb_dwyco_set_private_chat_display_callback(int chan_id, const char *com, int arg1, int arg2, const char *str, int len)
{
printcbfunname("dwyco_set_private_chat_display_callback" "DwycoPrivateChatDisplayCallback");
printarg("int ", "chan_id",chan_id);
printarg(" const char *", "com",com);
printarg(" int ", "arg1",arg1);
printarg(" int ", "arg2",arg2);
printarg(" const char *", "str",str, " int ", "len", len);
int _ret = (*user_dwyco_set_private_chat_display_callback)(chan_id,com,arg1,arg2,str,len);
/*++Reentered;
*/printcbretval(_ret);
return(_ret);
}

int
DWYCOCALLCONV
wrapped_cb_dwyco_set_public_chat_display_callback(const char *who, int len_who, const char *line, int len_line, const char *uid, int len_uid)
{
printcbfunname("dwyco_set_public_chat_display_callback" "DwycoPublicChatDisplayCallback");
printarg("const char *", "who",who, " int ", "len_who", len_who);
printarg(" const char *", "line",line, " int ", "len_line", len_line);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = (*user_dwyco_set_public_chat_display_callback)(who,len_who,line,len_line,uid,len_uid);
/*++Reentered;
*/printcbretval(_ret);
return(_ret);
}

int
DWYCOCALLCONV
wrapped_cb_dwyco_set_bgapp_msg_callback(const char *who, int len_who, const char *line, int len_line, const char *uid, int len_uid)
{
printcbfunname("dwyco_set_bgapp_msg_callback" "DwycoPublicChatDisplayCallback");
printarg("const char *", "who",who, " int ", "len_who", len_who);
printarg(" const char *", "line",line, " int ", "len_line", len_line);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = (*user_dwyco_set_bgapp_msg_callback)(who,len_who,line,len_line,uid,len_uid);
/*++Reentered;
*/printcbretval(_ret);
return(_ret);
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_call_appearance_callback(int chan_id, const char *name, const char *location, const char *uid, int len_uid, const char *call_type, int len_call_type)
{
printcbfunname("dwyco_set_call_appearance_callback" "DwycoCallAppearanceCallback");
printarg("int ", "chan_id",chan_id);
printarg(" const char *", "name",name);
printarg(" const char *", "location",location);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "call_type",call_type, " int ", "len_call_type", len_call_type);
(*user_dwyco_set_call_appearance_callback)(chan_id,name,location,uid,len_uid,call_type,len_call_type);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_call_appearance_death_callback(int chan_id)
{
printcbfunname("dwyco_set_call_appearance_death_callback" "DwycoCallAppearanceDeathCallback");
printarg("int ", "chan_id",chan_id);
(*user_dwyco_set_call_appearance_death_callback)(chan_id);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_zap_appearance_callback(int chan_id, const char *name, int size, const char *uid, int len_uid)
{
printcbfunname("dwyco_set_zap_appearance_callback" "DwycoZapAppearanceCallback");
printarg("int ", "chan_id",chan_id);
printarg(" const char *", "name",name);
printarg(" int ", "size",size);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
(*user_dwyco_set_zap_appearance_callback)(chan_id,name,size,uid,len_uid);
/*++Reentered;
*/printcbret();
}

void
DWYCOCALLCONV
wrapped_cb_dwyco_set_autoupdate_status_callback(int status, const char *desc)
{
printcbfunname("dwyco_set_autoupdate_status_callback" "DwycoAutoUpdateStatusCallback");
printarg("int ", "status",status);
printarg(" const char *", "desc",desc);
(*user_dwyco_set_autoupdate_status_callback)(status,desc);
/*++Reentered;
*/printcbret();
}

DWYCOEXPORT
void
dwyco_field_debug(const char *var, int num)
{
printfunname("dwyco_field_debug");
printarg("const char *", "var",var);
printarg(" int ", "num",num);
_real_dwyco_field_debug(var,num);
printret();
}

DWYCOEXPORT
void
dwyco_debug_dump()
{
printfunname("dwyco_debug_dump");
_real_dwyco_debug_dump();
printret();
}

DWYCOEXPORT
void
dwyco_random_string2(char **str_out, int len)
{
printfunname("dwyco_random_string2");
printarg("char **", "str_out",str_out);
printarg(" int ", "len",len);
_real_dwyco_random_string2(str_out,len);
printargout("char **", "str_out",str_out, " int ", "len", len);
printret();
}

DWYCOEXPORT
void
dwyco_eze2(const char *str, int len_str, char **str_out, int *len_out)
{
printfunname("dwyco_eze2");
printarg("const char *", "str",str, " int ", "len_str", len_str);
printarg(" char **", "str_out",str_out);
printarg(" int *", "len_out",len_out);
_real_dwyco_eze2(str,len_str,str_out,len_out);
printargout(" char **", "str_out",str_out, " int *", "len_out", len_out);
printret();
}

DWYCOEXPORT
void
dwyco_ezd2(const char *str, int len_str, char **str_out, int *len_out)
{
printfunname("dwyco_ezd2");
printarg("const char *", "str",str, " int ", "len_str", len_str);
printarg(" char **", "str_out",str_out);
printarg(" int *", "len_out",len_out);
_real_dwyco_ezd2(str,len_str,str_out,len_out);
printargout(" char **", "str_out",str_out, " int *", "len_out", len_out);
printret();
}

DWYCOEXPORT
void
dwyco_set_fn_prefixes( const char *sys_pfx, const char *user_pfx, const char *tmp_pfx)
{
printfunname("dwyco_set_fn_prefixes");
printarg(" const char *", "sys_pfx",sys_pfx);
printarg(" const char *", "user_pfx",user_pfx);
printarg(" const char *", "tmp_pfx",tmp_pfx);
_real_dwyco_set_fn_prefixes(sys_pfx,user_pfx,tmp_pfx);
printret();
}

DWYCOEXPORT
int
dwyco_get_suspend_state()
{
printfunname("dwyco_get_suspend_state");
int _ret = _real_dwyco_get_suspend_state();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_suspend()
{
printfunname("dwyco_suspend");
_real_dwyco_suspend();
printret();
}

DWYCOEXPORT
void
dwyco_resume()
{
printfunname("dwyco_resume");
_real_dwyco_resume();
printret();
}

DWYCOEXPORT
void
dwyco_set_chat_ctx_callback(DwycoChatCtxCallback cb)
{
printfunname("dwyco_set_chat_ctx_callback");
printarg("DwycoChatCtxCallback ", "cb",(void *)cb);
user_dwyco_set_chat_ctx_callback=cb;
cb=wrapped_cb_dwyco_set_chat_ctx_callback;
_real_dwyco_set_chat_ctx_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_chat_ctx_callback2(DwycoChatCtxCallback2 cb)
{
printfunname("dwyco_set_chat_ctx_callback2");
printarg("DwycoChatCtxCallback2 ", "cb",(void *)cb);
user_dwyco_set_chat_ctx_callback2=cb;
cb=wrapped_cb_dwyco_set_chat_ctx_callback2;
_real_dwyco_set_chat_ctx_callback2(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_debug_message_callback(DwycoStatusCallback cb)
{
printfunname("dwyco_set_debug_message_callback");
printarg("DwycoStatusCallback ", "cb",(void *)cb);
user_dwyco_set_debug_message_callback=cb;
cb=wrapped_cb_dwyco_set_debug_message_callback;
_real_dwyco_set_debug_message_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_video_display_callback(DwycoVideoDisplayCallback cb)
{
printfunname("dwyco_set_video_display_callback");
printarg("DwycoVideoDisplayCallback ", "cb",(void *)cb);
user_dwyco_set_video_display_callback=cb;
cb=wrapped_cb_dwyco_set_video_display_callback;
_real_dwyco_set_video_display_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_call_acceptance_callback(DwycoCallAppearanceCallback cb)
{
printfunname("dwyco_set_call_acceptance_callback");
printarg("DwycoCallAppearanceCallback ", "cb",(void *)cb);
user_dwyco_set_call_acceptance_callback=cb;
cb=wrapped_cb_dwyco_set_call_acceptance_callback;
_real_dwyco_set_call_acceptance_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_emergency_callback(DwycoEmergencyCallback cb)
{
printfunname("dwyco_set_emergency_callback");
printarg("DwycoEmergencyCallback ", "cb",(void *)cb);
user_dwyco_set_emergency_callback=cb;
cb=wrapped_cb_dwyco_set_emergency_callback;
_real_dwyco_set_emergency_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_user_control_callback(DwycoUserControlCallback cb)
{
printfunname("dwyco_set_user_control_callback");
printarg("DwycoUserControlCallback ", "cb",(void *)cb);
user_dwyco_set_user_control_callback=cb;
cb=wrapped_cb_dwyco_set_user_control_callback;
_real_dwyco_set_user_control_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_call_screening_callback(DwycoCallScreeningCallback cb)
{
printfunname("dwyco_set_call_screening_callback");
printarg("DwycoCallScreeningCallback ", "cb",(void *)cb);
user_dwyco_set_call_screening_callback=cb;
cb=wrapped_cb_dwyco_set_call_screening_callback;
_real_dwyco_set_call_screening_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_call_bandwidth_callback(DwycoStatusCallback cb)
{
printfunname("dwyco_set_call_bandwidth_callback");
printarg("DwycoStatusCallback ", "cb",(void *)cb);
user_dwyco_set_call_bandwidth_callback=cb;
cb=wrapped_cb_dwyco_set_call_bandwidth_callback;
_real_dwyco_set_call_bandwidth_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_finish_startup()
{
printfunname("dwyco_finish_startup");
_real_dwyco_finish_startup();
printret();
}

DWYCOEXPORT
void
dwyco_set_local_auth(int a)
{
printfunname("dwyco_set_local_auth");
printarg("int ", "a",a);
_real_dwyco_set_local_auth(a);
printret();
}

DWYCOEXPORT
void
dwyco_gen_pass(const char *pw, int len_pw, char **salt_in_out, int *len_salt_in_out, char **hash_out, int *len_hash_out)
{
printfunname("dwyco_gen_pass");
printarg("const char *", "pw",pw, " int ", "len_pw", len_pw);
printarg(" char **", "salt_in_out",salt_in_out);
printarg(" int *", "len_salt_in_out",len_salt_in_out);
printarg(" char **", "hash_out",hash_out);
printarg(" int *", "len_hash_out",len_hash_out);
_real_dwyco_gen_pass(pw,len_pw,salt_in_out,len_salt_in_out,hash_out,len_hash_out);
printargout(" char **", "salt_in_out",salt_in_out, " int *", "len_salt_in_out", len_salt_in_out);
printargout(" char **", "hash_out",hash_out, " int *", "len_hash_out", len_hash_out);
printret();
}

DWYCOEXPORT
int
dwyco_get_create_new_account()
{
printfunname("dwyco_get_create_new_account");
int _ret = _real_dwyco_get_create_new_account();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_free(char *p_elide)
{
printfunname("dwyco_free");
_real_dwyco_free(p_elide);
printret();
}

DWYCOEXPORT
void
dwyco_free_array(char *p_elide)
{
printfunname("dwyco_free_array");
_real_dwyco_free_array(p_elide);
printret();
}

DWYCOEXPORT
void
dwyco_free_image(char *p_elide, int rows)
{
printfunname("dwyco_free_image");
printarg(" int ", "rows",rows);
_real_dwyco_free_image(p_elide,rows);
printret();
}

DWYCOEXPORT
void
dwyco_enable_activity_checking(int on, int timeout, DwycoActivityCallback cb)
{
printfunname("dwyco_enable_activity_checking");
printarg("int ", "on",on);
printarg(" int ", "timeout",timeout);
printarg(" DwycoActivityCallback ", "cb",(void *)cb);
_real_dwyco_enable_activity_checking(on,timeout,cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_inactivity_time(int secs)
{
printfunname("dwyco_set_inactivity_time");
printarg("int ", "secs",secs);
_real_dwyco_set_inactivity_time(secs);
printret();
}

DWYCOEXPORT
int
dwyco_chat_set_activity_state(int active, const char *state, int len_state)
{
printfunname("dwyco_chat_set_activity_state");
printarg("int ", "active",active);
printarg(" const char *", "state",state, " int ", "len_state", len_state);
int _ret = _real_dwyco_chat_set_activity_state(active,state,len_state);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_client_version(const char *str, int len_str)
{
printfunname("dwyco_set_client_version");
printarg("const char *", "str",str, " int ", "len_str", len_str);
_real_dwyco_set_client_version(str,len_str);
printret();
}

DWYCOEXPORT
int
dwyco_init()
{
printfunname("dwyco_init");
int _ret = _real_dwyco_init();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_bg_init()
{
printfunname("dwyco_bg_init");
int _ret = _real_dwyco_bg_init();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_bg_exit()
{
printfunname("dwyco_bg_exit");
int _ret = _real_dwyco_bg_exit();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_update_server_list(const char *lhxfer_str, int lhxfer_str_len)
{
printfunname("dwyco_update_server_list");
printarg("const char *", "lhxfer_str",lhxfer_str, " int ", "lhxfer_str_len", lhxfer_str_len);
int _ret = _real_dwyco_update_server_list(lhxfer_str,lhxfer_str_len);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_power_clean_safe()
{
printfunname("dwyco_power_clean_safe");
_real_dwyco_power_clean_safe();
printret();
}

DWYCOEXPORT
int
dwyco_get_authenticator(const char **a_out, int *len_a_out)
{
printfunname("dwyco_get_authenticator");
printarg("const char **", "a_out",a_out);
printarg(" int *", "len_a_out",len_a_out);
int _ret = _real_dwyco_get_authenticator(a_out,len_a_out);
printargout("const char **", "a_out",a_out, " int *", "len_a_out", len_a_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_login_result_callback(DwycoServerLoginCallback cb)
{
printfunname("dwyco_set_login_result_callback");
printarg("DwycoServerLoginCallback ", "cb",(void *)cb);
user_dwyco_set_login_result_callback=cb;
cb=wrapped_cb_dwyco_set_login_result_callback;
_real_dwyco_set_login_result_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_database_login()
{
printfunname("dwyco_database_login");
_real_dwyco_database_login();
printret();
}

DWYCOEXPORT
int
dwyco_database_online()
{
printfunname("dwyco_database_online");
int _ret = _real_dwyco_database_online();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_online()
{
printfunname("dwyco_chat_online");
int _ret = _real_dwyco_chat_online();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_database_auth_remote()
{
printfunname("dwyco_database_auth_remote");
int _ret = _real_dwyco_database_auth_remote();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_inhibit_database(int i)
{
printfunname("dwyco_inhibit_database");
printarg("int ", "i",i);
_real_dwyco_inhibit_database(i);
printret();
}

DWYCOEXPORT
void
dwyco_inhibit_pal(int i)
{
printfunname("dwyco_inhibit_pal");
printarg("int ", "i",i);
_real_dwyco_inhibit_pal(i);
printret();
}

DWYCOEXPORT
void
dwyco_inhibit_sac(int i)
{
printfunname("dwyco_inhibit_sac");
printarg("int ", "i",i);
_real_dwyco_inhibit_sac(i);
printret();
}

DWYCOEXPORT
void
dwyco_inhibit_incoming_sac(int i)
{
printfunname("dwyco_inhibit_incoming_sac");
printarg("int ", "i",i);
_real_dwyco_inhibit_incoming_sac(i);
printret();
}

DWYCOEXPORT
void
dwyco_inhibit_outgoing_sac(int i)
{
printfunname("dwyco_inhibit_outgoing_sac");
printarg("int ", "i",i);
_real_dwyco_inhibit_outgoing_sac(i);
printret();
}

DWYCOEXPORT
void
dwyco_inhibit_all_incoming(int i)
{
printfunname("dwyco_inhibit_all_incoming");
printarg("int ", "i",i);
_real_dwyco_inhibit_all_incoming(i);
printret();
}

DWYCOEXPORT
void
dwyco_set_disposition(const char *str, int len_str)
{
printfunname("dwyco_set_disposition");
printarg("const char *", "str",str, " int ", "len_str", len_str);
_real_dwyco_set_disposition(str,len_str);
printret();
}

DWYCOEXPORT
void
dwyco_fetch_info(const char *uid, int len_uid)
{
printfunname("dwyco_fetch_info");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
_real_dwyco_fetch_info(uid,len_uid);
printret();
}

DWYCOEXPORT
int
dwyco_service_channels(int *spin_out)
{
printfunname("dwyco_service_channels");
printarg("int *", "spin_out",spin_out);
int _ret = _real_dwyco_service_channels(spin_out);
printargout("int *", "spin_out",spin_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_add_entropy_timer(const char *crap, int len_crap)
{
printfunname("dwyco_add_entropy_timer");
printarg("const char *", "crap",crap, " int ", "len_crap", len_crap);
_real_dwyco_add_entropy_timer(crap,len_crap);
printret();
}

DWYCOEXPORT
void
dwyco_get_my_uid(const char **uid_out, int *len_out)
{
printfunname("dwyco_get_my_uid");
printarg("const char **", "uid_out",uid_out);
printarg(" int *", "len_out",len_out);
_real_dwyco_get_my_uid(uid_out,len_out);
printargout("const char **", "uid_out",uid_out, " int *", "len_out", len_out);
printret();
}

DWYCOEXPORT
int
dwyco_enable_video_capture_preview(int on)
{
printfunname("dwyco_enable_video_capture_preview");
printarg("int ", "on",on);
int _ret = _real_dwyco_enable_video_capture_preview(on);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_destroy_channel(int chan_id)
{
printfunname("dwyco_destroy_channel");
printarg("int ", "chan_id",chan_id);
_real_dwyco_destroy_channel(chan_id);
printret();
}

DWYCOEXPORT
void
dwyco_hangup_all_calls()
{
printfunname("dwyco_hangup_all_calls");
_real_dwyco_hangup_all_calls();
printret();
}

DWYCOEXPORT
void
dwyco_cancel_call(int call_id)
{
printfunname("dwyco_cancel_call");
printarg("int ", "call_id",call_id);
_real_dwyco_cancel_call(call_id);
printret();
}

DWYCOEXPORT
void
dwyco_pause_all_channels(int p)
{
printfunname("dwyco_pause_all_channels");
printarg("int ", "p",p);
_real_dwyco_pause_all_channels(p);
printret();
}

DWYCOEXPORT
int
dwyco_pause_channel_media_set(int chan_id, int pause_video, int pause_audio)
{
printfunname("dwyco_pause_channel_media_set");
printarg("int ", "chan_id",chan_id);
printarg(" int ", "pause_video",pause_video);
printarg(" int ", "pause_audio",pause_audio);
int _ret = _real_dwyco_pause_channel_media_set(chan_id,pause_video,pause_audio);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_pause_channel_media_get(int chan_id, int *pause_video_out, int *pause_audio_out)
{
printfunname("dwyco_pause_channel_media_get");
printarg("int ", "chan_id",chan_id);
printarg(" int *", "pause_video_out",pause_video_out);
printarg(" int *", "pause_audio_out",pause_audio_out);
int _ret = _real_dwyco_pause_channel_media_get(chan_id,pause_video_out,pause_audio_out);
printargout(" int *", "pause_video_out",pause_video_out);
printargout(" int *", "pause_audio_out",pause_audio_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_remote_pause_channel_media_get(int chan_id, int *pause_video_out, int *pause_audio_out)
{
printfunname("dwyco_remote_pause_channel_media_get");
printarg("int ", "chan_id",chan_id);
printarg(" int *", "pause_video_out",pause_video_out);
printarg(" int *", "pause_audio_out",pause_audio_out);
int _ret = _real_dwyco_remote_pause_channel_media_get(chan_id,pause_video_out,pause_audio_out);
printargout(" int *", "pause_video_out",pause_video_out);
printargout(" int *", "pause_audio_out",pause_audio_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_channel_destroy_callback(int chan_id, DwycoChannelDestroyCallback cb, void *user_arg)
{
printfunname("dwyco_set_channel_destroy_callback");
printarg("int ", "chan_id",chan_id);
printarg(" DwycoChannelDestroyCallback ", "cb",(void *)cb);
printarg(" void *", "user_arg",user_arg);
_real_dwyco_set_channel_destroy_callback(chan_id,cb,user_arg);
printret();
}

DWYCOEXPORT
int
dwyco_get_audio_hw(int *has_audio_input_out, int *has_audio_output_out, int *audio_hw_full_duplex_out)
{
printfunname("dwyco_get_audio_hw");
printarg("int *", "has_audio_input_out",has_audio_input_out);
printarg(" int *", "has_audio_output_out",has_audio_output_out);
printarg(" int *", "audio_hw_full_duplex_out",audio_hw_full_duplex_out);
int _ret = _real_dwyco_get_audio_hw(has_audio_input_out,has_audio_output_out,audio_hw_full_duplex_out);
printargout("int *", "has_audio_input_out",has_audio_input_out);
printargout(" int *", "has_audio_output_out",has_audio_output_out);
printargout(" int *", "audio_hw_full_duplex_out",audio_hw_full_duplex_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_set_all_mute(int a)
{
printfunname("dwyco_set_all_mute");
printarg("int ", "a",a);
int _ret = _real_dwyco_set_all_mute(a);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_all_mute()
{
printfunname("dwyco_get_all_mute");
int _ret = _real_dwyco_get_all_mute();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_set_exclusive_audio(int a, int recv_chan_id)
{
printfunname("dwyco_set_exclusive_audio");
printarg("int ", "a",a);
printarg(" int ", "recv_chan_id",recv_chan_id);
int _ret = _real_dwyco_set_exclusive_audio(a,recv_chan_id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_exclusive_audio(int *state_out, int *chan_id_out)
{
printfunname("dwyco_get_exclusive_audio");
printarg("int *", "state_out",state_out);
printarg(" int *", "chan_id_out",chan_id_out);
int _ret = _real_dwyco_get_exclusive_audio(state_out,chan_id_out);
printargout("int *", "state_out",state_out);
printargout(" int *", "chan_id_out",chan_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_set_auto_squelch(int a)
{
printfunname("dwyco_set_auto_squelch");
printarg("int ", "a",a);
int _ret = _real_dwyco_set_auto_squelch(a);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_auto_squelch()
{
printfunname("dwyco_get_auto_squelch");
int _ret = _real_dwyco_get_auto_squelch();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_squelched()
{
printfunname("dwyco_get_squelched");
int _ret = _real_dwyco_get_squelched();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_full_duplex(int a)
{
printfunname("dwyco_set_full_duplex");
printarg("int ", "a",a);
_real_dwyco_set_full_duplex(a);
printret();
}

DWYCOEXPORT
int
dwyco_get_full_duplex()
{
printfunname("dwyco_get_full_duplex");
int _ret = _real_dwyco_get_full_duplex();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_audio_output_in_progress()
{
printfunname("dwyco_get_audio_output_in_progress");
int _ret = _real_dwyco_get_audio_output_in_progress();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_addq(int q)
{
printfunname("dwyco_chat_addq");
printarg("int ", "q",q);
int _ret = _real_dwyco_chat_addq(q);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_delq(int q, const char *uid, int len_uid)
{
printfunname("dwyco_chat_delq");
printarg("int ", "q",q);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_chat_delq(q,uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_talk(int q)
{
printfunname("dwyco_chat_talk");
printarg("int ", "q",q);
int _ret = _real_dwyco_chat_talk(q);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_mute(int q, int on)
{
printfunname("dwyco_chat_mute");
printarg("int ", "q",q);
printarg(" int ", "on",on);
int _ret = _real_dwyco_chat_mute(q,on);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_set_filter(int q, int gods_only,  int demigods_only, const char *uid, int len_uid)
{
printfunname("dwyco_chat_set_filter");
printarg("int ", "q",q);
printarg(" int ", "gods_only",gods_only);
printarg("  int ", "demigods_only",demigods_only);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_chat_set_filter(q,gods_only,demigods_only,uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_set_demigod(const char *uid, int len_uid, int on)
{
printfunname("dwyco_chat_set_demigod");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int ", "on",on);
int _ret = _real_dwyco_chat_set_demigod(uid,len_uid,on);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_clear_all_demigods()
{
printfunname("dwyco_chat_clear_all_demigods");
int _ret = _real_dwyco_chat_clear_all_demigods();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_set_unblock_time2(int q, const char *uid, int len_uid, int tm, const char *reason)
{
printfunname("dwyco_chat_set_unblock_time2");
printarg("int ", "q",q);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int ", "tm",tm);
printarg(" const char *", "reason",reason);
int _ret = _real_dwyco_chat_set_unblock_time2(q,uid,len_uid,tm,reason);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_set_unblock_time(int q, const char *uid, int len_uid, int tm)
{
printfunname("dwyco_chat_set_unblock_time");
printarg("int ", "q",q);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int ", "tm",tm);
int _ret = _real_dwyco_chat_set_unblock_time(q,uid,len_uid,tm);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_get_admin_info()
{
printfunname("dwyco_chat_get_admin_info");
int _ret = _real_dwyco_chat_get_admin_info();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_send_popup(const char *text, int len_text, int global)
{
printfunname("dwyco_chat_send_popup");
printarg("const char *", "text",text, " int ", "len_text", len_text);
printarg(" int ", "global",global);
int _ret = _real_dwyco_chat_send_popup(text,len_text,global);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_set_sys_attr(const char *name, int name_len, int dwyco_type, const char *val, int val_len, int int_val)
{
printfunname("dwyco_chat_set_sys_attr");
printarg("const char *", "name",name, " int ", "name_len", name_len);
printarg(" int ", "dwyco_type",dwyco_type);
printarg(" const char *", "val",val, " int ", "val_len", val_len);
printarg(" int ", "int_val",int_val);
int _ret = _real_dwyco_chat_set_sys_attr(name,name_len,dwyco_type,val,val_len,int_val);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_chat_create_user_lobby(const char *dispname,  const char *category, const char *sub_god_uid, int len_sub_god_uid, const char *pw, int user_limit, DwycoCommandCallback cb, void *user_arg)
{
printfunname("dwyco_chat_create_user_lobby");
printarg("const char *", "dispname",dispname);
printarg("  const char *", "category",category);
printarg(" const char *", "sub_god_uid",sub_god_uid, " int ", "len_sub_god_uid", len_sub_god_uid);
printarg(" const char *", "pw",pw);
printarg(" int ", "user_limit",user_limit);
printarg(" DwycoCommandCallback ", "cb",(void *)cb);
printarg(" void *", "user_arg",user_arg);
_real_dwyco_chat_create_user_lobby(dispname,category,sub_god_uid,len_sub_god_uid,pw,user_limit,cb,user_arg);
printret();
}

DWYCOEXPORT
void
dwyco_chat_remove_user_lobby(const char *lobby_id, DwycoCommandCallback cb, void *user_arg)
{
printfunname("dwyco_chat_remove_user_lobby");
printarg("const char *", "lobby_id",lobby_id);
printarg(" DwycoCommandCallback ", "cb",(void *)cb);
printarg(" void *", "user_arg",user_arg);
_real_dwyco_chat_remove_user_lobby(lobby_id,cb,user_arg);
printret();
}

DWYCOEXPORT
void
dwyco_chat_send_data(const char *txt, int txt_len, int pic_type, const char *pic_data, int pic_data_len)
{
printfunname("dwyco_chat_send_data");
printarg("const char *", "txt",txt, " int ", "txt_len", txt_len);
printarg(" int ", "pic_type",pic_type);
printarg(" const char *", "pic_data",pic_data, " int ", "pic_data_len", pic_data_len);
_real_dwyco_chat_send_data(txt,txt_len,pic_type,pic_data,pic_data_len);
printret();
}

DWYCOEXPORT
void
dwyco_set_moron_dork_mode(int m)
{
printfunname("dwyco_set_moron_dork_mode");
printarg("int ", "m",m);
_real_dwyco_set_moron_dork_mode(m);
printret();
}

DWYCOEXPORT
int
dwyco_get_moron_dork_mode()
{
printfunname("dwyco_get_moron_dork_mode");
int _ret = _real_dwyco_get_moron_dork_mode();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_public_chat_init_callback(DwycoPublicChatInitCallback cb)
{
printfunname("dwyco_set_public_chat_init_callback");
printarg("DwycoPublicChatInitCallback ", "cb",(void *)cb);
user_dwyco_set_public_chat_init_callback=cb;
cb=wrapped_cb_dwyco_set_public_chat_init_callback;
_real_dwyco_set_public_chat_init_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_private_chat_init_callback(DwycoPrivateChatInitCallback cb)
{
printfunname("dwyco_set_private_chat_init_callback");
printarg("DwycoPrivateChatInitCallback ", "cb",(void *)cb);
user_dwyco_set_private_chat_init_callback=cb;
cb=wrapped_cb_dwyco_set_private_chat_init_callback;
_real_dwyco_set_private_chat_init_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_private_chat_display_callback(DwycoPrivateChatDisplayCallback cb)
{
printfunname("dwyco_set_private_chat_display_callback");
printarg("DwycoPrivateChatDisplayCallback ", "cb",(void *)cb);
user_dwyco_set_private_chat_display_callback=cb;
cb=wrapped_cb_dwyco_set_private_chat_display_callback;
_real_dwyco_set_private_chat_display_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_public_chat_display_callback(DwycoPublicChatDisplayCallback cb)
{
printfunname("dwyco_set_public_chat_display_callback");
printarg("DwycoPublicChatDisplayCallback ", "cb",(void *)cb);
user_dwyco_set_public_chat_display_callback=cb;
cb=wrapped_cb_dwyco_set_public_chat_display_callback;
_real_dwyco_set_public_chat_display_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_bgapp_msg_callback(DwycoPublicChatDisplayCallback cb)
{
printfunname("dwyco_set_bgapp_msg_callback");
printarg("DwycoPublicChatDisplayCallback ", "cb",(void *)cb);
user_dwyco_set_bgapp_msg_callback=cb;
cb=wrapped_cb_dwyco_set_bgapp_msg_callback;
_real_dwyco_set_bgapp_msg_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_command_from_keyboard(int chan_id, int com, int arg1, int arg2, const char *str, int len)
{
printfunname("dwyco_command_from_keyboard");
printarg("int ", "chan_id",chan_id);
printarg(" int ", "com",com);
printarg(" int ", "arg1",arg1);
printarg(" int ", "arg2",arg2);
printarg(" const char *", "str",str, " int ", "len", len);
_real_dwyco_command_from_keyboard(chan_id,com,arg1,arg2,str,len);
printret();
}

DWYCOEXPORT
void
dwyco_line_from_keyboard(int id, const char *line, int len)
{
printfunname("dwyco_line_from_keyboard");
printarg("int ", "id",id);
printarg(" const char *", "line",line, " int ", "len", len);
_real_dwyco_line_from_keyboard(id,line,len);
printret();
}

DWYCOEXPORT
int
dwyco_selective_chat_recipient_enable(const char *uid, int len_uid, int enable)
{
printfunname("dwyco_selective_chat_recipient_enable");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int ", "enable",enable);
int _ret = _real_dwyco_selective_chat_recipient_enable(uid,len_uid,enable);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_selective_chat_enable(int e)
{
printfunname("dwyco_selective_chat_enable");
printarg("int ", "e",e);
int _ret = _real_dwyco_selective_chat_enable(e);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_reset_selective_chat_recipients()
{
printfunname("dwyco_reset_selective_chat_recipients");
_real_dwyco_reset_selective_chat_recipients();
printret();
}

DWYCOEXPORT
int
dwyco_is_selective_chat_recipient(const char *uid, int len_uid)
{
printfunname("dwyco_is_selective_chat_recipient");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_is_selective_chat_recipient(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_set_max_established_originated_calls(int n)
{
printfunname("dwyco_set_max_established_originated_calls");
printarg("int ", "n",n);
int _ret = _real_dwyco_set_max_established_originated_calls(n);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_channel_create(const char *uid, int len_uid, DwycoCallDispositionCallback cdc, void *cdc_arg1, DwycoStatusCallback scb, void *scb_arg1, const char *pw, const char *call_type, int len_call_type, int q_call)
{
printfunname("dwyco_channel_create");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" DwycoCallDispositionCallback ", "cdc",(void *)cdc);
printarg(" void *", "cdc_arg1",cdc_arg1);
printarg(" DwycoStatusCallback ", "scb",(void *)scb);
printarg(" void *", "scb_arg1",scb_arg1);
printarg(" const char *", "pw",pw);
printarg(" const char *", "call_type",call_type, " int ", "len_call_type", len_call_type);
printarg(" int ", "q_call",q_call);
int _ret = _real_dwyco_channel_create(uid,len_uid,cdc,cdc_arg1,scb,scb_arg1,pw,call_type,len_call_type,q_call);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_channel_send_video(int chan_id, int vid_dev)
{
printfunname("dwyco_channel_send_video");
printarg("int ", "chan_id",chan_id);
printarg(" int ", "vid_dev",vid_dev);
int _ret = _real_dwyco_channel_send_video(chan_id,vid_dev);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_channel_stop_send_video(int chan_id)
{
printfunname("dwyco_channel_stop_send_video");
printarg("int ", "chan_id",chan_id);
int _ret = _real_dwyco_channel_stop_send_video(chan_id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_channel_send_audio(int chan_id, int aud_dev)
{
printfunname("dwyco_channel_send_audio");
printarg("int ", "chan_id",chan_id);
printarg(" int ", "aud_dev",aud_dev);
int _ret = _real_dwyco_channel_send_audio(chan_id,aud_dev);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_channel_stop_send_audio(int chan_id)
{
printfunname("dwyco_channel_stop_send_audio");
printarg("int ", "chan_id",chan_id);
int _ret = _real_dwyco_channel_stop_send_audio(chan_id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_connect_uid(const char *uid, int len_uid, DwycoCallDispositionCallback cdc, void *cdc_arg1, DwycoStatusCallback scb, void *scb_arg1, int send_video, int recv_video, int send_audio, int recv_audio, int private_chat, int public_chat, const char *pw, int len_pw, const char *call_type, int len_call_type, int q_call)
{
printfunname("dwyco_connect_uid");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" DwycoCallDispositionCallback ", "cdc",(void *)cdc);
printarg(" void *", "cdc_arg1",cdc_arg1);
printarg(" DwycoStatusCallback ", "scb",(void *)scb);
printarg(" void *", "scb_arg1",scb_arg1);
printarg(" int ", "send_video",send_video);
printarg(" int ", "recv_video",recv_video);
printarg(" int ", "send_audio",send_audio);
printarg(" int ", "recv_audio",recv_audio);
printarg(" int ", "private_chat",private_chat);
printarg(" int ", "public_chat",public_chat);
printarg(" const char *", "pw",pw, " int ", "len_pw", len_pw);
printarg(" const char *", "call_type",call_type, " int ", "len_call_type", len_call_type);
printarg(" int ", "q_call",q_call);
int _ret = _real_dwyco_connect_uid(uid,len_uid,cdc,cdc_arg1,scb,scb_arg1,send_video,recv_video,send_audio,recv_audio,private_chat,public_chat,pw,len_pw,call_type,len_call_type,q_call);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chan_to_call(int chan_id)
{
printfunname("dwyco_chan_to_call");
printarg("int ", "chan_id",chan_id);
int _ret = _real_dwyco_chan_to_call(chan_id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_channel_streams(int chan_id, int *send_video_out, int *recv_video_out, int *send_audio_out, int *recv_audio_out, int *pubchat_out, int *privchat_out)
{
printfunname("dwyco_channel_streams");
printarg("int ", "chan_id",chan_id);
printarg(" int *", "send_video_out",send_video_out);
printarg(" int *", "recv_video_out",recv_video_out);
printarg(" int *", "send_audio_out",send_audio_out);
printarg(" int *", "recv_audio_out",recv_audio_out);
printarg(" int *", "pubchat_out",pubchat_out);
printarg(" int *", "privchat_out",privchat_out);
int _ret = _real_dwyco_channel_streams(chan_id,send_video_out,recv_video_out,send_audio_out,recv_audio_out,pubchat_out,privchat_out);
printargout(" int *", "send_video_out",send_video_out);
printargout(" int *", "recv_video_out",recv_video_out);
printargout(" int *", "send_audio_out",send_audio_out);
printargout(" int *", "recv_audio_out",recv_audio_out);
printargout(" int *", "pubchat_out",pubchat_out);
printargout(" int *", "privchat_out",privchat_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_connect_msg_chan(const char *uid, int len_uid, DwycoCallDispositionCallback cdc, void *cdc_arg1, DwycoStatusCallback scb, void *scb_arg1)
{
printfunname("dwyco_connect_msg_chan");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" DwycoCallDispositionCallback ", "cdc",(void *)cdc);
printarg(" void *", "cdc_arg1",cdc_arg1);
printarg(" DwycoStatusCallback ", "scb",(void *)scb);
printarg(" void *", "scb_arg1",scb_arg1);
int _ret = _real_dwyco_connect_msg_chan(uid,len_uid,cdc,cdc_arg1,scb,scb_arg1);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_send_user_control(const char *uid, int len_uid, const char *data, int len_data)
{
printfunname("dwyco_send_user_control");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "data",data, " int ", "len_data", len_data);
int _ret = _real_dwyco_send_user_control(uid,len_uid,data,len_data);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_call_appearance_callback(DwycoCallAppearanceCallback cb)
{
printfunname("dwyco_set_call_appearance_callback");
printarg("DwycoCallAppearanceCallback ", "cb",(void *)cb);
user_dwyco_set_call_appearance_callback=cb;
cb=wrapped_cb_dwyco_set_call_appearance_callback;
_real_dwyco_set_call_appearance_callback(cb);
printret();
}

DWYCOEXPORT
void
dwyco_set_call_appearance_death_callback(DwycoCallAppearanceDeathCallback cb)
{
printfunname("dwyco_set_call_appearance_death_callback");
printarg("DwycoCallAppearanceDeathCallback ", "cb",(void *)cb);
user_dwyco_set_call_appearance_death_callback=cb;
cb=wrapped_cb_dwyco_set_call_appearance_death_callback;
_real_dwyco_set_call_appearance_death_callback(cb);
printret();
}

DWYCOEXPORT
int
dwyco_call_accept(int id)
{
printfunname("dwyco_call_accept");
printarg("int ", "id",id);
int _ret = _real_dwyco_call_accept(id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_call_reject(int id, int session_ignore)
{
printfunname("dwyco_call_reject");
printarg("int ", "id",id);
printarg(" int ", "session_ignore",session_ignore);
int _ret = _real_dwyco_call_reject(id,session_ignore);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_zap_appearance_callback(DwycoZapAppearanceCallback cb)
{
printfunname("dwyco_set_zap_appearance_callback");
printarg("DwycoZapAppearanceCallback ", "cb",(void *)cb);
user_dwyco_set_zap_appearance_callback=cb;
cb=wrapped_cb_dwyco_set_zap_appearance_callback;
_real_dwyco_set_zap_appearance_callback(cb);
printret();
}

DWYCOEXPORT
int
dwyco_zap_accept(int id, int always_accept)
{
printfunname("dwyco_zap_accept");
printarg("int ", "id",id);
printarg(" int ", "always_accept",always_accept);
int _ret = _real_dwyco_zap_accept(id,always_accept);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_reject(int id, int session_ignore)
{
printfunname("dwyco_zap_reject");
printarg("int ", "id",id);
printarg(" int ", "session_ignore",session_ignore);
int _ret = _real_dwyco_zap_reject(id,session_ignore);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_server_list(DWYCO_SERVER_LIST *list_out, int *numlines_out)
{
printfunname("dwyco_get_server_list");
printarg("DWYCO_SERVER_LIST *", "list_out",list_out);
printarg(" int *", "numlines_out",numlines_out);
int _ret = _real_dwyco_get_server_list(list_out,numlines_out);
printargout("DWYCO_SERVER_LIST *", "list_out",list_out);
printargout(" int *", "numlines_out",numlines_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_lobby_name_by_id2(const char *id, DWYCO_LIST *list_out)
{
printfunname("dwyco_get_lobby_name_by_id2");
printarg("const char *", "id",id);
printarg(" DWYCO_LIST *", "list_out",list_out);
int _ret = _real_dwyco_get_lobby_name_by_id2(id,list_out);
printargout(" DWYCO_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_switch_to_chat_server(int i)
{
printfunname("dwyco_switch_to_chat_server");
printarg("int ", "i",i);
int _ret = _real_dwyco_switch_to_chat_server(i);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_check_chat_server_pw(const char *cid, const char *pw)
{
printfunname("dwyco_check_chat_server_pw");
printarg("const char *", "cid",cid);
printarg(" const char *", "pw",pw);
int _ret = _real_dwyco_check_chat_server_pw(cid,pw);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_chat_server_has_pw(const char *cid)
{
printfunname("dwyco_chat_server_has_pw");
printarg("const char *", "cid",cid);
int _ret = _real_dwyco_chat_server_has_pw(cid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_switch_to_chat_server2(const char *cid, const char *pw)
{
printfunname("dwyco_switch_to_chat_server2");
printarg("const char *", "cid",cid);
printarg(" const char *", "pw",pw);
int _ret = _real_dwyco_switch_to_chat_server2(cid,pw);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_disconnect_chat_server()
{
printfunname("dwyco_disconnect_chat_server");
int _ret = _real_dwyco_disconnect_chat_server();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_initial_invis(int invis)
{
printfunname("dwyco_set_initial_invis");
printarg("int ", "invis",invis);
_real_dwyco_set_initial_invis(invis);
printret();
}

DWYCOEXPORT
void
dwyco_set_invisible_state(int invis)
{
printfunname("dwyco_set_invisible_state");
printarg("int ", "invis",invis);
_real_dwyco_set_invisible_state(invis);
printret();
}

DWYCOEXPORT
int
dwyco_get_invisible_state()
{
printfunname("dwyco_get_invisible_state");
int _ret = _real_dwyco_get_invisible_state();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_delete_user(const char *uid, int len_uid)
{
printfunname("dwyco_delete_user");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_delete_user(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_clear_user(const char *uid, int len_uid)
{
printfunname("dwyco_clear_user");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_clear_user(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_clear_user_unfav(const char *uid, int len_uid)
{
printfunname("dwyco_clear_user_unfav");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_clear_user_unfav(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_pal_relogin()
{
printfunname("dwyco_pal_relogin");
_real_dwyco_pal_relogin();
printret();
}

DWYCOEXPORT
int
dwyco_get_pal_logged_in()
{
printfunname("dwyco_get_pal_logged_in");
int _ret = _real_dwyco_get_pal_logged_in();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_empty_trash()
{
printfunname("dwyco_empty_trash");
int _ret = _real_dwyco_empty_trash();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_count_trashed_users()
{
printfunname("dwyco_count_trashed_users");
int _ret = _real_dwyco_count_trashed_users();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_untrash_users()
{
printfunname("dwyco_untrash_users");
_real_dwyco_untrash_users();
printret();
}

DWYCOEXPORT
int
dwyco_set_profile_from_composer(int compid, const char *txt, int txt_len, DwycoProfileCallback cb, void *arg)
{
printfunname("dwyco_set_profile_from_composer");
printarg("int ", "compid",compid);
printarg(" const char *", "txt",txt, " int ", "txt_len", txt_len);
printarg(" DwycoProfileCallback ", "cb",(void *)cb);
printarg(" void *", "arg",arg);
int _ret = _real_dwyco_set_profile_from_composer(compid,txt,txt_len,cb,arg);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_profile_to_viewer(const char *uid, int len_uid, DwycoProfileCallback cb, void *arg)
{
printfunname("dwyco_get_profile_to_viewer");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" DwycoProfileCallback ", "cb",(void *)cb);
printarg(" void *", "arg",arg);
int _ret = _real_dwyco_get_profile_to_viewer(uid,len_uid,cb,arg);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_profile_to_viewer_sync(const char *uid, int len_uid, char **fn_out, int *len_fn_out)
{
printfunname("dwyco_get_profile_to_viewer_sync");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" char **", "fn_out",fn_out);
printarg(" int *", "len_fn_out",len_fn_out);
int _ret = _real_dwyco_get_profile_to_viewer_sync(uid,len_uid,fn_out,len_fn_out);
printargout(" char **", "fn_out",fn_out, " int *", "len_fn_out", len_fn_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_name_to_uid(const char *handle, int len_handle)
{
printfunname("dwyco_name_to_uid");
printarg("const char *", "handle",handle, " int ", "len_handle", len_handle);
_real_dwyco_name_to_uid(handle,len_handle);
printret();
}

DWYCOEXPORT
int
dwyco_map_uid_to_representative(const char *uid, int len_uid, DWYCO_LIST *list_out)
{
printfunname("dwyco_map_uid_to_representative");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" DWYCO_LIST *", "list_out",list_out);
int _ret = _real_dwyco_map_uid_to_representative(uid,len_uid,list_out);
printargout(" DWYCO_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_create_bootstrap_profile(const char *handle, int len_handle, const char *desc, int len_desc, const char *loc, int len_loc, const char *email, int len_email)
{
printfunname("dwyco_create_bootstrap_profile");
printarg("const char *", "handle",handle, " int ", "len_handle", len_handle);
printarg(" const char *", "desc",desc, " int ", "len_desc", len_desc);
printarg(" const char *", "loc",loc, " int ", "len_loc", len_loc);
printarg(" const char *", "email",email, " int ", "len_email", len_email);
int _ret = _real_dwyco_create_bootstrap_profile(handle,len_handle,desc,len_desc,loc,len_loc,email,len_email);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_profile_pack(const char *handle, int len_handle, const char *desc, int len_desc, const char *loc, int len_loc, const char *email, int len_email, const char **str_out, int *len_str_out)
{
printfunname("dwyco_make_profile_pack");
printarg("const char *", "handle",handle, " int ", "len_handle", len_handle);
printarg(" const char *", "desc",desc, " int ", "len_desc", len_desc);
printarg(" const char *", "loc",loc, " int ", "len_loc", len_loc);
printarg(" const char *", "email",email, " int ", "len_email", len_email);
printarg(" const char **", "str_out",str_out);
printarg(" int *", "len_str_out",len_str_out);
int _ret = _real_dwyco_make_profile_pack(handle,len_handle,desc,len_desc,loc,len_loc,email,len_email,str_out,len_str_out);
printargout(" const char **", "str_out",str_out, " int *", "len_str_out", len_str_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_set_setting(const char *name, const char *value)
{
printfunname("dwyco_set_setting");
printarg("const char *", "name",name);
printarg(" const char *", "value",value);
int _ret = _real_dwyco_set_setting(name,value);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_setting(const char *name, const char **value_out, int *len_out, int *dwyco_type_out)
{
printfunname("dwyco_get_setting");
printarg("const char *", "name",name);
printarg(" const char **", "value_out",value_out);
printarg(" int *", "len_out",len_out);
printarg(" int *", "dwyco_type_out",dwyco_type_out);
int _ret = _real_dwyco_get_setting(name,value_out,len_out,dwyco_type_out);
printargout(" const char **", "value_out",value_out, " int *", "len_out", len_out);
printargout(" int *", "dwyco_type_out",dwyco_type_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_zap_composition( char *dum)
{
printfunname("dwyco_make_zap_composition");
printarg(" char *", "dum",dum);
int _ret = _real_dwyco_make_zap_composition(dum);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_zap_composition_raw(const char *filename, const char *possible_extension)
{
printfunname("dwyco_make_zap_composition_raw");
printarg("const char *", "filename",filename);
printarg(" const char *", "possible_extension",possible_extension);
int _ret = _real_dwyco_make_zap_composition_raw(filename,possible_extension);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_dup_zap_composition(int compid)
{
printfunname("dwyco_dup_zap_composition");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_dup_zap_composition(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_forward_zap_composition2(const char *msg_id, int strip_forward_text)
{
printfunname("dwyco_make_forward_zap_composition2");
printarg("const char *", "msg_id",msg_id);
printarg(" int ", "strip_forward_text",strip_forward_text);
int _ret = _real_dwyco_make_forward_zap_composition2(msg_id,strip_forward_text);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_is_forward_composition(int compid)
{
printfunname("dwyco_is_forward_composition");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_is_forward_composition(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_flim(int compid)
{
printfunname("dwyco_flim");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_flim(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_is_file_zap(int compid)
{
printfunname("dwyco_is_file_zap");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_is_file_zap(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_special_zap_composition( int special_type, const char *user_block, int len_user_block)
{
printfunname("dwyco_make_special_zap_composition");
printarg(" int ", "special_type",special_type);
printarg(" const char *", "user_block",user_block, " int ", "len_user_block", len_user_block);
int _ret = _real_dwyco_make_special_zap_composition(special_type,user_block,len_user_block);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_set_special_zap(int compid, int special_type)
{
printfunname("dwyco_set_special_zap");
printarg("int ", "compid",compid);
printarg(" int ", "special_type",special_type);
int _ret = _real_dwyco_set_special_zap(compid,special_type);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_file_zap_composition( const char *filename, int len_filename)
{
printfunname("dwyco_make_file_zap_composition");
printarg(" const char *", "filename",filename, " int ", "len_filename", len_filename);
int _ret = _real_dwyco_make_file_zap_composition(filename,len_filename);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_copy_out_qd_file_zap(DWYCO_SAVED_MSG_LIST m, const char *dst_filename)
{
printfunname("dwyco_copy_out_qd_file_zap");
printarg("DWYCO_SAVED_MSG_LIST ", "m",m);
printarg(" const char *", "dst_filename",dst_filename);
int _ret = _real_dwyco_copy_out_qd_file_zap(m,dst_filename);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_copy_out_file_zap2(const char *msg_id, const char *dst_filename)
{
printfunname("dwyco_copy_out_file_zap2");
printarg("const char *", "msg_id",msg_id);
printarg(" const char *", "dst_filename",dst_filename);
int _ret = _real_dwyco_copy_out_file_zap2(msg_id,dst_filename);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_copy_out_file_zap_buf2(const char *msg_id, const char **buf_out, int *buf_len_out, int max)
{
printfunname("dwyco_copy_out_file_zap_buf2");
printarg("const char *", "msg_id",msg_id);
printarg(" const char **", "buf_out",buf_out);
printarg(" int *", "buf_len_out",buf_len_out);
printarg(" int ", "max",max);
int _ret = _real_dwyco_copy_out_file_zap_buf2(msg_id,buf_out,buf_len_out,max);
printargout(" const char **", "buf_out",buf_out, " int *", "buf_len_out", buf_len_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_delete_zap_composition(int compid)
{
printfunname("dwyco_delete_zap_composition");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_delete_zap_composition(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_record2(int compid, int video, int audio, int max_frames, int max_bytes, int hi_quality, DwycoStatusCallback scb, void *scb_arg1, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *chan_id_out)
{
printfunname("dwyco_zap_record2");
printarg("int ", "compid",compid);
printarg(" int ", "video",video);
printarg(" int ", "audio",audio);
printarg(" int ", "max_frames",max_frames);
printarg(" int ", "max_bytes",max_bytes);
printarg(" int ", "hi_quality",hi_quality);
printarg(" DwycoStatusCallback ", "scb",(void *)scb);
printarg(" void *", "scb_arg1",scb_arg1);
printarg(" DwycoChannelDestroyCallback ", "dcb",(void *)dcb);
printarg(" void *", "dcb_arg1",dcb_arg1);
printarg(" int *", "chan_id_out",chan_id_out);
int _ret = _real_dwyco_zap_record2(compid,video,audio,max_frames,max_bytes,hi_quality,scb,scb_arg1,dcb,dcb_arg1,chan_id_out);
printargout(" int *", "chan_id_out",chan_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_record(int compid, int video, int audio, int pic, int frames, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out)
{
printfunname("dwyco_zap_record");
printarg("int ", "compid",compid);
printarg(" int ", "video",video);
printarg(" int ", "audio",audio);
printarg(" int ", "pic",pic);
printarg(" int ", "frames",frames);
printarg(" DwycoChannelDestroyCallback ", "dcb",(void *)dcb);
printarg(" void *", "dcb_arg1",dcb_arg1);
printarg(" int *", "ui_id_out",ui_id_out);
int _ret = _real_dwyco_zap_record(compid,video,audio,pic,frames,dcb,dcb_arg1,ui_id_out);
printargout(" int *", "ui_id_out",ui_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_composition_chan_id(int compid)
{
printfunname("dwyco_zap_composition_chan_id");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_zap_composition_chan_id(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_stop(int compid)
{
printfunname("dwyco_zap_stop");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_zap_stop(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_play(int compid, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out)
{
printfunname("dwyco_zap_play");
printarg("int ", "compid",compid);
printarg(" DwycoChannelDestroyCallback ", "dcb",(void *)dcb);
printarg(" void *", "dcb_arg1",dcb_arg1);
printarg(" int *", "ui_id_out",ui_id_out);
int _ret = _real_dwyco_zap_play(compid,dcb,dcb_arg1,ui_id_out);
printargout(" int *", "ui_id_out",ui_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_send4(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, const char **pers_id_out, int *len_pers_id_out)
{
printfunname("dwyco_zap_send4");
printarg("int ", "compid",compid);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "text",text, " int ", "len_text", len_text);
printarg(" int ", "no_forward",no_forward);
printarg(" const char **", "pers_id_out",pers_id_out);
printarg(" int *", "len_pers_id_out",len_pers_id_out);
int _ret = _real_dwyco_zap_send4(compid,uid,len_uid,text,len_text,no_forward,pers_id_out,len_pers_id_out);
printargout(" const char **", "pers_id_out",pers_id_out, " int *", "len_pers_id_out", len_pers_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_send5(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, int save_sent, const char **pers_id_out, int *len_pers_id_out)
{
printfunname("dwyco_zap_send5");
printarg("int ", "compid",compid);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "text",text, " int ", "len_text", len_text);
printarg(" int ", "no_forward",no_forward);
printarg(" int ", "save_sent",save_sent);
printarg(" const char **", "pers_id_out",pers_id_out);
printarg(" int *", "len_pers_id_out",len_pers_id_out);
int _ret = _real_dwyco_zap_send5(compid,uid,len_uid,text,len_text,no_forward,save_sent,pers_id_out,len_pers_id_out);
printargout(" const char **", "pers_id_out",pers_id_out, " int *", "len_pers_id_out", len_pers_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_send6(int compid, const char *uid, int len_uid, const char *text, int len_text, int no_forward, int save_sent, int defer, const char **pers_id_out, int *len_pers_id_out)
{
printfunname("dwyco_zap_send6");
printarg("int ", "compid",compid);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "text",text, " int ", "len_text", len_text);
printarg(" int ", "no_forward",no_forward);
printarg(" int ", "save_sent",save_sent);
printarg(" int ", "defer",defer);
printarg(" const char **", "pers_id_out",pers_id_out);
printarg(" int *", "len_pers_id_out",len_pers_id_out);
int _ret = _real_dwyco_zap_send6(compid,uid,len_uid,text,len_text,no_forward,save_sent,defer,pers_id_out,len_pers_id_out);
printargout(" const char **", "pers_id_out",pers_id_out, " int *", "len_pers_id_out", len_pers_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_cancel(int compid)
{
printfunname("dwyco_zap_cancel");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_zap_cancel(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_still_active(int compid)
{
printfunname("dwyco_zap_still_active");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_zap_still_active(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_kill_message(const char *pers_id, int len_pers_id)
{
printfunname("dwyco_kill_message");
printarg("const char *", "pers_id",pers_id, " int ", "len_pers_id", len_pers_id);
int _ret = _real_dwyco_kill_message(pers_id,len_pers_id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_zap_view2(DWYCO_SAVED_MSG_LIST list, int qd)
{
printfunname("dwyco_make_zap_view2");
printarg("DWYCO_SAVED_MSG_LIST ", "list",list);
printarg(" int ", "qd",qd);
int _ret = _real_dwyco_make_zap_view2(list,qd);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_zap_view_file(const char *filename)
{
printfunname("dwyco_make_zap_view_file");
printarg("const char *", "filename",filename);
int _ret = _real_dwyco_make_zap_view_file(filename);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_make_zap_view_file_raw(const char *filename)
{
printfunname("dwyco_make_zap_view_file_raw");
printarg("const char *", "filename",filename);
int _ret = _real_dwyco_make_zap_view_file_raw(filename);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_delete_zap_view(int compid)
{
printfunname("dwyco_delete_zap_view");
printarg("int ", "compid",compid);
int _ret = _real_dwyco_delete_zap_view(compid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_play_view(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out)
{
printfunname("dwyco_zap_play_view");
printarg("int ", "viewid",viewid);
printarg(" DwycoChannelDestroyCallback ", "dcb",(void *)dcb);
printarg(" void *", "dcb_arg1",dcb_arg1);
printarg(" int *", "ui_id_out",ui_id_out);
int _ret = _real_dwyco_zap_play_view(viewid,dcb,dcb_arg1,ui_id_out);
printargout(" int *", "ui_id_out",ui_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_play_view_no_audio(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out)
{
printfunname("dwyco_zap_play_view_no_audio");
printarg("int ", "viewid",viewid);
printarg(" DwycoChannelDestroyCallback ", "dcb",(void *)dcb);
printarg(" void *", "dcb_arg1",dcb_arg1);
printarg(" int *", "ui_id_out",ui_id_out);
int _ret = _real_dwyco_zap_play_view_no_audio(viewid,dcb,dcb_arg1,ui_id_out);
printargout(" int *", "ui_id_out",ui_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_play_preview(int viewid, DwycoChannelDestroyCallback dcb, void *dcb_arg1, int *ui_id_out)
{
printfunname("dwyco_zap_play_preview");
printarg("int ", "viewid",viewid);
printarg(" DwycoChannelDestroyCallback ", "dcb",(void *)dcb);
printarg(" void *", "dcb_arg1",dcb_arg1);
printarg(" int *", "ui_id_out",ui_id_out);
int _ret = _real_dwyco_zap_play_preview(viewid,dcb,dcb_arg1,ui_id_out);
printargout(" int *", "ui_id_out",ui_id_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_stop_view(int viewid)
{
printfunname("dwyco_zap_stop_view");
printarg("int ", "viewid",viewid);
int _ret = _real_dwyco_zap_stop_view(viewid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_quick_stats_view(int viewid, int *has_video_out, int *has_audio_out, int *short_video_out)
{
printfunname("dwyco_zap_quick_stats_view");
printarg("int ", "viewid",viewid);
printarg(" int *", "has_video_out",has_video_out);
printarg(" int *", "has_audio_out",has_audio_out);
printarg(" int *", "short_video_out",short_video_out);
int _ret = _real_dwyco_zap_quick_stats_view(viewid,has_video_out,has_audio_out,short_video_out);
printargout(" int *", "has_video_out",has_video_out);
printargout(" int *", "has_audio_out",has_audio_out);
printargout(" int *", "short_video_out",short_video_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_create_preview_buf(int viewid, const char **buf_out_elide, int *len_out, int *cols_out, int *rows_out)
{
printfunname("dwyco_zap_create_preview_buf");
printarg("int ", "viewid",viewid);
printarg(" int *", "len_out",len_out);
printarg(" int *", "cols_out",cols_out);
printarg(" int *", "rows_out",rows_out);
int _ret = _real_dwyco_zap_create_preview_buf(viewid,buf_out_elide,len_out,cols_out,rows_out);
printargout(" int *", "len_out",len_out);
printargout(" int *", "cols_out",cols_out);
printargout(" int *", "rows_out",rows_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_zap_create_preview(int viewid, const char *filename, int len_filename)
{
printfunname("dwyco_zap_create_preview");
printarg("int ", "viewid",viewid);
printarg(" const char *", "filename",filename, " int ", "len_filename", len_filename);
int _ret = _real_dwyco_zap_create_preview(viewid,filename,len_filename);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_refresh_users()
{
printfunname("dwyco_get_refresh_users");
int _ret = _real_dwyco_get_refresh_users();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_refresh_users(int i)
{
printfunname("dwyco_set_refresh_users");
printarg("int ", "i",i);
_real_dwyco_set_refresh_users(i);
printret();
}

DWYCOEXPORT
int
dwyco_get_rescan_messages()
{
printfunname("dwyco_get_rescan_messages");
int _ret = _real_dwyco_get_rescan_messages();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_rescan_messages(int i)
{
printfunname("dwyco_set_rescan_messages");
printarg("int ", "i",i);
_real_dwyco_set_rescan_messages(i);
printret();
}

DWYCOEXPORT
int
dwyco_uid_online(const char *uid, int len_uid)
{
printfunname("dwyco_uid_online");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_uid_online(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_uid_status(const char *uid, int len_uid)
{
printfunname("dwyco_uid_status");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_uid_status(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_uid_to_ip(const char *uid, int len_uid, int *can_do_direct_out, char **str_out)
{
printfunname("dwyco_uid_to_ip");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int *", "can_do_direct_out",can_do_direct_out);
printarg(" char **", "str_out",str_out);
_real_dwyco_uid_to_ip(uid,len_uid,can_do_direct_out,str_out);
printargout(" int *", "can_do_direct_out",can_do_direct_out);
printargout(" char **", "str_out",str_out);
printret();
}

DWYCOEXPORT
int
dwyco_uid_to_ip2(const char *uid, int len_uid, int *can_do_direct_out, char **str_out)
{
printfunname("dwyco_uid_to_ip2");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int *", "can_do_direct_out",can_do_direct_out);
printarg(" char **", "str_out",str_out);
int _ret = _real_dwyco_uid_to_ip2(uid,len_uid,can_do_direct_out,str_out);
printargout(" int *", "can_do_direct_out",can_do_direct_out);
printargout(" char **", "str_out",str_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_uid_g(const char *uid, int len_uid)
{
printfunname("dwyco_uid_g");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_uid_g(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_load_users_internal()
{
printfunname("dwyco_load_users_internal");
int _ret = _real_dwyco_load_users_internal();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_load_users2(int recent, int *total_out)
{
printfunname("dwyco_load_users2");
printarg("int ", "recent",recent);
printarg(" int *", "total_out",total_out);
int _ret = _real_dwyco_load_users2(recent,total_out);
printargout(" int *", "total_out",total_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_user_list2(DWYCO_USER_LIST *list_out, int *nelems_out)
{
printfunname("dwyco_get_user_list2");
printarg("DWYCO_USER_LIST *", "list_out",list_out);
printarg(" int *", "nelems_out",nelems_out);
int _ret = _real_dwyco_get_user_list2(list_out,nelems_out);
printargout("DWYCO_USER_LIST *", "list_out",list_out);
printargout(" int *", "nelems_out",nelems_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_updated_uids(DWYCO_USER_LIST *list_out, long time)
{
printfunname("dwyco_get_updated_uids");
printarg("DWYCO_USER_LIST *", "list_out",list_out);
printarg(" long ", "time",time);
int _ret = _real_dwyco_get_updated_uids(list_out,time);
printargout("DWYCO_USER_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_message_index(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid)
{
printfunname("dwyco_get_message_index");
printarg("DWYCO_MSG_IDX *", "list_out",list_out);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_get_message_index(list_out,uid,len_uid);
printargout("DWYCO_MSG_IDX *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_message_index2(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid, int *available_count_out, int load_count)
{
printfunname("dwyco_get_message_index2");
printarg("DWYCO_MSG_IDX *", "list_out",list_out);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int *", "available_count_out",available_count_out);
printarg(" int ", "load_count",load_count);
int _ret = _real_dwyco_get_message_index2(list_out,uid,len_uid,available_count_out,load_count);
printargout("DWYCO_MSG_IDX *", "list_out",list_out);
printargout(" int *", "available_count_out",available_count_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_new_message_index(DWYCO_MSG_IDX *list_out, const char *uid, int len_uid, long logical_clock)
{
printfunname("dwyco_get_new_message_index");
printarg("DWYCO_MSG_IDX *", "list_out",list_out);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" long ", "logical_clock",logical_clock);
int _ret = _real_dwyco_get_new_message_index(list_out,uid,len_uid,logical_clock);
printargout("DWYCO_MSG_IDX *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_get_qd_messages(DWYCO_QD_MSG_LIST *list_out, const char *uid, int len_uid)
{
printfunname("dwyco_get_qd_messages");
printarg("DWYCO_QD_MSG_LIST *", "list_out",list_out);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
_real_dwyco_get_qd_messages(list_out,uid,len_uid);
printargout("DWYCO_QD_MSG_LIST *", "list_out",list_out);
printret();
}

DWYCOEXPORT
int
dwyco_qd_message_to_body(DWYCO_SAVED_MSG_LIST *list_out, const char *pers_id, int len_pers_id)
{
printfunname("dwyco_qd_message_to_body");
printarg("DWYCO_SAVED_MSG_LIST *", "list_out",list_out);
printarg(" const char *", "pers_id",pers_id, " int ", "len_pers_id", len_pers_id);
int _ret = _real_dwyco_qd_message_to_body(list_out,pers_id,len_pers_id);
printargout("DWYCO_SAVED_MSG_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_message_bodies(DWYCO_SAVED_MSG_LIST *list_out, const char *uid, int len_uid, int load_sent)
{
printfunname("dwyco_get_message_bodies");
printarg("DWYCO_SAVED_MSG_LIST *", "list_out",list_out);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int ", "load_sent",load_sent);
int _ret = _real_dwyco_get_message_bodies(list_out,uid,len_uid,load_sent);
printargout("DWYCO_SAVED_MSG_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_start_bulk_update()
{
printfunname("dwyco_start_bulk_update");
_real_dwyco_start_bulk_update();
printret();
}

DWYCOEXPORT
void
dwyco_end_bulk_update()
{
printfunname("dwyco_end_bulk_update");
_real_dwyco_end_bulk_update();
printret();
}

DWYCOEXPORT
int
dwyco_get_sync_model(DWYCO_SYNC_MODEL *list_out)
{
printfunname("dwyco_get_sync_model");
printarg("DWYCO_SYNC_MODEL *", "list_out",list_out);
int _ret = _real_dwyco_get_sync_model(list_out);
printargout("DWYCO_SYNC_MODEL *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_join_log_model(DWYCO_JOIN_LOG_MODEL *list_out)
{
printfunname("dwyco_get_join_log_model");
printarg("DWYCO_JOIN_LOG_MODEL *", "list_out",list_out);
int _ret = _real_dwyco_get_join_log_model(list_out);
printargout("DWYCO_JOIN_LOG_MODEL *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_group_status(DWYCO_LIST *list_out)
{
printfunname("dwyco_get_group_status");
printarg("DWYCO_LIST *", "list_out",list_out);
int _ret = _real_dwyco_get_group_status(list_out);
printargout("DWYCO_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_saved_message3(DWYCO_SAVED_MSG_LIST *list_out, const char *a_msg_id)
{
printfunname("dwyco_get_saved_message3");
printarg("DWYCO_SAVED_MSG_LIST *", "list_out",list_out);
printarg(" const char *", "a_msg_id",a_msg_id);
int _ret = _real_dwyco_get_saved_message3(list_out,a_msg_id);
printargout("DWYCO_SAVED_MSG_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_saved_message(DWYCO_SAVED_MSG_LIST *list_out, const char *uid, int len_uid, const char *msg_id)
{
printfunname("dwyco_get_saved_message");
printarg("DWYCO_SAVED_MSG_LIST *", "list_out",list_out);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "msg_id",msg_id);
int _ret = _real_dwyco_get_saved_message(list_out,uid,len_uid,msg_id);
printargout("DWYCO_SAVED_MSG_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_unfetched_messages(DWYCO_UNFETCHED_MSG_LIST *list_out, const char *uid, int len_uid)
{
printfunname("dwyco_get_unfetched_messages");
printarg("DWYCO_UNFETCHED_MSG_LIST *", "list_out",list_out);
printarg(" const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_get_unfetched_messages(list_out,uid,len_uid);
printargout("DWYCO_UNFETCHED_MSG_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_unfetched_message(DWYCO_UNFETCHED_MSG_LIST *list_out, const char *msg_id)
{
printfunname("dwyco_get_unfetched_message");
printarg("DWYCO_UNFETCHED_MSG_LIST *", "list_out",list_out);
printarg(" const char *", "msg_id",msg_id);
int _ret = _real_dwyco_get_unfetched_message(list_out,msg_id);
printargout("DWYCO_UNFETCHED_MSG_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_mid_disposition(const char *mid)
{
printfunname("dwyco_mid_disposition");
printarg("const char *", "mid",mid);
int _ret = _real_dwyco_mid_disposition(mid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_is_special_message2(DWYCO_UNFETCHED_MSG_LIST ml, int *what_out)
{
printfunname("dwyco_is_special_message2");
printarg("DWYCO_UNFETCHED_MSG_LIST ", "ml",ml);
printarg(" int *", "what_out",what_out);
int _ret = _real_dwyco_is_special_message2(ml,what_out);
printargout(" int *", "what_out",what_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_user_payload(DWYCO_SAVED_MSG_LIST ml, const char **str_out, int *len_out)
{
printfunname("dwyco_get_user_payload");
printarg("DWYCO_SAVED_MSG_LIST ", "ml",ml);
printarg(" const char **", "str_out",str_out);
printarg(" int *", "len_out",len_out);
int _ret = _real_dwyco_get_user_payload(ml,str_out,len_out);
printargout(" const char **", "str_out",str_out, " int *", "len_out", len_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_start_gj2(const char *gname, const char *password)
{
printfunname("dwyco_start_gj2");
printarg("const char *", "gname",gname);
printarg(" const char *", "password",password);
int _ret = _real_dwyco_start_gj2(gname,password);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_is_special_message(const char *msg_id, int *what_out)
{
printfunname("dwyco_is_special_message");
printarg("const char *", "msg_id",msg_id);
printarg(" int *", "what_out",what_out);
int _ret = _real_dwyco_is_special_message(msg_id,what_out);
printargout(" int *", "what_out",what_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_is_delivery_report(const char *mid, const char **uid_out, int *len_uid_out, const char **dlv_mid_out, int *what_out)
{
printfunname("dwyco_is_delivery_report");
printarg("const char *", "mid",mid);
printarg(" const char **", "uid_out",uid_out);
printarg(" int *", "len_uid_out",len_uid_out);
printarg(" const char **", "dlv_mid_out",dlv_mid_out);
printarg(" int *", "what_out",what_out);
int _ret = _real_dwyco_is_delivery_report(mid,uid_out,len_uid_out,dlv_mid_out,what_out);
printargout(" const char **", "uid_out",uid_out, " int *", "len_uid_out", len_uid_out);
printargout(" const char **", "dlv_mid_out",dlv_mid_out);
printargout(" int *", "what_out",what_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
DWYCO_LIST
dwyco_get_body_text(DWYCO_SAVED_MSG_LIST m)
{
printfunname("dwyco_get_body_text");
printarg("DWYCO_SAVED_MSG_LIST ", "m",m);
DWYCO_LIST _ret = _real_dwyco_get_body_text(m);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
DWYCO_LIST
dwyco_get_body_array(DWYCO_SAVED_MSG_LIST m)
{
printfunname("dwyco_get_body_array");
printarg("DWYCO_SAVED_MSG_LIST ", "m",m);
DWYCO_LIST _ret = _real_dwyco_get_body_array(m);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_authenticate_body(DWYCO_SAVED_MSG_LIST m, const char *recip_uid, int len_uid, int unsaved)
{
printfunname("dwyco_authenticate_body");
printarg("DWYCO_SAVED_MSG_LIST ", "m",m);
printarg(" const char *", "recip_uid",recip_uid, " int ", "len_uid", len_uid);
printarg(" int ", "unsaved",unsaved);
int _ret = _real_dwyco_authenticate_body(m,recip_uid,len_uid,unsaved);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_save_message(const char *msg_id)
{
printfunname("dwyco_save_message");
printarg("const char *", "msg_id",msg_id);
int _ret = _real_dwyco_save_message(msg_id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_fetch_server_message(const char *msg_id, DwycoMessageDownloadCallback dcb, void *mdc_arg1, DwycoStatusCallback scb, void *scb_arg1)
{
printfunname("dwyco_fetch_server_message");
printarg("const char *", "msg_id",msg_id);
printarg(" DwycoMessageDownloadCallback ", "dcb",(void *)dcb);
printarg(" void *", "mdc_arg1",mdc_arg1);
printarg(" DwycoStatusCallback ", "scb",(void *)scb);
printarg(" void *", "scb_arg1",scb_arg1);
int _ret = _real_dwyco_fetch_server_message(msg_id,dcb,mdc_arg1,scb,scb_arg1);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_cancel_message_fetch(int fetch_id)
{
printfunname("dwyco_cancel_message_fetch");
printarg("int ", "fetch_id",fetch_id);
_real_dwyco_cancel_message_fetch(fetch_id);
printret();
}

DWYCOEXPORT
int
dwyco_delete_unfetched_message(const char *msg_id)
{
printfunname("dwyco_delete_unfetched_message");
printarg("const char *", "msg_id",msg_id);
int _ret = _real_dwyco_delete_unfetched_message(msg_id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_delete_saved_message(const char *user_id, int len_uid, const char *msg_id)
{
printfunname("dwyco_delete_saved_message");
printarg("const char *", "user_id",user_id, " int ", "len_uid", len_uid);
printarg(" const char *", "msg_id",msg_id);
int _ret = _real_dwyco_delete_saved_message(user_id,len_uid,msg_id);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_pal_add(const char *uid, int len_uid)
{
printfunname("dwyco_pal_add");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
_real_dwyco_pal_add(uid,len_uid);
printret();
}

DWYCOEXPORT
void
dwyco_pal_delete(const char *uid, int len_uid)
{
printfunname("dwyco_pal_delete");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
_real_dwyco_pal_delete(uid,len_uid);
printret();
}

DWYCOEXPORT
int
dwyco_is_pal(const char *uid, int len_uid)
{
printfunname("dwyco_is_pal");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_is_pal(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
DWYCO_LIST
dwyco_pal_get_list()
{
printfunname("dwyco_pal_get_list");
DWYCO_LIST _ret = _real_dwyco_pal_get_list();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_msg_tag(const char *mid, const char *tag)
{
printfunname("dwyco_set_msg_tag");
printarg("const char *", "mid",mid);
printarg(" const char *", "tag",tag);
_real_dwyco_set_msg_tag(mid,tag);
printret();
}

DWYCOEXPORT
void
dwyco_unset_msg_tag(const char *mid, const char *tag)
{
printfunname("dwyco_unset_msg_tag");
printarg("const char *", "mid",mid);
printarg(" const char *", "tag",tag);
_real_dwyco_unset_msg_tag(mid,tag);
printret();
}

DWYCOEXPORT
void
dwyco_unset_all_msg_tag(const char *tag)
{
printfunname("dwyco_unset_all_msg_tag");
printarg("const char *", "tag",tag);
_real_dwyco_unset_all_msg_tag(tag);
printret();
}

DWYCOEXPORT
int
dwyco_get_tagged_mids(DWYCO_LIST *list_out, const char *tag)
{
printfunname("dwyco_get_tagged_mids");
printarg("DWYCO_LIST *", "list_out",list_out);
printarg(" const char *", "tag",tag);
int _ret = _real_dwyco_get_tagged_mids(list_out,tag);
printargout("DWYCO_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_tagged_mids2(DWYCO_LIST *list_out, const char *tag)
{
printfunname("dwyco_get_tagged_mids2");
printarg("DWYCO_LIST *", "list_out",list_out);
printarg(" const char *", "tag",tag);
int _ret = _real_dwyco_get_tagged_mids2(list_out,tag);
printargout("DWYCO_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_tagged_mids_older_than(DWYCO_LIST *list_out, const char *tag, int days)
{
printfunname("dwyco_get_tagged_mids_older_than");
printarg("DWYCO_LIST *", "list_out",list_out);
printarg(" const char *", "tag",tag);
printarg(" int ", "days",days);
int _ret = _real_dwyco_get_tagged_mids_older_than(list_out,tag,days);
printargout("DWYCO_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_tagged_idx(DWYCO_MSG_IDX *list_out, const char *tag, int order_by_tag_time)
{
printfunname("dwyco_get_tagged_idx");
printarg("DWYCO_MSG_IDX *", "list_out",list_out);
printarg(" const char *", "tag",tag);
printarg(" int ", "order_by_tag_time",order_by_tag_time);
int _ret = _real_dwyco_get_tagged_idx(list_out,tag,order_by_tag_time);
printargout("DWYCO_MSG_IDX *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_mid_has_tag(const char *mid, const char *tag)
{
printfunname("dwyco_mid_has_tag");
printarg("const char *", "mid",mid);
printarg(" const char *", "tag",tag);
int _ret = _real_dwyco_mid_has_tag(mid,tag);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_uid_has_tag(const char *uid, int len_uid, const char *tag)
{
printfunname("dwyco_uid_has_tag");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "tag",tag);
int _ret = _real_dwyco_uid_has_tag(uid,len_uid,tag);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_uid_count_tag(const char *uid, int len_uid, const char *tag)
{
printfunname("dwyco_uid_count_tag");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "tag",tag);
int _ret = _real_dwyco_uid_count_tag(uid,len_uid,tag);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_count_tag(const char *tag)
{
printfunname("dwyco_count_tag");
printarg("const char *", "tag",tag);
int _ret = _real_dwyco_count_tag(tag);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_valid_tag_exists(const char *tag)
{
printfunname("dwyco_valid_tag_exists");
printarg("const char *", "tag",tag);
int _ret = _real_dwyco_valid_tag_exists(tag);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_all_messages_tagged(const char *uid, int len_uid, const char *tag)
{
printfunname("dwyco_all_messages_tagged");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" const char *", "tag",tag);
int _ret = _real_dwyco_all_messages_tagged(uid,len_uid,tag);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_fav_msg(const char *mid, int fav)
{
printfunname("dwyco_set_fav_msg");
printarg("const char *", "mid",mid);
printarg(" int ", "fav",fav);
_real_dwyco_set_fav_msg(mid,fav);
printret();
}

DWYCOEXPORT
int
dwyco_get_fav_msg(const char *mid)
{
printfunname("dwyco_get_fav_msg");
printarg("const char *", "mid",mid);
int _ret = _real_dwyco_get_fav_msg(mid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_run_sql(const char *stmt, const char *a1, const char *a2, const char *a3)
{
printfunname("dwyco_run_sql");
printarg("const char *", "stmt",stmt);
printarg(" const char *", "a1",a1);
printarg(" const char *", "a2",a2);
printarg(" const char *", "a3",a3);
int _ret = _real_dwyco_run_sql(stmt,a1,a2,a3);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_is_ignored(const char *uid, int len_uid)
{
printfunname("dwyco_is_ignored");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
int _ret = _real_dwyco_is_ignored(uid,len_uid);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_ignore(const char *uid, int len_uid)
{
printfunname("dwyco_ignore");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
_real_dwyco_ignore(uid,len_uid);
printret();
}

DWYCOEXPORT
void
dwyco_unignore(const char *uid, int len_uid)
{
printfunname("dwyco_unignore");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
_real_dwyco_unignore(uid,len_uid);
printret();
}

DWYCOEXPORT
DWYCO_LIST
dwyco_ignore_list_get()
{
printfunname("dwyco_ignore_list_get");
DWYCO_LIST _ret = _real_dwyco_ignore_list_get();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_pals_only(int on)
{
printfunname("dwyco_set_pals_only");
printarg("int ", "on",on);
_real_dwyco_set_pals_only(on);
printret();
}

DWYCOEXPORT
int
dwyco_get_pals_only()
{
printfunname("dwyco_get_pals_only");
int _ret = _real_dwyco_get_pals_only();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
DWYCO_LIST
dwyco_uid_to_info(const char *uid, int len_uid, int* cant_resolve_now_out)
{
printfunname("dwyco_uid_to_info");
printarg("const char *", "uid",uid, " int ", "len_uid", len_uid);
printarg(" int* ", "cant_resolve_now_out",cant_resolve_now_out);
DWYCO_LIST _ret = _real_dwyco_uid_to_info(uid,len_uid,cant_resolve_now_out);
printargout(" int* ", "cant_resolve_now_out",cant_resolve_now_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_list_release(DWYCO_LIST l)
{
printfunname("dwyco_list_release");
printarg("DWYCO_LIST ", "l",l);
_real_dwyco_list_release(l);
printret();
}

DWYCOEXPORT
int
dwyco_list_numelems(DWYCO_LIST l, int *rows_out, int *cols_out)
{
printfunname("dwyco_list_numelems");
printarg("DWYCO_LIST ", "l",l);
printarg(" int *", "rows_out",rows_out);
printarg(" int *", "cols_out",cols_out);
int _ret = _real_dwyco_list_numelems(l,rows_out,cols_out);
printargout(" int *", "rows_out",rows_out);
printargout(" int *", "cols_out",cols_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_list_get(DWYCO_LIST l, int row, const char *col, const char **val_out, int *len_out, int *type_out)
{
printfunname("dwyco_list_get");
printarg("DWYCO_LIST ", "l",l);
printarg(" int ", "row",row);
printarg(" const char *", "col",col);
printarg(" const char **", "val_out",val_out);
printarg(" int *", "len_out",len_out);
printarg(" int *", "type_out",type_out);
int _ret = _real_dwyco_list_get(l,row,col,val_out,len_out,type_out);
printargout(" const char **", "val_out",val_out, " int *", "len_out", len_out);
printargout(" int *", "type_out",type_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_list_print(DWYCO_LIST l)
{
printfunname("dwyco_list_print");
printarg("DWYCO_LIST ", "l",l);
int _ret = _real_dwyco_list_print(l);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
DWYCO_LIST
dwyco_list_copy(DWYCO_LIST l)
{
printfunname("dwyco_list_copy");
printarg("DWYCO_LIST ", "l",l);
DWYCO_LIST _ret = _real_dwyco_list_copy(l);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
DWYCO_LIST
dwyco_list_new()
{
printfunname("dwyco_list_new");
DWYCO_LIST _ret = _real_dwyco_list_new();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_list_append(DWYCO_LIST l, const char *val, int len, int type)
{
printfunname("dwyco_list_append");
printarg("DWYCO_LIST ", "l",l);
printarg(" const char *", "val",val, " int ", "len", len);
printarg(" int ", "type",type);
_real_dwyco_list_append(l,val,len,type);
printret();
}

DWYCOEXPORT
void
dwyco_list_append_int(DWYCO_LIST l, int i)
{
printfunname("dwyco_list_append_int");
printarg("DWYCO_LIST ", "l",l);
printarg(" int ", "i",i);
_real_dwyco_list_append_int(l,i);
printret();
}

DWYCOEXPORT
void
dwyco_list_to_string(DWYCO_LIST l, const char **str_out, int *len_out)
{
printfunname("dwyco_list_to_string");
printarg("DWYCO_LIST ", "l",l);
printarg(" const char **", "str_out",str_out);
printarg(" int *", "len_out",len_out);
_real_dwyco_list_to_string(l,str_out,len_out);
printargout(" const char **", "str_out",str_out, " int *", "len_out", len_out);
printret();
}

DWYCOEXPORT
int
dwyco_list_from_string(DWYCO_LIST *list_out, const char *str, int len_str)
{
printfunname("dwyco_list_from_string");
printarg("DWYCO_LIST *", "list_out",list_out);
printarg(" const char *", "str",str, " int ", "len_str", len_str);
int _ret = _real_dwyco_list_from_string(list_out,str,len_str);
printargout("DWYCO_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
DWYCO_LIST
dwyco_get_vfw_drivers()
{
printfunname("dwyco_get_vfw_drivers");
DWYCO_LIST _ret = _real_dwyco_get_vfw_drivers();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_start_vfw(int idx, void *main_hwnd, void *client_hwnd)
{
printfunname("dwyco_start_vfw");
printarg("int ", "idx",idx);
printarg(" void *", "main_hwnd",main_hwnd);
printarg(" void *", "client_hwnd",client_hwnd);
int _ret = _real_dwyco_start_vfw(idx,main_hwnd,client_hwnd);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_shutdown_vfw()
{
printfunname("dwyco_shutdown_vfw");
int _ret = _real_dwyco_shutdown_vfw();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_change_driver(int new_idx)
{
printfunname("dwyco_change_driver");
printarg("int ", "new_idx",new_idx);
int _ret = _real_dwyco_change_driver(new_idx);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_is_preview_on()
{
printfunname("dwyco_is_preview_on");
int _ret = _real_dwyco_is_preview_on();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_preview_on(void *display_window)
{
printfunname("dwyco_preview_on");
printarg("void *", "display_window",display_window);
int _ret = _real_dwyco_preview_on(display_window);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_preview_off()
{
printfunname("dwyco_preview_off");
int _ret = _real_dwyco_preview_off();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_vfw_format()
{
printfunname("dwyco_vfw_format");
int _ret = _real_dwyco_vfw_format();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_vfw_source()
{
printfunname("dwyco_vfw_source");
int _ret = _real_dwyco_vfw_source();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_set_external_video(int v)
{
printfunname("dwyco_set_external_video");
printarg("int ", "v",v);
int _ret = _real_dwyco_set_external_video(v);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_external_video_capture_callbacks( DwycoVVCallback nw, DwycoVVCallback del, DwycoIVICallback init, DwycoIVCallback has_data, DwycoVVCallback need, DwycoVVCallback pass, DwycoVVCallback stop, DwycoVidGetDataCallback get_data, DwycoVVCallback free_data, DwycoCACallback get_vid_devices, DwycoFCACallback free_vid_list, DwycoVICallback set_vid_device, DwycoVCallback stop_vid_device, DwycoVCallback show_source_dialog, DwycoVVCallback hw_preview_on, DwycoVCallback hw_preview_off, DwycoVVCallback set_app_data)
{
printfunname("dwyco_set_external_video_capture_callbacks");
printarg(" DwycoVVCallback ", "nw",(void *)nw);
printarg(" DwycoVVCallback ", "del",(void *)del);
printarg(" DwycoIVICallback ", "init",(void *)init);
printarg(" DwycoIVCallback ", "has_data",(void *)has_data);
printarg(" DwycoVVCallback ", "need",(void *)need);
printarg(" DwycoVVCallback ", "pass",(void *)pass);
printarg(" DwycoVVCallback ", "stop",(void *)stop);
printarg(" DwycoVidGetDataCallback ", "get_data",(void *)get_data);
printarg(" DwycoVVCallback ", "free_data",(void *)free_data);
printarg(" DwycoCACallback ", "get_vid_devices",(void *)get_vid_devices);
printarg(" DwycoFCACallback ", "free_vid_list",(void *)free_vid_list);
printarg(" DwycoVICallback ", "set_vid_device",(void *)set_vid_device);
printarg(" DwycoVCallback ", "stop_vid_device",(void *)stop_vid_device);
printarg(" DwycoVCallback ", "show_source_dialog",(void *)show_source_dialog);
printarg(" DwycoVVCallback ", "hw_preview_on",(void *)hw_preview_on);
printarg(" DwycoVCallback ", "hw_preview_off",(void *)hw_preview_off);
printarg(" DwycoVVCallback ", "set_app_data",(void *)set_app_data);
_real_dwyco_set_external_video_capture_callbacks(nw,del,init,has_data,need,pass,stop,get_data,free_data,get_vid_devices,free_vid_list,set_vid_device,stop_vid_device,show_source_dialog,hw_preview_on,hw_preview_off,set_app_data);
printret();
}

DWYCOEXPORT
void
dwyco_set_external_audio_capture_callbacks( DwycoVVIICallback nw, DwycoVVCallback del, DwycoIVCallback init, DwycoIVCallback has_data, DwycoVVCallback need, DwycoVVCallback pass, DwycoVVCallback stop, DwycoVVCallback on, DwycoVVCallback off, DwycoVVCallback reset, DwycoIVCallback status, DwycoAudGetDataCallback get_data)
{
printfunname("dwyco_set_external_audio_capture_callbacks");
printarg(" DwycoVVIICallback ", "nw",(void *)nw);
printarg(" DwycoVVCallback ", "del",(void *)del);
printarg(" DwycoIVCallback ", "init",(void *)init);
printarg(" DwycoIVCallback ", "has_data",(void *)has_data);
printarg(" DwycoVVCallback ", "need",(void *)need);
printarg(" DwycoVVCallback ", "pass",(void *)pass);
printarg(" DwycoVVCallback ", "stop",(void *)stop);
printarg(" DwycoVVCallback ", "on",(void *)on);
printarg(" DwycoVVCallback ", "off",(void *)off);
printarg(" DwycoVVCallback ", "reset",(void *)reset);
printarg(" DwycoIVCallback ", "status",(void *)status);
printarg(" DwycoAudGetDataCallback ", "get_data",(void *)get_data);
_real_dwyco_set_external_audio_capture_callbacks(nw,del,init,has_data,need,pass,stop,on,off,reset,status,get_data);
printret();
}

DWYCOEXPORT
void
dwyco_set_external_audio_output_callbacks( DwycoVVCallback nw, DwycoVVCallback del, DwycoIVCallback init, DwycoDevOutputCallback output, DwycoDevDoneCallback done, DwycoIVCallback stop, DwycoIVCallback reset, DwycoIVCallback status, DwycoIVCallback close, DwycoIICallback buffer_time, DwycoIVCallback play_silence, DwycoIVCallback bufs_playing)
{
printfunname("dwyco_set_external_audio_output_callbacks");
printarg(" DwycoVVCallback ", "nw",(void *)nw);
printarg(" DwycoVVCallback ", "del",(void *)del);
printarg(" DwycoIVCallback ", "init",(void *)init);
printarg(" DwycoDevOutputCallback ", "output",(void *)output);
printarg(" DwycoDevDoneCallback ", "done",(void *)done);
printarg(" DwycoIVCallback ", "stop",(void *)stop);
printarg(" DwycoIVCallback ", "reset",(void *)reset);
printarg(" DwycoIVCallback ", "status",(void *)status);
printarg(" DwycoIVCallback ", "close",(void *)close);
printarg(" DwycoIICallback ", "buffer_time",(void *)buffer_time);
printarg(" DwycoIVCallback ", "play_silence",(void *)play_silence);
printarg(" DwycoIVCallback ", "bufs_playing",(void *)bufs_playing);
_real_dwyco_set_external_audio_output_callbacks(nw,del,init,output,done,stop,reset,status,close,buffer_time,play_silence,bufs_playing);
printret();
}

DWYCOEXPORT
void
dwyco_force_autoupdate_check()
{
printfunname("dwyco_force_autoupdate_check");
_real_dwyco_force_autoupdate_check();
printret();
}

DWYCOEXPORT
void
dwyco_set_autoupdate_status_callback(DwycoAutoUpdateStatusCallback sb)
{
printfunname("dwyco_set_autoupdate_status_callback");
printarg("DwycoAutoUpdateStatusCallback ", "sb",(void *)sb);
user_dwyco_set_autoupdate_status_callback=sb;
sb=wrapped_cb_dwyco_set_autoupdate_status_callback;
_real_dwyco_set_autoupdate_status_callback(sb);
printret();
}

DWYCOEXPORT
int
dwyco_start_autoupdate_download(DwycoStatusCallback cb, void *arg1, DwycoAutoUpdateDownloadCallback dcb)
{
printfunname("dwyco_start_autoupdate_download");
printarg("DwycoStatusCallback ", "cb",(void *)cb);
printarg(" void *", "arg1",arg1);
printarg(" DwycoAutoUpdateDownloadCallback ", "dcb",(void *)dcb);
int _ret = _real_dwyco_start_autoupdate_download(cb,arg1,dcb);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_start_autoupdate_download_bg()
{
printfunname("dwyco_start_autoupdate_download_bg");
int _ret = _real_dwyco_start_autoupdate_download_bg();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_run_autoupdate()
{
printfunname("dwyco_run_autoupdate");
int _ret = _real_dwyco_run_autoupdate();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_abort_autoupdate_download()
{
printfunname("dwyco_abort_autoupdate_download");
_real_dwyco_abort_autoupdate_download();
printret();
}

DWYCOEXPORT
void
dwyco_network_diagnostics2(char **report_out, int *len_out)
{
printfunname("dwyco_network_diagnostics2");
printarg("char **", "report_out",report_out);
printarg(" int *", "len_out",len_out);
_real_dwyco_network_diagnostics2(report_out,len_out);
printargout("char **", "report_out",report_out, " int *", "len_out", len_out);
printret();
}

DWYCOEXPORT
void
dwyco_estimate_bandwidth2(int *out_bw_out, int *in_bw_out)
{
printfunname("dwyco_estimate_bandwidth2");
printarg("int *", "out_bw_out",out_bw_out);
printarg(" int *", "in_bw_out",in_bw_out);
_real_dwyco_estimate_bandwidth2(out_bw_out,in_bw_out);
printargout("int *", "out_bw_out",out_bw_out);
printargout(" int *", "in_bw_out",in_bw_out);
printret();
}

DWYCOEXPORT
int
dwyco_create_backup(int days_to_run, int days_to_rebuild)
{
printfunname("dwyco_create_backup");
printarg("int ", "days_to_run",days_to_run);
printarg(" int ", "days_to_rebuild",days_to_rebuild);
int _ret = _real_dwyco_create_backup(days_to_run,days_to_rebuild);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_copy_out_backup(const char *dir, int force)
{
printfunname("dwyco_copy_out_backup");
printarg("const char *", "dir",dir);
printarg(" int ", "force",force);
int _ret = _real_dwyco_copy_out_backup(dir,force);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_remove_backup()
{
printfunname("dwyco_remove_backup");
_real_dwyco_remove_backup();
printret();
}

DWYCOEXPORT
int
dwyco_restore_from_backup(const char *bu_fn, int msgs_only)
{
printfunname("dwyco_restore_from_backup");
printarg("const char *", "bu_fn",bu_fn);
printarg(" int ", "msgs_only",msgs_only);
int _ret = _real_dwyco_restore_from_backup(bu_fn,msgs_only);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_android_backup_state()
{
printfunname("dwyco_get_android_backup_state");
int _ret = _real_dwyco_get_android_backup_state();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_set_android_backup_state(int i)
{
printfunname("dwyco_set_android_backup_state");
printarg("int ", "i",i);
int _ret = _real_dwyco_set_android_backup_state(i);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_restore_android_backup()
{
printfunname("dwyco_restore_android_backup");
int _ret = _real_dwyco_restore_android_backup();
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_set_aux_string(const char *str)
{
printfunname("dwyco_set_aux_string");
printarg("const char *", "str",str);
_real_dwyco_set_aux_string(str);
printret();
}

DWYCOEXPORT
void
dwyco_write_token(const char *token)
{
printfunname("dwyco_write_token");
printarg("const char *", "token",token);
_real_dwyco_write_token(token);
printret();
}

DWYCOEXPORT
void
dwyco_clear_contact_list()
{
printfunname("dwyco_clear_contact_list");
_real_dwyco_clear_contact_list();
printret();
}

DWYCOEXPORT
int
dwyco_add_contact(const char *name, const char *phone, const char *email)
{
printfunname("dwyco_add_contact");
printarg("const char *", "name",name);
printarg(" const char *", "phone",phone);
printarg(" const char *", "email",email);
int _ret = _real_dwyco_add_contact(name,phone,email);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_contact_list(DWYCO_LIST *list_out)
{
printfunname("dwyco_get_contact_list");
printarg("DWYCO_LIST *", "list_out",list_out);
int _ret = _real_dwyco_get_contact_list(list_out);
printargout("DWYCO_LIST *", "list_out",list_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_get_aux_string(const char **str_out, int *len_str_out)
{
printfunname("dwyco_get_aux_string");
printarg("const char **", "str_out",str_out);
printarg(" int *", "len_str_out",len_str_out);
int _ret = _real_dwyco_get_aux_string(str_out,len_str_out);
printargout("const char **", "str_out",str_out, " int *", "len_str_out", len_str_out);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
void
dwyco_signal_msg_cond()
{
printfunname("dwyco_signal_msg_cond");
_real_dwyco_signal_msg_cond();
printret();
}

DWYCOEXPORT
void
dwyco_wait_msg_cond(int ms)
{
printfunname("dwyco_wait_msg_cond");
printarg("int ", "ms",ms);
_real_dwyco_wait_msg_cond(ms);
printret();
}

DWYCOEXPORT
int
dwyco_test_funny_mutex(int port)
{
printfunname("dwyco_test_funny_mutex");
printarg("int ", "port",port);
int _ret = _real_dwyco_test_funny_mutex(port);
printretval(_ret);
return(_ret);
}

DWYCOEXPORT
int
dwyco_background_sync(int port, const char *sys_pfx, const char *user_pfx, const char *tmp_pfx, const char *token, const char *grpname, const char *grppw)
{
printfunname("dwyco_background_sync");
printarg("int ", "port",port);
printarg(" const char *", "sys_pfx",sys_pfx);
printarg(" const char *", "user_pfx",user_pfx);
printarg(" const char *", "tmp_pfx",tmp_pfx);
printarg(" const char *", "token",token);
printarg(" const char *", "grpname",grpname);
printarg(" const char *", "grppw",grppw);
int _ret = _real_dwyco_background_sync(port,sys_pfx,user_pfx,tmp_pfx,token,grpname,grppw);
printretval(_ret);
return(_ret);
}

