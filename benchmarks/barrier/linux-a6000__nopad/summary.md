# barrier / linux-a6000__nopad: benchmark summary (medians over repetitions)

```
platform: linux-a6000__nopad
date: Fri Oct 30 05:08:19 PM +07 2026
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
build: build-nopad  CACHE_LINE=8  SEQ_CST=OFF
runner: none
bench_args: none
load1_at_start: 43.19
```

## Barrier: median episodes/s

|   threads |               sr |              std |
|----------:|-----------------:|-----------------:|
|         1 |      1.20063e+08 |      4.51542e+07 |
|         2 |      2.08182e+06 | 467047           |
|         4 | 715286           | 205713           |
|         8 | 360838           | 101490           |
|        16 | 226549           |  50513           |
|        24 | 200564           |  37190           |
|        48 | 176527           |  17340           |
|        96 |  56100           |   7790           |

TIMEOUT rows dropped: barrier.csv: 0.
