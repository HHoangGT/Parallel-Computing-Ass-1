#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "backoff.h"
#include "locks.h"

template <class Lock>
class LockCounter {
    mutable Lock lock_;
    alignas(CACHE_LINE) long ctr_ = 0;

public:
    long get_next_value() {
        lock_.acquire();
        long value = ctr_++;
        lock_.release();
        return value;
    }
    long get() const {
        lock_.acquire();
        long value = ctr_;
        lock_.release();
        return value;
    }
};

class CasCounter {
    alignas(CACHE_LINE) std::atomic<long> ctr_{0};

public:
    long get_next_value() {
        long old = ctr_.load(std::memory_order_relaxed);
        while (!ctr_.compare_exchange_weak(old, old + 1, std::memory_order_relaxed,
                                           std::memory_order_relaxed)) {}
        return old;
    }
    long get() const { return ctr_.load(std::memory_order_relaxed); }
};

class CasBackoffCounter {
    alignas(CACHE_LINE) std::atomic<long> ctr_{0};

public:
    long get_next_value() {
        ExpBackoff backoff;
        long old = ctr_.load(std::memory_order_relaxed);
        while (!ctr_.compare_exchange_weak(old, old + 1, std::memory_order_relaxed,
                                           std::memory_order_relaxed))
            backoff.pause();
        return old;
    }
    long get() const { return ctr_.load(std::memory_order_relaxed); }
};

class FaaCounter {
    alignas(CACHE_LINE) std::atomic<long> ctr_{0};

public:
    long get_next_value() { return ctr_.fetch_add(1, std::memory_order_relaxed); }
    long get() const { return ctr_.load(std::memory_order_relaxed); }
};

class ShardedCounter {
    struct alignas(CACHE_LINE) Slot {
        std::atomic<long> value{0};
    };
    std::unique_ptr<Slot[]> slots_;
    int nshards_;

    static int my_slot(int nshards) {
        static std::atomic<int> next_id{0};
        static thread_local int id = next_id.fetch_add(1, std::memory_order_relaxed);
        return id % nshards;
    }

public:
    explicit ShardedCounter(int nshards) : slots_(new Slot[nshards]), nshards_(nshards) {}

    long get_next_value() {
        std::atomic<long>& slot = slots_[my_slot(nshards_)].value;
        long old = slot.load(std::memory_order_relaxed);
        slot.store(old + 1, std::memory_order_relaxed);
        return old;
    }
    long get() const {
        long sum = 0;
        for (int i = 0; i < nshards_; ++i) sum += slots_[i].value.load(std::memory_order_relaxed);
        return sum;
    }
};

inline std::vector<std::string> counter_names() {
    std::vector<std::string> names = {"lock_tas", "lock_ttas", "lock_tas_bo", "lock_ttas_bo",
                                      "lock_ticket", "lock_mcs", "lock_mutex"};
#ifndef _WIN32
    names.push_back("lock_pthread");
#endif
#ifdef _OPENMP
    names.push_back("lock_omp");
#endif
    names.insert(names.end(), {"cas", "cas_bo", "faa", "sharded"});
    return names;
}

template <class F>
bool with_counter(const std::string& name, int nthreads, F&& f) {
    if (name == "lock_tas")     { LockCounter<TASLock> counter;         f(counter); return true; }
    if (name == "lock_ttas")    { LockCounter<TTASLock> counter;        f(counter); return true; }
    if (name == "lock_tas_bo")  { LockCounter<TASBackoffLock> counter;  f(counter); return true; }
    if (name == "lock_ttas_bo") { LockCounter<TTASBackoffLock> counter; f(counter); return true; }
    if (name == "lock_ticket")  { LockCounter<TicketLock> counter;      f(counter); return true; }
    if (name == "lock_mcs")     { LockCounter<MCSLock> counter;         f(counter); return true; }
    if (name == "lock_mutex")   { LockCounter<StdMutexLock> counter;    f(counter); return true; }
#ifndef _WIN32
    if (name == "lock_pthread") { LockCounter<PthreadLock> counter;     f(counter); return true; }
#endif
#ifdef _OPENMP
    if (name == "lock_omp")     { LockCounter<OmpLock> counter;         f(counter); return true; }
#endif
    if (name == "cas")          { CasCounter counter;                   f(counter); return true; }
    if (name == "cas_bo")       { CasBackoffCounter counter;            f(counter); return true; }
    if (name == "faa")          { FaaCounter counter;                   f(counter); return true; }
    if (name == "sharded")      { ShardedCounter counter(nthreads);     f(counter); return true; }
    return false;
}
