# mpi / linux-h200: benchmark summary (medians over repetitions)

```
platform: linux-h200
date: Fri Oct 30 09:38:41 AM UTC 2026
uname: Linux namih200001 6.8.0-147-generic #147-Ubuntu SMP PREEMPT_DYNAMIC Thu Sep 24 11:33:16 UTC 2026 x86_64 x86_64 x86_64 GNU/Linux
logical_cpus: 512
Model name:                              AMD EPYC 9754 128-Core Processor
Thread(s) per core:                      2
Core(s) per socket:                      128
Socket(s):                               2
L1d cache:                               8 MiB (256 instances)
L2 cache:                                256 MiB (256 instances)
L3 cache:                                512 MiB (32 instances)
NUMA node(s):                            2
MemTotal:       1584053800 kB
compiler: clang version 23.1.4 (https://github.com/conda-forge/clangdev-feedstock babf9b0ec04c97fffe9fc24ec045deeb91a2c910)
threads_swept: 1 2 4 8 16 32 64 128 256 384 512
build: build  CACHE_LINE=64  SEQ_CST=OFF
runner: none
bench_args: none
load1_at_start: 282.37
```

## mpi_counter.csv: median mops_per_sec

|   procs |   cas |   cas_bo |   faa |
|--------:|------:|---------:|------:|
|       2 |  6.5  |    17.28 | 10.36 |
|       4 |  3.91 |    14.53 |  8    |
|       8 |  3.32 |    14.06 |  6.35 |
|      16 |  1.84 |    13.73 |  4.22 |

All completed runs passed the correctness check.

## mpi_lock.csv: median acq_per_sec

|   procs |             mcs |         tas |     winlock |
|--------:|----------------:|------------:|------------:|
|       2 |      1.4578e+06 | 9.01932e+06 | 2.77302e+06 |
|       4 | 259552          | 8.71199e+06 | 1.32134e+06 |
|       8 | 239540          | 7.8433e+06  | 1.1679e+06  |
|      16 | 166820          | 7.67371e+06 | 1.21121e+06 |

All completed runs passed the correctness check.

## mpi_barrier.csv: median episodes_per_sec

|   procs |              mpi |       sr |   sr_bo |
|--------:|-----------------:|---------:|--------:|
|       2 |      2.02157e+06 | 695418   | 32398.5 |
|       4 | 657568           |  93957.1 | 22379.8 |
|       8 | 446114           |   5397.5 | 19502.6 |
|      16 | 342930           |    294.5 | 18211.2 |

All completed runs passed the correctness check. 1 run(s) hit the timeout and are excluded.
