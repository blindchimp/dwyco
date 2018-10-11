#ifndef GRPMSG_H
#define GRPMSG_H
#include "vc.h"

namespace dwyco {
int init_gj();
int exit_gj();
int start_gj(vc target_uid, vc password);
}
#endif
