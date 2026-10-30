#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#ifndef _WIN32
#include <pthread.h>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif

#include "backoff.h"

class TASLock {
    alignas(CACHE_LINE) std::atomic<bool> locked{false};

public:
    void acquire() {
        while (locked.exchange(true, std::memory_order_acquire)) {}
    }
    void release() { locked.store(false, std::memory_order_release); }
};

class TTASLock {
    alignas(CACHE_LINE) std::atomic<bool> locked{false};

public:
    void acquire() {
        while (locked.exchange(true, std::memory_order_acquire))
            while (locked.load(std::memory_order_relaxed)) {}
    }
    void release() { locked.store(false, std::memory_order_release); }
};

class TASBackoffLock {
    alignas(CACHE_LINE) std::atomic<bool> locked{false};

public:
    void acquire() {
        ExpBackoff backoff;
        while (locked.exchange(true, std::memory_order_acquire)) backoff.pause();
    }
    void release() { locked.store(false, std::memory_order_release); }
};

class TTASBackoffLock {
    alignas(CACHE_LINE) std::atomic<bool> locked{false};

public:
    void acquire() {
        ExpBackoff backoff;
        while (true) {
            while (locked.load(std::memory_order_relaxed)) cpu_relax();
            if (!locked.exchange(true, std::memory_order_acquire)) return;
            backoff.pause();
        }
    }
    void release() { locked.store(false, std::memory_order_release); }
};

class TicketLock {
    alignas(CACHE_LINE) std::atomic<unsigned> next_ticket{0};
    alignas(CACHE_LINE) std::atomic<unsigned> now_serving{0};

public:
    void acquire() {
        unsigned my_ticket = next_ticket.fetch_add(1, std::memory_order_relaxed);
        while (now_serving.load(std::memory_order_acquire) != my_ticket) cpu_relax();
    }
    void release() {
        now_serving.store(now_serving.load(std::memory_order_relaxed) + 1, std::memory_order_release);
    }
};

struct alignas(CACHE_LINE) MCSNode {
    std::atomic<MCSNode*> next{nullptr};
    std::atomic<bool> wait{false};
};

class MCSLock {
    alignas(CACHE_LINE) std::atomic<MCSNode*> tail{nullptr};

    static MCSNode& my_node() {
        static thread_local MCSNode node;
        return node;
    }

public:
    void acquire(MCSNode* p) {
        p->next.store(nullptr, std::memory_order_relaxed);
        p->wait.store(true, std::memory_order_relaxed);
        MCSNode* prev = tail.exchange(p, std::memory_order_acq_rel);
        if (prev != nullptr) {
            prev->next.store(p, std::memory_order_release);
            while (p->wait.load(std::memory_order_acquire)) cpu_relax();
        }
    }
    void release(MCSNode* p) {
        MCSNode* succ = p->next.load(std::memory_order_acquire);
        if (succ == nullptr) {
            MCSNode* expected = p;
            if (tail.compare_exchange_strong(expected, nullptr, std::memory_order_release,
                                             std::memory_order_relaxed))
                return;

            while ((succ = p->next.load(std::memory_order_acquire)) == nullptr) cpu_relax();
        }
        succ->wait.store(false, std::memory_order_release);
    }
    void acquire() { acquire(&my_node()); }
    void release() { release(&my_node()); }
};

class StdMutexLock {
    std::mutex mutex_;

public:
    void acquire() { mutex_.lock(); }
    void release() { mutex_.unlock(); }
};

#ifndef _WIN32
class PthreadLock {
    pthread_mutex_t mutex_;

public:
    PthreadLock() { pthread_mutex_init(&mutex_, nullptr); }
    ~PthreadLock() { pthread_mutex_destroy(&mutex_); }
    void acquire() { pthread_mutex_lock(&mutex_); }
    void release() { pthread_mutex_unlock(&mutex_); }
};
#endif

#ifdef _OPENMP
class OmpLock {
    omp_lock_t lock_;

public:
    OmpLock() { omp_init_lock(&lock_); }
    ~OmpLock() { omp_destroy_lock(&lock_); }
    void acquire() { omp_set_lock(&lock_); }
    void release() { omp_unset_lock(&lock_); }
};
#endif

inline std::vector<std::string> lock_names() {
    std::vector<std::string> names = {"tas", "ttas", "tas_bo", "ttas_bo", "ticket", "mcs", "std_mutex"};
#ifndef _WIN32
    names.push_back("pthread");
#endif
#ifdef _OPENMP
    names.push_back("omp");
#endif
    return names;
}

template <class F>
bool with_lock(const std::string& name, F&& f) {
    if (name == "tas")       { TASLock lock;         f(lock); return true; }
    if (name == "ttas")      { TTASLock lock;        f(lock); return true; }
    if (name == "tas_bo")    { TASBackoffLock lock;  f(lock); return true; }
    if (name == "ttas_bo")   { TTASBackoffLock lock; f(lock); return true; }
    if (name == "ticket")    { TicketLock lock;      f(lock); return true; }
    if (name == "mcs")       { MCSLock lock;         f(lock); return true; }
    if (name == "std_mutex") { StdMutexLock lock;    f(lock); return true; }
#ifndef _WIN32
    if (name == "pthread")   { PthreadLock lock;     f(lock); return true; }
#endif
#ifdef _OPENMP
    if (name == "omp")       { OmpLock lock;         f(lock); return true; }
#endif
    return false;
}
