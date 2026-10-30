#pragma once

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

template <class F>
void run_threads(int n, F body) {
    std::vector<std::thread> workers;
    workers.reserve(n);
    for (int tid = 0; tid < n; ++tid) workers.emplace_back(body, tid);
    for (auto& worker : workers) worker.join();
}
