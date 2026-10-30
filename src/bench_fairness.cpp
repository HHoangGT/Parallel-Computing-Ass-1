#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#include "common/common.h"
#include "counter.h"

int main(int argc, char** argv) {
    Args args(argc, argv);
    std::string name = args.get("counter", "faa");
    int threads = (int)args.geti("threads", hw_threads());
    double seconds = std::atof(args.get("seconds", "1").c_str());
    BackoffConfig::min_delay = (unsigned)args.geti("min_delay", BackoffConfig::min_delay);
    BackoffConfig::max_delay = (unsigned)args.geti("max_delay", BackoffConfig::max_delay);
    BackoffConfig::use_sleep = args.geti("sleep", 0) != 0;

    char seconds_text[32];
    std::snprintf(seconds_text, sizeof seconds_text, "%g", seconds);
    if (args.geti("header", 0)) std::printf("counter,threads,seconds,total_ops,min_ops,max_ops,jain\n");
    start_watchdog(args.geti("timeout", 60),
                   name + "," + std::to_string(threads) + "," + seconds_text + ",TIMEOUT,0,0,0");

    std::vector<long long> ops(threads);
    bool ok = with_counter(name, threads, [&](auto& counter) {
        std::atomic<int> ready{0};
        std::atomic<bool> go{false}, stop{false};
        std::vector<std::thread> workers;
        for (int tid = 0; tid < threads; ++tid)
            workers.emplace_back([&, tid] {
                ready.fetch_add(1, std::memory_order_relaxed);
                while (!go.load(std::memory_order_relaxed)) {}
                long long count = 0;
                unsigned long long sum = 0;
                while (!stop.load(std::memory_order_relaxed)) {
                    sum += counter.get_next_value();
                    ++count;
                }
                ops[tid] = count;
                consume((int)sum);
            });

        while (ready.load(std::memory_order_relaxed) < threads) std::this_thread::yield();
        go.store(true, std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
        stop.store(true, std::memory_order_relaxed);
        for (auto& worker : workers) worker.join();
    });
    if (!ok) {
        std::fprintf(stderr, "unknown counter '%s'\n", name.c_str());
        return 1;
    }

    long long total = 0, min_ops = ops[0], max_ops = ops[0];
    double sum_squares = 0;
    for (long long count : ops) {
        total += count;
        min_ops = std::min(min_ops, count);
        max_ops = std::max(max_ops, count);
        sum_squares += (double)count * count;
    }
    double jain = sum_squares > 0 ? (double)total * total / (threads * sum_squares) : 0;
    std::printf("%s,%d,%s,%lld,%lld,%lld,%.4f\n", name.c_str(), threads, seconds_text, total, min_ops,
                max_ops, jain);
    std::fflush(stdout);
}
