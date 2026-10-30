# barrier / linux-a6000: benchmark summary (medians over repetitions)

```
platform: linux-a6000
date: Fri Oct 30 04:39:30 PM +07 2026
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
load1_at_start: 50.07
```

## Barrier: median episodes/s

|   threads |              omp |          pthread |               sr |            sr_bo |              std |
|----------:|-----------------:|-----------------:|-----------------:|-----------------:|-----------------:|
|         1 |      9.01108e+07 |      8.25612e+06 |      1.21051e+08 |      1.06451e+08 |      4.62988e+07 |
|         2 | 941278           | 221092           |      2.23824e+06 |      2.62538e+06 | 579264           |
|         4 | 739286           | 166586           |      1.06793e+06 | 827510           | 128662           |
|         8 | 449020           |  92159           | 549379           | 445597           | 100221           |
|        12 | 441628           |  63125           | 361844           | 340132           |  63854           |
|        16 | 394237           |  51888           | 317580           | 204634           |  51583           |
|        24 | 264075           |  39541           | 306468           |  76018           |  36199           |
|        32 | 253556           |  34054           | 299325           |  54892           |  26729           |
|        48 | 252380           |  23295           | 268788           |  54207           |  16840           |
|        64 | 147326           |  15888           | 255169           |  45801           |  12534           |
|        96 |  25968           |   8126           | 138937           |  24318           |   7770           |
|       144 |  14028           |   5023           |    nan           |    nan           |   5249           |

TIMEOUT rows dropped: barrier.csv: 2.
