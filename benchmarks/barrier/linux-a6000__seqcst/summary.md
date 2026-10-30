# barrier / linux-a6000__seqcst: benchmark summary (medians over repetitions)

```
platform: linux-a6000__seqcst
date: Fri Oct 30 05:09:23 PM +07 2026
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
threads_swept: 1 2 4 8 16 24 48 96 
build: build-seqcst  CACHE_LINE=64  SEQ_CST=ON
runner: none
bench_args: none
load1_at_start: 50.88
```

## Barrier: median episodes/s

|   threads |               sr |              std |
|----------:|-----------------:|-----------------:|
|         1 |      5.11696e+07 |      4.90717e+07 |
|         2 |      3.50189e+06 | 305091           |
|         4 |      1.10737e+06 | 136612           |
|         8 | 544832           |  94661           |
|        16 | 298297           |  49637           |
|        24 | 313476           |  34371           |
|        48 | 277639           |  17666           |
|        96 | 100165           |   7738           |

TIMEOUT rows dropped: barrier.csv: 0.
