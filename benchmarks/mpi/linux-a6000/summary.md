# mpi / linux-a6000: benchmark summary (medians over repetitions)

```
platform: linux-a6000
date: Fri Oct 30 04:44:03 PM +07 2026
uname: Linux u116614 6.8.0-147-generic #147-Ubuntu SMP PREEMPT_DYNAMIC Thu Sep 24 11:33:16 UTC 2026 x86_64 x86_64 x86_64 GNU/Linux
logical_cpus: 96
Model name:                              AMD EPYC 7352 24-Core Processor
Thread(s) per core:                      2
Core(s) per socket:                      24
Socket(s):                               2
L1d cache:                               1.5 MiB (48 instances)
L2 cache:                                24 MiB (48 instances)
L3 cache:                                256 MiB (16 instances)
NUMA node(s):                            2
MemTotal:       528191584 kB
compiler: clang version 23.1.4 (https://github.com/conda-forge/clangdev-feedstock babf9b0ec04c97fffe9fc24ec045deeb91a2c910)
threads_swept: 1 2 4 8 12 16 24 32 48 64 96 144
build: build  CACHE_LINE=64  SEQ_CST=OFF
runner: none
bench_args: none
load1_at_start: 42.66
```

## mpi_counter.csv: median mops_per_sec

|   procs |   cas |   cas_bo |   faa |
|--------:|------:|---------:|------:|
|       2 |  4.66 |    12.6  | 12.1  |
|       4 |  3.18 |    12.16 |  6.31 |
|       8 |  2.51 |    11.28 |  5.35 |
|      16 |  1.71 |     7.37 |  4.26 |

All completed runs passed the correctness check.

## mpi_lock.csv: median acq_per_sec

|   procs |    mcs |         tas |          winlock |
|--------:|-------:|------------:|-----------------:|
|       2 | 524424 | 5.9932e+06  |      1.90764e+06 |
|       4 | 223492 | 5.97972e+06 |      1.04945e+06 |
|       8 | 228412 | 5.6644e+06  | 989380           |
|      16 | 213933 | 5.07017e+06 |      1.01741e+06 |

All completed runs passed the correctness check.

## mpi_barrier.csv: median episodes_per_sec

|   procs |              mpi |       sr |   sr_bo |
|--------:|-----------------:|---------:|--------:|
|       2 |      1.96637e+06 | 754073   | 36189.7 |
|       4 | 495144           |   7703.1 | 24039.1 |
|       8 | 376755           |    620.6 | 20571.9 |
|      16 | 257092           |    nan   | 18556.9 |

All completed runs passed the correctness check. 5 run(s) hit the timeout and are excluded.
