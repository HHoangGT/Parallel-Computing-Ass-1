#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

#if defined(_MSC_VER)
#include <intrin.h>
#if defined(_M_ARM64)
inline void cpu_relax() { __yield(); }
#else
inline void cpu_relax() { _mm_pause(); }
#endif
#elif defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
inline void cpu_relax() { _mm_pause(); }
#elif defined(__aarch64__) || defined(__arm__)
inline void cpu_relax() { asm volatile("yield" ::: "memory"); }
#else
inline void cpu_relax() {}
#endif

struct BackoffConfig {
    static inline unsigned min_delay = 4;
    static inline unsigned max_delay = 1024;
    static inline bool use_sleep = false;
};

class ExpBackoff {
    unsigned delay_ = BackoffConfig::min_delay;
    uint32_t rng_ = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(this)) | 1u;

public:
    void pause() {
        rng_ ^= rng_ << 13;
        rng_ ^= rng_ >> 17;
        rng_ ^= rng_ << 5;
        unsigned wait = delay_ / 2 + rng_ % (delay_ / 2 + 1);
        if (BackoffConfig::use_sleep) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(wait));
        } else {
            for (unsigned i = 0; i < wait; ++i) cpu_relax();
        }
        if (delay_ < BackoffConfig::max_delay) delay_ *= 2;
    }
};
