#ifndef ACTIVEUID_H
#define ACTIVEUID_H

#include "vc.h"

namespace dwyco {
vc find_best_candidate_for_initial_send(vc uid, int &skip_direct);
}

#endif // ACTIVEUID_H
