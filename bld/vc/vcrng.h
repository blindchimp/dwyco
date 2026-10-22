
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
;
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCRNG_H
#define VCRNG_H

#include "config.h"
#include "randpool.h"

#if !defined(NO_OS_DEPENDENCE) && defined(OS_RNG_AVAILABLE)
#include "osrng.h"
#endif

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>

namespace dwyco {

// create an OS-seeded generator, falling back to a plain RandomPool
// when the OS RNG is unavailable. any supplied seed is always mixed
// into the generator so a saved local pool still contributes.
inline CryptoPP::RandomPool *
make_rng(const void *seed = 0, size_t seed_len = 0)
{
    CryptoPP::RandomPool *rng = 0;
    bool os_seeded = false;

#if !defined(NO_OS_DEPENDENCE) && defined(OS_RNG_AVAILABLE)
    try
    {
        rng = new CryptoPP::AutoSeededRandomPool(false, 32);
        os_seeded = true;
    }
    catch(const CryptoPP::Exception&)
    {
        rng = 0;
    }
#endif
    if(!rng)
        rng = new CryptoPP::RandomPool;

    if(seed && seed_len)
        rng->IncorporateEntropy((const CryptoPP::byte *)seed, seed_len);

    if(!os_seeded)
    {
        // last-resort fallback: enough cheap, varying data to keep the
        // pool from starting out with a predictable all-zero state.
        struct Noise
        {
            std::uint64_t clock;
            std::time_t time;
            std::uintptr_t self;
            unsigned int r1;
            unsigned int r2;
        } noise;
        std::memset(&noise, 0, sizeof(noise));
        noise.clock = (std::uint64_t)
                      std::chrono::high_resolution_clock::now().time_since_epoch().count();
        noise.time = std::time(0);
        noise.self = (std::uintptr_t)&noise;
        noise.r1 = (unsigned int)std::rand();
        noise.r2 = (unsigned int)std::rand();
        rng->IncorporateEntropy((const CryptoPP::byte *)&noise, sizeof(noise));
    }

    return rng;
}

inline void
rng_add(CryptoPP::RandomPool *rng, const void *data, size_t len)
{
    if(rng && data && len)
        rng->IncorporateEntropy((const CryptoPP::byte *)data, len);
}

}

#endif
