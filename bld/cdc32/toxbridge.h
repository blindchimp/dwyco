#ifndef TOXBRIDGE_H
#define TOXBRIDGE_H

#include "vc.h"
#include "ser.h"
#include "simplesql.h"

namespace dwyco {

class ToxQueue : public SimpleSql
{
public:
    ToxQueue();
    void init_schema(const DwString&);
    int enqueue(const vc &qqm_blob, const vc &recipient_pseudo, const vc &local_mid);
    vc dequeue(vc *recipient_pseudo_out, vc *local_mid_out, int64_t *row_id);
    int mark_inprogress(int64_t row_id, uint32_t tox_mid);
    int mark_sent(int64_t row_id);
    int mark_failed(int64_t row_id);
    vc load_qqm_blob(int64_t row_id);
    void recover_inprogress();
};

// NOTE: for this API, identifiers and addresses are all
// binary strings. there is no hex encoding.

// tox bridge lifecycle
int tox_bridge_init(const char *toxd_path, const char *data_dir);
void tox_bridge_shutdown();
int tox_bridge_is_active();

// polling — call from dwyco timer loop
void tox_bridge_poll();

// identity
vc tox_bridge_get_address();
vc tox_bridge_get_pubkey();

// contact management
int tox_bridge_friend_add(const vc &address, const vc &message);
int tox_bridge_friend_add_norequest(const vc &pubkey);
int tox_bridge_friend_delete(uint32_t friend_number);

// messaging
int tox_bridge_send_message(uint32_t friend_number, const vc &text, int is_action, uint32_t *mid_out = 0);
int tox_bridge_send_message_by_uid(const vc &pseudo_uid, const vc &text, int is_action);
void tox_bridge_send_queued();

// file transfer
int tox_bridge_file_send(uint32_t friend_number, const vc &name, uint64_t size);
int tox_bridge_file_send_data(uint32_t friend_number, uint32_t file_number,
                              uint64_t pos, const vc &data);
int tox_bridge_file_accept(uint32_t friend_number, uint32_t file_number);

// pseudo-uid mapping
vc tox_pubkey_to_pseudo_uid(const vc &pubkey);
int tox_pseudo_uid_to_friend_number(const vc &pseudo_uid, uint32_t *fn_out);
int tox_friend_number_to_pseudo_uid(uint32_t fn, vc &pseudo_uid_out);

// friend number lookup (run on startup to rebuild cache)
void tox_bridge_rebuild_friend_cache();

// convenience wrappers for dlli
int tox_bridge_is_tox_uid(const vc &uid);
// must call dwyco_free_array on returned pointer
int tox_bridge_get_self_public_key(char **out, int *len_out);
vc tox_bridge_get_friend_list_vc();

}

#endif
