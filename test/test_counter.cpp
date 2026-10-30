#include <algorithm>
#include <atomic>
#include <cstdio>
#include <string>
#include <vector>

#include "common/common.h"
#include "counter.h"

#if defined(__SANITIZE_THREAD__)
#define UNDER_TSAN 1
#elif defined(__has_feature)
#if __has_feature(thread_sanitizer)
#define UNDER_TSAN 1
#endif
#endif
#ifndef UNDER_TSAN
#define UNDER_TSAN 0
#endif

class BrokenCounter {
    long ctr_ = 0;

public:
    long get_next_value() { return ctr_++; }
    long get() const { return ctr_; }
};

template <class Counter>
bool check(Counter& counter, int threads, long per, bool unique) {
    std::vector<std::vector<long>> values(threads);
    std::atomic<int> waiting{threads};
    run_threads(threads, [&](int tid) {
        std::vector<long> my_values;
        my_values.reserve(per);

        waiting.fetch_sub(1, std::memory_order_relaxed);
        while (waiting.load(std::memory_order_relaxed) > 0) std::this_thread::yield();
        for (long i = 0; i < per; ++i) my_values.push_back(counter.get_next_value());
        values[tid] = std::move(my_values);
    });
    long expected = threads * per;
    bool ok = counter.get() == expected;
    if (!ok)
        std::printf("    FAIL threads=%d: get()=%ld expected=%ld (lost updates)\n", threads, counter.get(),
                    expected);
    if (unique) {
        std::vector<long> all_values;
        all_values.reserve(expected);
        for (auto& thread_values : values)
            all_values.insert(all_values.end(), thread_values.begin(), thread_values.end());
        std::sort(all_values.begin(), all_values.end());
        long duplicates = 0, first_bad = -1;
        for (long i = 0; i < expected; ++i) {
            if (i > 0 && all_values[i] == all_values[i - 1]) ++duplicates;
            if (first_bad < 0 && all_values[i] != i) first_bad = i;
        }
        if (first_bad >= 0) {
            ok = false;
            std::printf("    FAIL threads=%d: %ld duplicate values, first bad index %ld (value %ld)\n",
                        threads, duplicates, first_bad, all_values[first_bad]);
        }
    }
    return ok;
}

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    Args args(argc, argv);
    long per = args.geti("per", 100000);
    int maxt = (int)args.geti("maxt", 0);
    int cores = (int)hw_threads();
    int thread_counts[] = {1, 2, 4, cores, 2 * cores};
    auto calls = [&](int threads) { return threads > cores ? std::max(per / 1000, 1L) : per; };
    int failures = 0;
    for (auto& name : counter_names()) {
        bool unique = name != "sharded";
        bool passed = true;
        for (int threads : thread_counts) {
            if (maxt && threads > maxt) continue;
            with_counter(name, threads, [&](auto& counter) {

                passed &= check(counter, threads, calls(threads), unique);
            });
        }
        std::printf("[%s] %s%s\n", passed ? " OK " : "FAIL", name.c_str(),
                    unique ? "" : " (total only: per-slot values repeat by design)");
        failures += !passed;
    }
#if UNDER_TSAN
    std::printf("negative test skipped under ThreadSanitizer (it would report BrokenCounter's race)\n");
#else
    std::printf("negative test: BrokenCounter (plain long) must fail the checks:\n");
    std::string caught;
    for (int threads : thread_counts) {
        if (maxt && threads > maxt) continue;
        BrokenCounter counter;
        if (!check(counter, threads, calls(threads), true) && threads >= 2)
            caught += (caught.empty() ? "" : ",") + std::to_string(threads);
    }
    if (caught.empty()) {
        std::printf("[FAIL] broken: lost updates / duplicates were NOT detected\n");
        failures++;
    } else {
        std::printf("[ OK ] broken (caught at threads=%s)\n", caught.c_str());
    }
#endif
    std::printf("%s\n", failures ? "SOME COUNTER TESTS FAILED" : "all counter tests passed");
    return failures ? 1 : 0;
}
