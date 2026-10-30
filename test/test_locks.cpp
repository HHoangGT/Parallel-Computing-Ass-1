#include <atomic>
#include <cstdio>

#include "common/common.h"
#include "locks.h"

template <class Lock>
bool check(Lock& lock, int threads, long iters) {
    long plain_counter = 0;
    std::atomic<int> in_cs{0};
    std::atomic<long> violations{0};
    run_threads(threads, [&](int) {
        for (long i = 0; i < iters; ++i) {
            lock.acquire();
            if (in_cs.fetch_add(1, std::memory_order_relaxed) != 0) violations++;
            long value = plain_counter;
            if ((i & 63) == 0) std::this_thread::yield();
            plain_counter = value + 1;
            in_cs.fetch_sub(1, std::memory_order_relaxed);
            lock.release();
        }
    });
    long expected = threads * iters;
    bool ok = plain_counter == expected && violations == 0;
    if (!ok)
        std::printf("    FAIL threads=%d: counter=%ld expected=%ld violations=%ld\n", threads, plain_counter,
                    expected, violations.load());
    return ok;
}

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    Args args(argc, argv);
    int maxt = (int)args.geti("maxt", 0);
    long iters = args.geti("iters", 20000);
    int cores = (int)hw_threads();
    int thread_counts[] = {1, 2, 4, cores, 2 * cores};
    int failures = 0;
    for (auto& name : lock_names()) {
        bool all = true;
        for (int threads : thread_counts) {
            if (maxt && threads > maxt) continue;
            with_lock(name, [&](auto& lock) {

                all &= check(lock, threads, threads > cores ? iters / 100 : iters);
            });
        }
        std::printf("[%s] %s\n", all ? " OK " : "FAIL", name.c_str());
        failures += !all;
    }
    std::printf("%s\n", failures ? "SOME LOCK TESTS FAILED" : "all lock tests passed");
    return failures ? 1 : 0;
}
