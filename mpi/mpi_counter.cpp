// Part 4 counter in rank 0's window: faa, cas, cas_bo (CAS + backoff).
#include <cstdio>

#include "common/common.h"
#include "mpi_common.h"

static int rank, nprocs;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    Args args(argc, argv);
    std::string variant = args.get("variant", "faa");
    long ops = args.geti("ops", 20000);

    std::string row = variant + "," + std::to_string(nprocs) + "," + std::to_string(ops) + ",TIMEOUT,0,timeout";
    // rank 0 prints the TIMEOUT row and exits first; other ranks only fire 5 s later as a fallback
    start_watchdog(args.geti("timeout", 60) + (rank == 0 ? 0 : 5), rank == 0 ? row : "");

    long* base;
    MPI_Win win;
    MPI_Win_allocate(sizeof(long), sizeof(long), MPI_INFO_NULL, MPI_COMM_WORLD, &base, &win);
    *base = 0;
    MPI_Win_lock_all(0, win);
    MPI_Barrier(MPI_COMM_WORLD);

    double start = MPI_Wtime();
    for (long i = 0; i < ops; ++i) {
        if (variant == "faa") {
            rma_fetch_add<long>(1, 0, 0, win);
        } else {
            MpiBackoff backoff;
            long old = rma_load<long>(0, 0, win);
            while (true) {
                long seen = rma_cas<long>(old, old + 1, 0, 0, win);
                if (seen == old) break;
                old = seen;
                if (variant == "cas_bo") backoff.pause();
            }
        }
    }
    double seconds = MPI_Wtime() - start;
    MPI_Barrier(MPI_COMM_WORLD);
    double max_seconds;
    MPI_Reduce(&seconds, &max_seconds, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        long final_value = rma_load<long>(0, 0, win);
        if (args.geti("header", 0)) std::printf("counter,procs,ops_per_rank,seconds,mops_per_sec,correct\n");
        std::printf("%s,%d,%ld,%.6f,%.4f,%s\n", variant.c_str(), nprocs, ops, max_seconds,
                    nprocs * ops / max_seconds / 1e6, final_value == nprocs * ops ? "yes" : "NO");
    }
    MPI_Win_unlock_all(win);
    MPI_Win_free(&win);
    MPI_Finalize();
}
