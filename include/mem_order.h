#pragma once

#include <atomic>

// Memory orders used by the locks, barrier and counters. The SEQ_CST CMake option (ablation)
// defines ASM1_SEQ_CST, which turns every one of them into memory_order_seq_cst, so the cost of
// sequential consistency can be measured against the acquire/release orders of the slides.
#ifdef ASM1_SEQ_CST
inline constexpr std::memory_order mo_relaxed = std::memory_order_seq_cst;
inline constexpr std::memory_order mo_acquire = std::memory_order_seq_cst;
inline constexpr std::memory_order mo_release = std::memory_order_seq_cst;
inline constexpr std::memory_order mo_acq_rel = std::memory_order_seq_cst;
#else
inline constexpr std::memory_order mo_relaxed = std::memory_order_relaxed;
inline constexpr std::memory_order mo_acquire = std::memory_order_acquire;
inline constexpr std::memory_order mo_release = std::memory_order_release;
inline constexpr std::memory_order mo_acq_rel = std::memory_order_acq_rel;
#endif
