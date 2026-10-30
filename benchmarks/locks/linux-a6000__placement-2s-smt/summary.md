# locks / linux-a6000__placement-2s-smt: benchmark summary (medians over repetitions)

```
platform: linux-a6000__placement-2s-smt
date: Fri Oct 30 05:07:32 PM +07 2026
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
threads_swept: 24
build: build  CACHE_LINE=64  SEQ_CST=OFF
runner: numactl --physcpubind=0,48,1,49,2,50,3,51,4,52,5,53,24,72,25,73,26,74,27,75,28,76,29,77
bench_args: none
load1_at_start: 15.24
```

## Short CS, n=101: median µs per critical section

|   threads |   mcs |   std_mutex |   tas |   tas_bo |   ticket |   ttas |   ttas_bo |
|----------:|------:|------------:|------:|---------:|---------:|-------:|----------:|
|        24 | 0.588 |       1.149 | 1.636 |     0.51 |    0.863 |  0.717 |     0.639 |

TIMEOUT rows dropped: locks_shortcs.csv: 0.
