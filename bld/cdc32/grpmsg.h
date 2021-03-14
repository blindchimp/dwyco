#ifndef GRPMSG_H
#define GRPMSG_H
#include "vc.h"

namespace dwyco {
int init_gj();
int exit_gj();
void clear_gj();

// initiator
int start_gj(vc target_uid, vc gname, vc password);
int recv_gj2(vc from, vc msg, vc password);
int install_group_key(vc from, vc msg, vc password);
//responder
int recv_gj1(vc from, vc msg, vc password);
int recv_gj3(vc from, vc msg, vc password);
}
#endif
