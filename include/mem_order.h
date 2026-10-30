#pragma once

#include <atomic>

// Building with SEQ_CST=ON (ablation) turns every order below into seq_cst.
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
