# mpi / mac-m4: benchmark summary (medians over repetitions)

```
platform: mac-m4
date: Wed Oct 28 22:40:44 +07 2026
uname: Darwin HoangTrans-MacBook-Air.local 25.5.0 Darwin Kernel Version 25.5.0: Tue Jun  9 22:26:22 PDT 2026; root:xnu-12377.121.10~1/RELEASE_ARM64_T8132 arm64
logical_cpus: 10
cpu: Apple M4
perf_cores: 4  efficiency_cores: 6
memory_bytes: 25769803776
compiler: Apple clang version 16.0.0 (clang-1600.0.26.3)
threads_swept: 1 2 4 6 8 10 15 20
```

## mpi_counter.csv: median mops_per_sec

|   procs |   cas |   cas_bo |   faa |
|--------:|------:|---------:|------:|
|       2 | 17.36 |    24.55 | 38.95 |
|       4 | 11.52 |    13.94 | 27.65 |
|       8 |  4.61 |     9.52 |  9.58 |
|      10 |  4.12 |     8.43 |  8.74 |

All completed runs passed the correctness check.

## mpi_lock.csv: median acq_per_sec

|   procs |      mcs |         tas |       winlock |
|--------:|---------:|------------:|--------------:|
|       2 | 914077   | 1.61551e+07 |   1.21433e+07 |
|       4 |   6285.4 | 1.52381e+07 |   1.05597e+07 |
|       8 | 242363   | 1.1491e+07  | nan           |
|      10 |  69970.2 | 1.07608e+07 |   4.55e+06    |

All completed runs passed the correctness check. 9 run(s) hit the timeout and are excluded.

## mpi_barrier.csv: median episodes_per_sec

|   procs |              mpi |    sr_bo |
|--------:|-----------------:|---------:|
|       2 |      3.3557e+06  | 197285   |
|       4 |      1.12385e+06 | 108571   |
|       8 | 303214           |  31929.1 |
|      10 | 214629           |  24848.5 |

All completed runs passed the correctness check.
