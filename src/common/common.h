#pragma once

#include <atomic>
#include <chrono>
#include <map>
#include <string>
#include <thread>
#include <vector>

int is_prime(int p);

void consume(int x);

class Timer {
    std::chrono::steady_clock::time_point start_ = std::chrono::steady_clock::now();

public:
    double seconds() const;
};

class Args {
    std::map<std::string, std::string> values_;

public:
    Args(int argc, char** argv);
    std::string get(const std::string& key, const std::string& default_value) const;
    long geti(const std::string& key, long default_value) const;
};

unsigned hw_threads();

void start_watchdog(double seconds, std::string message);

// Times body(tid) on n threads from a common start signal; thread creation is not timed.
template <class F>
double run_threads_timed(int n, F body) {
    std::atomic<int> ready{0};
    std::atomic<bool> go{false};
    std::vector<std::thread> workers;
    workers.reserve(n);
    for (int tid = 0; tid < n; ++tid)
        workers.emplace_back([&, tid] {
            ready.fetch_add(1, std::memory_order_relaxed);
            while (!go.load(std::memory_order_acquire)) std::this_thread::yield();
            body(tid);
        });
    while (ready.load(std::memory_order_relaxed) < n) std::this_thread::yield();
    Timer timer;
    go.store(true, std::memory_order_release);
    for (auto& worker : workers) worker.join();
    return timer.seconds();
}

template <class F>
void run_threads(int n, F body) {
    std::vector<std::thread> workers;
    workers.reserve(n);
    for (int tid = 0; tid < n; ++tid) workers.emplace_back(body, tid);
    for (auto& worker : workers) worker.join();
}
