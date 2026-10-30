#pragma once

#include <atomic>
#include <barrier>
#include <memory>
#ifdef __linux__
#include <pthread.h>
#endif

#include "backoff.h"

struct alignas(CACHE_LINE) PaddedBool {
    bool value = true;
};

// Sense-reversing barrier (slide 04 p14): toggle local sense, FAI(count); the last arrival resets count
// first, then flips sense (release). lsense is one padded slot per thread to avoid false sharing.
template <bool Backoff>
class SenseReversingBarrierT {
    const int N;
    alignas(CACHE_LINE) std::atomic<int> count{0};
    alignas(CACHE_LINE) std::atomic<bool> sense{true};
    std::unique_ptr<PaddedBool[]> lsense;

public:
    explicit SenseReversingBarrierT(int n) : N(n), lsense(new PaddedBool[n]) {}

    void arrive_and_wait(int self) {
        bool my_sense = lsense[self].value = !lsense[self].value;
        if (count.fetch_add(1, std::memory_order_acq_rel) == N - 1) {
            count.store(0, std::memory_order_relaxed);
            sense.store(my_sense, std::memory_order_release);
        } else {
            ExpBackoff backoff;
            while (sense.load(std::memory_order_acquire) != my_sense) {
                if constexpr (Backoff) backoff.pause(); else cpu_relax();
            }
        }
    }
};
using SenseReversingBarrier = SenseReversingBarrierT<false>;
using SenseReversingBarrierBO = SenseReversingBarrierT<true>;

// Baselines: C++20 std::barrier (04 p28-31); pthread_barrier_t exists on Linux only (not on macOS).
class StdBarrier {
    std::barrier<> barrier_;

public:
    explicit StdBarrier(int n) : barrier_(n) {}
    void arrive_and_wait(int) { barrier_.arrive_and_wait(); }
};

#ifdef __linux__
class PthreadBarrier {
    pthread_barrier_t barrier_;

public:
    explicit PthreadBarrier(int n) { pthread_barrier_init(&barrier_, nullptr, n); }
    ~PthreadBarrier() { pthread_barrier_destroy(&barrier_); }
    void arrive_and_wait(int) { pthread_barrier_wait(&barrier_); }
};
#endif
