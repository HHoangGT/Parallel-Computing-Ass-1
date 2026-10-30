#include <cstdio>
#include <vector>

#include "common/common.h"
#include "mpi_common.h"

enum { COUNT = 0, SENSE = 1, SLOTS = 2 };
static int rank, nprocs;

template <bool Backoff>
struct SenseBarrierMPI {
    MPI_Win win;
    int lsense = 1;
    void arrive_and_wait() {
        lsense = !lsense;
        if (rma_fetch_add<int>(1, 0, COUNT, win) == nprocs - 1) {
            rma_store<int>(0, 0, COUNT, win);
            rma_store<int>(lsense, 0, SENSE, win);
        } else {
            MpiBackoff backoff;
            while (rma_load<int>(0, SENSE, win) != lsense)
                if (Backoff) backoff.pause();
        }
    }
};
struct MPIBarrier {
    void arrive_and_wait() { MPI_Barrier(MPI_COMM_WORLD); }
};

template <class Barrier>
double run(Barrier& barrier, long episodes, bool check, MPI_Win win, long& errors) {
    std::vector<int> slots(nprocs);
    double start = MPI_Wtime();
    for (long episode = 0; episode < episodes; ++episode) {
        if (check) rma_store<int>((int)episode, 0, SLOTS + rank, win);
        barrier.arrive_and_wait();
        if (check) {
            MPI_Get(slots.data(), nprocs, MPI_INT, 0, SLOTS, nprocs, MPI_INT, win);
            MPI_Win_flush(0, win);
            for (int j = 0; j < nprocs; ++j) errors += slots[j] != episode;
            barrier.arrive_and_wait();
        }
    }
    return MPI_Wtime() - start;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    Args args(argc, argv);
    std::string variant = args.get("variant", "sr");
    long episodes = args.geti("episodes", 5000);
    bool check = args.geti("check", 0) != 0;

    std::string row = variant + "," + std::to_string(nprocs) + "," + std::to_string(episodes) + ",TIMEOUT,0,timeout";
    start_watchdog(args.geti("timeout", 60), rank == 0 ? row : "");

    int* base;
    MPI_Win win;
    MPI_Win_allocate((SLOTS + nprocs) * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &base, &win);
    base[COUNT] = 0; base[SENSE] = 1;
    for (int i = 0; i < nprocs; ++i) base[SLOTS + i] = -1;
    MPI_Win_lock_all(0, win);
    MPI_Barrier(MPI_COMM_WORLD);

    long errors = 0;
    double seconds;
    if (variant == "sr") {
        SenseBarrierMPI<false> barrier{win};
        seconds = run(barrier, episodes, check, win, errors);
    } else if (variant == "sr_bo") {
        SenseBarrierMPI<true> barrier{win};
        seconds = run(barrier, episodes, check, win, errors);
    } else {
        MPIBarrier barrier;
        seconds = run(barrier, episodes, check, win, errors);
    }

    double max_seconds;
    long all_errors;
    MPI_Reduce(&seconds, &max_seconds, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&errors, &all_errors, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        if (args.geti("header", 0)) std::printf("barrier,procs,episodes,seconds,episodes_per_sec,correct\n");
        std::printf("%s,%d,%ld,%.6f,%.1f,%s\n", variant.c_str(), nprocs, episodes, max_seconds, episodes / max_seconds,
                    !check ? "unchecked" : all_errors == 0 ? "yes" : "NO");
    }
    MPI_Win_unlock_all(win);
    MPI_Win_free(&win);
    MPI_Finalize();
}
