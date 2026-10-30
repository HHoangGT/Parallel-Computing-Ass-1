#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#include "common/common.h"
#include "counter.h"

double run_once(const std::string& name, int threads, long total) {
    double seconds = -1;
    with_counter(name, threads, [&](auto& counter) {
        long per = total / threads, extra = total % threads;
        std::atomic<int> ready{0};
        std::atomic<bool> go{false};
        std::vector<std::thread> workers;
        for (int tid = 0; tid < threads; ++tid)
            workers.emplace_back([&, tid] {
                long count = per + (tid < extra ? 1 : 0);
                unsigned long long sum = 0;
                ready.fetch_add(1, std::memory_order_relaxed);
                while (!go.load(std::memory_order_relaxed)) {}
                for (long i = 0; i < count; ++i) sum += counter.get_next_value();
                consume((int)sum);
            });

        while (ready.load(std::memory_order_relaxed) < threads) std::this_thread::yield();
        Timer timer;
        go.store(true, std::memory_order_relaxed);
        for (auto& worker : workers) worker.join();
        seconds = timer.seconds();
        if (counter.get() != total) {
            std::fprintf(stderr, "%s: get() = %ld after %ld increments\n", name.c_str(), counter.get(),
                         total);
            std::exit(1);
        }
    });
    return seconds;
}

int main(int argc, char** argv) {
    Args args(argc, argv);
    std::string name = args.get("counter", "faa");
    int threads = (int)args.geti("threads", hw_threads());
    long total = args.geti("total", 4000000);
    int reps = (int)args.geti("reps", 3);
    BackoffConfig::min_delay = (unsigned)args.geti("min_delay", BackoffConfig::min_delay);
    BackoffConfig::max_delay = (unsigned)args.geti("max_delay", BackoffConfig::max_delay);
    BackoffConfig::use_sleep = args.geti("sleep", 0) != 0;

    if (args.geti("header", 0)) std::printf("counter,threads,total,rep,seconds,mops_per_sec\n");
    start_watchdog(args.geti("timeout", 60),
                   name + "," + std::to_string(threads) + "," + std::to_string(total) + ",-1,TIMEOUT,0");
    if (run_once(name, threads, total) < 0) {
        std::fprintf(stderr, "unknown counter '%s'\n", name.c_str());
        return 1;
    }
    for (int rep = 0; rep < reps; ++rep) {
        double seconds = run_once(name, threads, total);
        std::printf("%s,%d,%ld,%d,%.6f,%.3f\n", name.c_str(), threads, total, rep, seconds,
                    total / seconds / 1e6);
        std::fflush(stdout);
    }
}
