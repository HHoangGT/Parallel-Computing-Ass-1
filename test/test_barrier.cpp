#include <atomic>
#include <cstdio>
#include <memory>

#include "barrier.h"
#include "common/common.h"

struct alignas(CACHE_LINE) Slot {
    std::atomic<long> value{-1};
};

template <class Barrier>
bool check(int threads, long episodes) {
    Barrier barrier(threads);
    std::unique_ptr<Slot[]> slot(new Slot[threads]);
    std::atomic<long> errors{0};
    run_threads(threads, [&](int tid) {
        for (long episode = 0; episode < episodes; ++episode) {
            slot[tid].value.store(episode, std::memory_order_relaxed);
            barrier.arrive_and_wait(tid);
            for (int j = 0; j < threads; ++j)
                if (slot[j].value.load(std::memory_order_relaxed) != episode) errors++;
            barrier.arrive_and_wait(tid);
        }
    });
    if (errors) std::printf("    FAIL threads=%d: %ld wrong observations\n", threads, errors.load());
    return errors == 0;
}

template <class Barrier>
bool check_all(const char* name, long episodes, int maxt) {
    int cores = (int)hw_threads();
    bool ok = true;
    for (int threads : {1, 2, 4, cores, 2 * cores}) {
        if (maxt && threads > maxt) continue;

        ok &= check<Barrier>(threads, threads > cores ? episodes / 100 : episodes);
    }
    std::printf("[%s] %s\n", ok ? " OK " : "FAIL", name);
    return ok;
}

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    Args args(argc, argv);
    int maxt = (int)args.geti("maxt", 0);
    long episodes = args.geti("episodes", 20000);
    bool ok = true;
    ok &= check_all<SenseReversingBarrier>("sr", episodes, maxt);
    ok &= check_all<SenseReversingBarrierBO>("sr_bo", episodes, maxt);
    ok &= check_all<StdBarrier>("std", episodes, maxt);
#ifdef __linux__
    ok &= check_all<PthreadBarrier>("pthread", episodes, maxt);
#endif
    std::printf("%s\n", ok ? "all barrier tests passed" : "SOME BARRIER TESTS FAILED");
    return ok ? 0 : 1;
}
