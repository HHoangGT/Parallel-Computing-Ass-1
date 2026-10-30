#include <cstdio>

#include "common/common.h"
#include "locks.h"

template <class Lock>
double run_once(Lock& lock, int threads, long iters, long outside, int n) {
    return run_threads_timed(threads, [&](int) {
        for (long i = 0; i < iters; ++i) {
            lock.acquire();
            consume(is_prime(n));
            lock.release();
            for (long j = 0; j < outside; ++j) consume(is_prime(n));
        }
    });
}

int main(int argc, char** argv) {
    Args args(argc, argv);
    std::string name = args.get("lock", "tas");
    int threads = (int)args.geti("threads", hw_threads());
    long iters = args.geti("iters", 2000);
    long outside = args.geti("outside", 0);
    int n = (int)args.geti("n", 10007);
    int reps = (int)args.geti("reps", 3);
    BackoffConfig::min_delay = (unsigned)args.geti("min_delay", BackoffConfig::min_delay);
    BackoffConfig::max_delay = (unsigned)args.geti("max_delay", BackoffConfig::max_delay);
    BackoffConfig::use_sleep = args.geti("sleep", 0) != 0;

    if (args.geti("header", 0)) std::printf("lock,threads,iters,outside,n,rep,seconds\n");
    start_watchdog(args.geti("timeout", 60),
                   name + "," + std::to_string(threads) + "," + std::to_string(iters) + "," +
                       std::to_string(outside) + "," + std::to_string(n) + ",-1,TIMEOUT");
    bool ok = with_lock(name, [&](auto& lock) {
        run_once(lock, threads, iters / 10 + 1, outside, n);
        for (int rep = 0; rep < reps; ++rep) {
            double seconds = run_once(lock, threads, iters, outside, n);
            std::printf("%s,%d,%ld,%ld,%d,%d,%.6f\n", name.c_str(), threads, iters, outside, n, rep, seconds);
            std::fflush(stdout);
        }
    });
    if (!ok) {
        std::fprintf(stderr, "unknown lock '%s'\n", name.c_str());
        return 1;
    }
}
