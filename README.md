# Assignment 1: locks, barrier, concurrent counters, MPI one-sided

C++20, `std::thread` + `std::atomic`. Layout: `include/` the lock, barrier, counter and backoff headers, `src/common/` the benchmark harness, `src/` benchmarks, `test/` correctness tests, `mpi/` Part 4, `cmake/` compiler options, `scripts/` sweep and plots, `benchmarks/<part>/<platform>/` measurements.

## Build

```bash
cmake -S . -B build && cmake --build build
```

Options on the first configure: `-DCMAKE_CXX_COMPILER=<path>` (macOS defaults to Homebrew LLVM when installed), `-DCACHE_LINE=<bytes>` (auto-detected: `sysctl` on macOS, `getconf` on Linux, 64 on Windows), `-DWITH_OPENMP=OFF`, `-DWITH_MPI=OFF`.
Dependencies: macOS `brew install libomp open-mpi` (Apple clang works; `brew install llvm` is optional); Linux g++ 13, libomp, OpenMPI; Windows MSVC or clang-cl (the shell scripts need Git Bash or WSL).

## Tests

```bash
ctest --test-dir build --output-on-failure
```

```bash
cmake -S . -B build-tsan -DTSAN=ON && cmake --build build-tsan && ctest --test-dir build-tsan --output-on-failure
```

TSan is not available on Windows and crashes on Apple clang 16 / macOS 26; it works with Homebrew clang 23 and Apple clang 21. Under TSan, `test_counter` skips the counters built on our own spinlocks (a tight spin livelocks under TSan's serialised atomics); `test_locks` covers those locks.

## Benchmarks

```bash
./build/bench_lock     --lock mcs       --threads 8 --iters 1000 --outside 2 --n 10007 --reps 5 --header 1
./build/bench_barrier  --barrier sr     --threads 8 --episodes 100000 --reps 5 --header 1
./build/bench_counter  --counter faa    --threads 8 --total 4000000 --reps 5 --header 1
./build/bench_fairness --counter cas_bo --threads 8 --seconds 1 --header 1
mpirun -np 4 ./build/mpi_counter --variant faa --ops 20000 --header 1
mpirun -np 4 ./build/mpi_lock    --variant mcs --iters 1000 --header 1
mpirun -np 4 ./build/mpi_barrier --variant sr  --episodes 5000 --check 1 --header 1
```

Names: locks `tas ttas tas_bo ttas_bo ticket mcs std_mutex pthread omp`; barriers `sr sr_bo std omp pthread` (pthread on Linux only); counters `lock_<lock>` for every lock above (`lock_mutex` for std_mutex) plus `cas cas_bo faa sharded`; MPI counters `faa cas cas_bo`, locks `tas mcs winlock`, barriers `sr sr_bo mpi`.
Common flags: `--timeout S` (prints a `TIMEOUT` row and exits 2 instead of hanging, default 60), `--min_delay`, `--max_delay`, `--sleep 1` (backoff knobs).

## Sweep and figures

```bash
PLATFORM=mac-m4pro ./scripts/run_all.sh
```

```bash
uv run scripts/plot.py
```

`run_all.sh` builds `./build` if needed and writes `benchmarks/<part>/<platform>/*.csv` and `platform.txt`; `PARTS` entries map to folders as locks, shortcs, backoff -> `locks`, barrier -> `barrier`, counter, fairness -> `counter`, mpi -> `mpi`. Knobs: `PARTS` (default `locks shortcs backoff barrier counter fairness mpi`), `THREADS`, `REPS`, `TIMEOUT`, `TOTAL`, `FAIR_SECONDS`, `BUILD`. `plot.py` writes `fig_*.png` and `summary.md` next to each CSV (medians over repetitions).

## Measured platforms

| Folder | Machine | Compiler | Parts |
|---|---|---|---|
| `mac-m4pro` | MacBook Pro, Apple M4 Pro (8 P + 4 E cores) | Homebrew clang 23 | 3 |
| `mac-m4` | MacBook Air, Apple M4 (4 P + 6 E cores) | Apple clang 16 | 1, 2, 4 |
| `linux-a6000` | 2 × AMD EPYC 7352 (48 cores / 96 threads, 2 NUMA nodes) | g++ 13.3 (no clang installed) | 1, 2 |

