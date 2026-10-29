#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <typeinfo>
#include <vector>
#include <vc.h>
#include <dwqmap4.h>
#include <dwqmap3.h>

[[noreturn]] void oopanic(const char *a) {
    std::fprintf(stderr, "oopanic: %s\n", a);
    std::abort();
}

using Clock = std::chrono::high_resolution_clock;

static double ns_per_op(std::chrono::nanoseconds d, long n) {
    return (double)d.count() / (double)n;
}

template <class M>
static void bench_empty_create_destroy(long iters) {
    long sink = 0;
    auto t0 = Clock::now();
    for (long i = 0; i < iters; ++i) {
        M m;
        sink += m.num_elems();
    }
    auto t1 = Clock::now();
    std::printf("empty create/destroy: %s  iters=%ld  ns/op=%.3f  sink=%ld\n",
                typeid(M).name(), iters, ns_per_op(t1 - t0, iters), sink);
}

template <class M>
static void bench_insert(long iters, int n) {
    std::vector<vc> keys;
    std::vector<vc> vals;
    keys.reserve(n);
    vals.reserve(n);
    for (int k = 0; k < n; ++k) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "key%d", k);
        keys.emplace_back(VC_STRING, buf);
        vals.emplace_back(VC_STRING, buf);
    }
    long sink = 0;
    auto t0 = Clock::now();
    for (long i = 0; i < iters; ++i) {
        M m;
        for (int k = 0; k < n; ++k) m.add(keys[k], vals[k]);
        sink += m.num_elems();
    }
    auto t1 = Clock::now();
    std::printf("insert %d: %s  iters=%ld  ns/op=%.2f  sink=%ld\n",
                n, typeid(M).name(), iters, ns_per_op(t1 - t0, iters), sink);
}

template <class M>
static void bench_lookup(long iters, int n, int mlook) {
    std::vector<vc> keys;
    std::vector<vc> vals;
    keys.reserve(n);
    vals.reserve(n);
    for (int k = 0; k < n; ++k) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "key%d", k);
        keys.emplace_back(VC_STRING, buf);
        vals.emplace_back(VC_STRING, buf);
    }
    long sink = 0;
    auto t0 = Clock::now();
    for (long i = 0; i < iters; ++i) {
        M mm;
        for (int k = 0; k < n; ++k) mm.add(keys[k], vals[k]);
        for (int j = 0; j < mlook; ++j) {
            vc v = mm.get(keys[j % n]);
            sink += v.type();
        }
    }
    auto t1 = Clock::now();
    std::printf("lookup n=%d m=%d: %s  iters=%ld  ns/op=%.2f  sink=%ld\n",
                n, mlook, typeid(M).name(), iters, ns_per_op(t1 - t0, iters), sink);
}

template <class M>
static void bench_lookup_miss(long iters, int n, int mlook) {
    std::vector<vc> keys, vals, miss;
    keys.reserve(n); vals.reserve(n); miss.reserve(mlook);
    for (int k = 0; k < n; ++k) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "key%d", k);
        keys.emplace_back(VC_STRING, buf);
        vals.emplace_back(VC_STRING, buf);
    }
    for (int j = 0; j < mlook; ++j) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "missing%d", j);
        miss.emplace_back(VC_STRING, buf);
    }
    long sink = 0;
    auto t0 = Clock::now();
    for (long i = 0; i < iters; ++i) {
        M mm;
        for (int k = 0; k < n; ++k) mm.add(keys[k], vals[k]);
        for (int j = 0; j < mlook; ++j) {
            vc v = mm.get(miss[j]);
            sink += v.type();
        }
    }
    auto t1 = Clock::now();
    std::printf("lookup_miss n=%d m=%d: %s  iters=%ld  ns/op=%.2f  ns/lookup=%.2f  sink=%ld\n",
                n, mlook, typeid(M).name(), iters, ns_per_op(t1 - t0, iters),
                ns_per_op(t1 - t0, iters * mlook), sink);
}

int main() {
    const long EMPTY_ITERS = 200000;
    const long SMALL_ITERS = 50000;

    std::printf("== DwQMap4<vc,vc,8> ==\n");
    bench_empty_create_destroy<DwQMap4<vc, vc, 8>>(EMPTY_ITERS);
    for (int n = 1; n <= 5; ++n) bench_insert<DwQMap4<vc, vc, 8>>(SMALL_ITERS, n);
    for (int n = 1; n <= 5; ++n) bench_lookup<DwQMap4<vc, vc, 8>>(SMALL_ITERS, n, n);
    for (int n : {1,2,4,7}) bench_lookup_miss<DwQMap4<vc, vc, 8>>(SMALL_ITERS, n, 5);

    std::printf("== DwQMap4<vc,vc,32> ==\n");
    bench_empty_create_destroy<DwQMap4<vc, vc, 32>>(EMPTY_ITERS);
    for (int n = 1; n <= 5; ++n) bench_insert<DwQMap4<vc, vc, 32>>(SMALL_ITERS, n);
    for (int n = 1; n <= 5; ++n) bench_lookup<DwQMap4<vc, vc, 32>>(SMALL_ITERS, n, n);
    for (int n : {1,2,4,8,16,28}) bench_lookup_miss<DwQMap4<vc, vc, 32>>(SMALL_ITERS, n, 5);

    std::printf("== DwQMapLazyC<vc,vc,8> ==\n");
    bench_empty_create_destroy<DwQMapLazyC<vc, vc, 8>>(EMPTY_ITERS);
    for (int n = 1; n <= 5; ++n) bench_insert<DwQMapLazyC<vc, vc, 8>>(SMALL_ITERS, n);
    for (int n = 1; n <= 5; ++n) bench_lookup<DwQMapLazyC<vc, vc, 8>>(SMALL_ITERS, n, n);
    for (int n : {1,2,4,7}) bench_lookup_miss<DwQMapLazyC<vc, vc, 8>>(SMALL_ITERS, n, 5);

    std::printf("== DwQMapLazyC<vc,vc,32> ==\n");
    bench_empty_create_destroy<DwQMapLazyC<vc, vc, 32>>(EMPTY_ITERS);
    for (int n = 1; n <= 5; ++n) bench_insert<DwQMapLazyC<vc, vc, 32>>(SMALL_ITERS, n);
    for (int n = 1; n <= 5; ++n) bench_lookup<DwQMapLazyC<vc, vc, 32>>(SMALL_ITERS, n, n);
    for (int n : {1,2,4,8,16,28}) bench_lookup_miss<DwQMapLazyC<vc, vc, 32>>(SMALL_ITERS, n, 5);

    return 0;
}
