#include <cstdio>
#include <string>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "barrier.h"
#include "common/common.h"

template <class Barrier>
double run_once(int threads, long episodes) {
    Barrier barrier(threads);
    return run_threads_timed(threads, [&](int tid) {
        for (long episode = 0; episode < episodes; ++episode) barrier.arrive_and_wait(tid);
    });
}

#ifdef _OPENMP
double run_omp(int threads, long episodes) {
    // Time inside the parallel region (after one untimed barrier) so OpenMP's thread start-up is
    // excluded, matching run_threads_timed for the other barriers.
    double seconds = 0;
#pragma omp parallel num_threads(threads)
    {
#pragma omp barrier
        Timer timer;
        for (long episode = 0; episode < episodes; ++episode) {
#pragma omp barrier
        }
#pragma omp master
        seconds = timer.seconds();
    }
    return seconds;
}
#endif

double run_by_name(const std::string& name, int threads, long episodes) {
    if (name == "sr") return run_once<SenseReversingBarrier>(threads, episodes);
    if (name == "sr_bo") return run_once<SenseReversingBarrierBO>(threads, episodes);
    if (name == "std") return run_once<StdBarrier>(threads, episodes);
#ifdef __linux__
    if (name == "pthread") return run_once<PthreadBarrier>(threads, episodes);
#endif
#ifdef _OPENMP
    if (name == "omp") return run_omp(threads, episodes);
#endif
    return -1;
}

int main(int argc, char** argv) {
    Args args(argc, argv);
    std::string name = args.get("barrier", "sr");
    int threads = (int)args.geti("threads", hw_threads());
    long episodes = args.geti("episodes", 100000);
    int reps = (int)args.geti("reps", 3);
    BackoffConfig::min_delay = (unsigned)args.geti("min_delay", BackoffConfig::min_delay);
    BackoffConfig::max_delay = (unsigned)args.geti("max_delay", BackoffConfig::max_delay);
    BackoffConfig::use_sleep = args.geti("sleep", 0) != 0;

    if (args.geti("header", 0)) std::printf("barrier,threads,episodes,rep,seconds,episodes_per_sec\n");
    start_watchdog(args.geti("timeout", 60),
                   name + "," + std::to_string(threads) + "," + std::to_string(episodes) + ",-1,TIMEOUT,0");
    if (run_by_name(name, threads, episodes / 10 + 1) < 0) {
        std::fprintf(stderr, "unknown/unsupported barrier '%s'\n", name.c_str());
        return 1;
    }
    for (int rep = 0; rep < reps; ++rep) {
        double seconds = run_by_name(name, threads, episodes);
        std::printf("%s,%d,%ld,%d,%.6f,%.1f\n", name.c_str(), threads, episodes, rep, seconds,
                    episodes / seconds);
        std::fflush(stdout);
    }
}
