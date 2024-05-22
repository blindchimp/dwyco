#ifndef GRPMSG_H
#define GRPMSG_H
#include "vc.h"
#include "ssns.h"

namespace dwyco {
int init_gj();
int exit_gj();
void clear_gj();
vc get_status_gj();
vc get_join_log();
void add_join_log(vc msg, vc uid);

// initiator
int start_gj(vc target_uid, vc gname, vc password);
int recv_gj2(vc from, vc msg, vc password);
int install_group_key(vc from, vc msg, vc password);
//responder
int recv_gj1(vc from, vc msg, vc password);
int recv_gj3(vc from, vc msg, vc password);

extern ssns::signal1<vc> Join_signal;
extern ssns::signal2<vc, vc> Join_attempts;

}
#endif
