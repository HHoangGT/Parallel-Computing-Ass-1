# mpi / mac-m4: benchmark summary (medians over repetitions)

```
platform: mac-m4
date: Fri Oct 30 16:46:34 +07 2026
uname: Darwin HoangTrans-MacBook-Air.local 25.5.0 Darwin Kernel Version 25.5.0: Tue Jun  9 22:26:22 PDT 2026; root:xnu-12377.121.10~1/RELEASE_ARM64_T8132 arm64
logical_cpus: 10
macos: 26.5.2
cpu: Apple M4
perf_cores: 4  efficiency_cores: 6
memory_bytes: 25769803776
compiler: Apple clang version 16.0.0 (clang-1600.0.26.3)
threads_swept: 1 2 4 6 8 10 15 20
build: build  CACHE_LINE=128  SEQ_CST=OFF
runner: none
bench_args: none
load1_at_start: 22.35
```

## mpi_counter.csv: median mops_per_sec

|   procs |   cas |   cas_bo |   faa |
|--------:|------:|---------:|------:|
|       2 | 16.59 |    29.67 | 52.56 |
|       4 |  9.05 |    13.49 | 23.88 |
|       8 |  4.39 |     8.73 |  9.33 |
|      16 |  2.79 |     5.59 |  5.86 |

All completed runs passed the correctness check.

## mpi_lock.csv: median acq_per_sec

|   procs |              mcs |         tas |          winlock |
|--------:|-----------------:|------------:|-----------------:|
|       2 |      1.62588e+06 | 1.55521e+07 |      1.19617e+07 |
|       4 |  11045.3         | 1.34816e+07 |      8.45487e+06 |
|       8 | 294504           | 1.08725e+07 |      4.54545e+06 |
|      16 |    nan           | 9.79372e+06 | 150797           |

All completed runs passed the correctness check. 5 run(s) hit the timeout and are excluded.

## mpi_barrier.csv: median episodes_per_sec

|   procs |             mpi |       sr |    sr_bo |
|--------:|----------------:|---------:|---------:|
|       2 |     8.99281e+06 | 531237   | 492126   |
|       4 |     3.89105e+06 |   1078.1 | 220643   |
|       8 |     1.25031e+06 |    365.8 |  63794.2 |
|      16 | 26489.8         |    nan   |  13124.3 |

All completed runs passed the correctness check. 5 run(s) hit the timeout and are excluded.
