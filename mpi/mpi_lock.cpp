#include <cstdio>

#include "common/common.h"
#include "mpi_common.h"

enum { TAIL = 0, LOCKWORD = 1, NEXT = 2, WAIT = 3, COUNTER = 4, NSLOTS = 5 };
static int rank, nprocs;

struct TASLockMPI {
    MPI_Win win;
    void acquire() {
        MpiBackoff backoff;
        while (rma_cas<int>(0, 1, 0, LOCKWORD, win) != 0) backoff.pause();
    }
    void release() { rma_store<int>(0, 0, LOCKWORD, win); }
};

struct MCSLockMPI {
    MPI_Win win;
    void acquire() {
        rma_store<int>(-1, rank, NEXT, win);
        rma_store<int>(1, rank, WAIT, win);
        int prev = rma_swap<int>(rank, 0, TAIL, win);
        if (prev != -1) {
            rma_store<int>(rank, prev, NEXT, win);
            while (rma_load<int>(rank, WAIT, win) == 1) {}
        }
    }
    void release() {
        int succ = rma_load<int>(rank, NEXT, win);
        if (succ == -1) {
            if (rma_cas<int>(rank, -1, 0, TAIL, win) == rank) return;
            while ((succ = rma_load<int>(rank, NEXT, win)) == -1) {}
        }
        rma_store<int>(0, succ, WAIT, win);
    }
};

template <class Lock>
void critical_section_loop(Lock& lock, long iters, MPI_Win win) {
    for (long i = 0; i < iters; ++i) {
        lock.acquire();
        int value;
        MPI_Get(&value, 1, MPI_INT, 0, COUNTER, 1, MPI_INT, win);
        MPI_Win_flush(0, win);
        value += 1;
        MPI_Put(&value, 1, MPI_INT, 0, COUNTER, 1, MPI_INT, win);
        MPI_Win_flush(0, win);
        lock.release();
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    Args args(argc, argv);
    std::string variant = args.get("variant", "mcs");
    long iters = args.geti("iters", 2000);

    std::string row = variant + "," + std::to_string(nprocs) + "," + std::to_string(iters) + ",TIMEOUT,0,timeout";
    // rank 0 prints the TIMEOUT row and exits first; other ranks only fire 5 s later as a fallback
    start_watchdog(args.geti("timeout", 60) + (rank == 0 ? 0 : 5), rank == 0 ? row : "");

    int* base;
    MPI_Win win;
    MPI_Win_allocate(NSLOTS * sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &base, &win);
    base[TAIL] = -1; base[LOCKWORD] = 0; base[NEXT] = -1; base[WAIT] = 0; base[COUNTER] = 0;
    MPI_Barrier(MPI_COMM_WORLD);

    double start = 0, seconds = 0;
    if (variant == "winlock") {
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        for (long i = 0; i < iters; ++i) {
            int value;
            MPI_Win_lock(MPI_LOCK_EXCLUSIVE, 0, 0, win);
            MPI_Get(&value, 1, MPI_INT, 0, COUNTER, 1, MPI_INT, win);
            MPI_Win_flush(0, win);
            value += 1;
            MPI_Put(&value, 1, MPI_INT, 0, COUNTER, 1, MPI_INT, win);
            MPI_Win_unlock(0, win);
        }
        seconds = MPI_Wtime() - start;
        // Only after every rank has left its MPI_Win_lock(EXCLUSIVE) loop may a rank take the shared
        // lock_all epoch; taking it earlier blocks the slower ranks' exclusive locks forever.
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Win_lock_all(0, win);
    } else {
        MPI_Win_lock_all(0, win);
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();
        if (variant == "tas") { TASLockMPI lock{win}; critical_section_loop(lock, iters, win); }
        else                  { MCSLockMPI lock{win}; critical_section_loop(lock, iters, win); }
        seconds = MPI_Wtime() - start;
    }
    MPI_Barrier(MPI_COMM_WORLD);
    double max_seconds;
    MPI_Reduce(&seconds, &max_seconds, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        int final_value = rma_load<int>(0, COUNTER, win);
        bool ok = final_value == nprocs * iters;
        if (args.geti("header", 0)) std::printf("lock,procs,iters_per_rank,seconds,acq_per_sec,correct\n");
        std::printf("%s,%d,%ld,%.6f,%.1f,%s\n", variant.c_str(), nprocs, iters, max_seconds,
                    nprocs * iters / max_seconds, ok ? "yes" : "NO");
    }
    MPI_Win_unlock_all(win);
    MPI_Win_free(&win);
    MPI_Finalize();
}
