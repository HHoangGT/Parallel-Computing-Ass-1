#include "common/common.h"

#include <cstdio>
#include <cstdlib>

int is_prime(int p) {
    for (int d = 2; d < p; d = d + 1)
        if (p % d == 0)
            return 0;
    return 1;
}

static volatile int g_sink;
void consume(int x) { g_sink = x; }

double Timer::seconds() const {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start_).count();
}

Args::Args(int argc, char** argv) {
    for (int i = 1; i + 1 < argc; i += 2) {
        const char* key = argv[i];
        while (*key == '-') ++key;
        values_[key] = argv[i + 1];
    }
}

std::string Args::get(const std::string& key, const std::string& default_value) const {
    auto it = values_.find(key);
    return it == values_.end() ? default_value : it->second;
}

long Args::geti(const std::string& key, long default_value) const {
    auto it = values_.find(key);
    return it == values_.end() ? default_value : std::atol(it->second.c_str());
}

unsigned hw_threads() {
    unsigned n = std::thread::hardware_concurrency();
    return n ? n : 4;
}

void start_watchdog(double seconds, std::string message) {
    if (seconds <= 0) return;
    std::thread([seconds, message] {
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
        if (!message.empty()) {
            std::printf("%s\n", message.c_str());
            std::fflush(stdout);
        }
        std::_Exit(2);
    }).detach();
}
