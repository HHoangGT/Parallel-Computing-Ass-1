#pragma once

#include <mpi.h>

template <class T> MPI_Datatype mpi_type();
template <> inline MPI_Datatype mpi_type<int>() { return MPI_INT; }
template <> inline MPI_Datatype mpi_type<long>() { return MPI_LONG; }

template <class T>
T rma_fetch_op(T value, int target, MPI_Aint disp, MPI_Op op, MPI_Win win) {
    T old;
    MPI_Fetch_and_op(&value, &old, mpi_type<T>(), target, disp, op, win);
    MPI_Win_flush(target, win);
    return old;
}

template <class T> T rma_load(int target, MPI_Aint disp, MPI_Win win) {
    return rma_fetch_op<T>(0, target, disp, MPI_NO_OP, win);
}

template <class T> void rma_store(T value, int target, MPI_Aint disp, MPI_Win win) {
    rma_fetch_op<T>(value, target, disp, MPI_REPLACE, win);
}

template <class T> T rma_swap(T value, int target, MPI_Aint disp, MPI_Win win) {
    return rma_fetch_op<T>(value, target, disp, MPI_REPLACE, win);
}

template <class T> T rma_fetch_add(T value, int target, MPI_Aint disp, MPI_Win win) {
    return rma_fetch_op<T>(value, target, disp, MPI_SUM, win);
}

template <class T> T rma_cas(T expected, T desired, int target, MPI_Aint disp, MPI_Win win) {
    T old;
    MPI_Compare_and_swap(&desired, &expected, &old, mpi_type<T>(), target, disp, win);
    MPI_Win_flush(target, win);
    return old;
}

struct MpiBackoff {
    unsigned delay_us = 1, max_us = 64;
    void pause();
};
