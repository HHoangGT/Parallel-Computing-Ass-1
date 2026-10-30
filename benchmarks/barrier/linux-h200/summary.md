# barrier / linux-h200: benchmark summary (medians over repetitions)

```
platform: linux-h200
date: Fri Oct 30 09:33:43 AM UTC 2026
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
load1_at_start: 510.72
```

## Barrier: median episodes/s

|   threads |              omp |          pthread |               sr |            sr_bo |              std |
|----------:|-----------------:|-----------------:|-----------------:|-----------------:|-----------------:|
|         1 |      1.19171e+08 |      1.85632e+06 |      2.26081e+08 |      1.3116e+08  |      4.67013e+07 |
|         2 |      2.07561e+06 | 170305           |      1.82384e+06 |      1.90842e+06 |      1.04824e+06 |
|         4 |      1.77939e+06 | 133263           |      1.48546e+06 | 587154           | 500243           |
|         8 | 935359           |  75920           | 635687           | 429266           | 103264           |
|        16 | 687626           |  45319           | 427881           | 233316           |  58955           |
|        32 | 562617           |  28135           | 269690           |  97225           |  29588           |
|        64 | 421912           |  16412           | 187050           |  49011           |  14987           |
|       128 | 276184           |   9441           | 126432           |  47596           |   7173           |
|       256 |  96675           |   4624           |  70118           |  44490           |   3878           |
|       384 | 114018           |   2816           |  53612           |  34205           |   2153           |
|       512 |   3486           |   1500           |    nan           |    nan           |   1584           |

TIMEOUT rows dropped: barrier.csv: 2.
