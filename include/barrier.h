#pragma once

#include <atomic>
#include <barrier>
#include <memory>
#ifdef __linux__
#include <pthread.h>
#endif

#include "backoff.h"
#include "mem_order.h"

struct alignas(CACHE_LINE) PaddedBool {
    bool value = true;
};

// The last arrival resets count before it flips sense, so no thread can lap the others.
template <bool Backoff>
class alignas(CACHE_LINE > 64 ? CACHE_LINE : 64) SenseReversingBarrierT {  // outer alignment kept for the nopad ablation
    const int N;
    alignas(CACHE_LINE) std::atomic<int> count{0};
    alignas(CACHE_LINE) std::atomic<bool> sense{true};
    std::unique_ptr<PaddedBool[]> lsense;

public:
    explicit SenseReversingBarrierT(int n) : N(n), lsense(new PaddedBool[n]) {}

    void arrive_and_wait(int self) {
        bool my_sense = lsense[self].value = !lsense[self].value;
        if (count.fetch_add(1, mo_acq_rel) == N - 1) {
            count.store(0, mo_relaxed);
            sense.store(my_sense, mo_release);
        } else {
            ExpBackoff backoff;
            while (sense.load(mo_acquire) != my_sense) {
                if constexpr (Backoff) backoff.pause(); else cpu_relax();
            }
        }
    }
};
using SenseReversingBarrier = SenseReversingBarrierT<false>;
using SenseReversingBarrierBO = SenseReversingBarrierT<true>;

// pthread_barrier_t exists on Linux only.
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
