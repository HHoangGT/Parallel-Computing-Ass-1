# locks / linux-a6000__placement-2s-nosmt: benchmark summary (medians over repetitions)

```
platform: linux-a6000__placement-2s-nosmt
date: Fri Oct 30 05:07:27 PM +07 2026
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
runner: numactl --physcpubind=0,1,2,3,4,5,6,7,8,9,10,11,24,25,26,27,28,29,30,31,32,33,34,35
bench_args: none
load1_at_start: 15.61
```

## Short CS, n=101: median µs per critical section

|   threads |   mcs |   std_mutex |   tas |   tas_bo |   ticket |   ttas |   ttas_bo |
|----------:|------:|------------:|------:|---------:|---------:|-------:|----------:|
|        24 | 0.649 |         1.2 | 1.209 |    0.509 |    0.918 |  0.759 |       0.8 |

TIMEOUT rows dropped: locks_shortcs.csv: 0.
