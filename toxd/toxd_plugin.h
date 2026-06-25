#ifndef TOXD_PLUGIN_H
#define TOXD_PLUGIN_H

#include "vc.h"

struct ToxPlugin;

typedef void (*ToxpEventCB)(const char *type, const vc &args, void *userdata);

ToxPlugin *toxp_init(const char *data_dir, ToxpEventCB cb, void *userdata);
void toxp_shutdown(ToxPlugin *p);
void toxp_save(ToxPlugin *p);
void toxp_iterate(ToxPlugin *p);

vc toxp_get_address(ToxPlugin *p);
vc toxp_get_self_pubkey(ToxPlugin *p);
vc toxp_get_name(ToxPlugin *p);
vc toxp_get_status_message(ToxPlugin *p);

int toxp_set_name(ToxPlugin *p, const char *name, int len);
int toxp_set_status_message(ToxPlugin *p, const char *msg, int len);
void toxp_set_user_status(ToxPlugin *p, const char *status);
vc toxp_get_user_status(ToxPlugin *p);

int toxp_friend_add(ToxPlugin *p, const vc &address, const vc &message, uint32_t *fn_out);
int toxp_friend_add_norequest(ToxPlugin *p, const vc &pubkey, uint32_t *fn_out);
int toxp_friend_delete(ToxPlugin *p, uint32_t fn);
vc toxp_friend_list(ToxPlugin *p);

int toxp_message_send(ToxPlugin *p, uint32_t fn, const vc &text, int is_action, uint32_t *mid_out, int *tox_err_out);
int toxp_typing_set(ToxPlugin *p, uint32_t fn, int typing);

int toxp_file_send(ToxPlugin *p, uint32_t fn, const vc &name, uint64_t size, uint32_t *fnum_out);
int toxp_file_send_data(ToxPlugin *p, uint32_t fn, uint32_t fnum, uint64_t pos, const vc &data);
int toxp_file_accept(ToxPlugin *p, uint32_t fn, uint32_t fnum, int *error_out);
int toxp_file_cancel(ToxPlugin *p, uint32_t fn, uint32_t fnum);

#endif
