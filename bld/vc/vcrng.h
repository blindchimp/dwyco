
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCRNG_H
#define VCRNG_H

#include <cstddef>

namespace CryptoPP {
class RandomPool;
}

namespace dwyco {

// create an OS-seeded generator, falling back to a plain RandomPool
// when the OS RNG is unavailable. any supplied seed is always mixed
// into the generator so a saved local pool still contributes.
CryptoPP::RandomPool *make_rng(const void *seed = 0, size_t seed_len = 0);

void rng_add(CryptoPP::RandomPool *rng, const void *data, size_t len);

}

#endif
