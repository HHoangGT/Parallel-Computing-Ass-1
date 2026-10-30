# locks / linux-a6000__nopad: benchmark summary (medians over repetitions)

```
platform: linux-a6000__nopad
date: Fri Oct 30 05:07:44 PM +07 2026
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
load1_at_start: 13.98
```

## Short CS, n=101: median µs per critical section

|   threads |   mcs |   std_mutex |   tas |   tas_bo |   ticket |   ttas |   ttas_bo |
|----------:|------:|------------:|------:|---------:|---------:|-------:|----------:|
|         1 | 0.477 |       0.473 | 0.468 |    0.468 |    0.472 |  0.469 |     0.469 |
|         2 | 0.584 |       0.764 | 0.466 |    0.468 |    0.629 |  0.468 |     0.468 |
|         4 | 0.817 |       1.065 | 0.58  |    0.472 |    0.856 |  0.58  |     0.566 |
|         8 | 0.834 |       1.173 | 0.935 |    0.479 |    1.115 |  0.717 |     0.698 |
|        16 | 0.773 |       1.207 | 1.691 |    0.496 |    1.129 |  0.816 |     0.798 |
|        24 | 0.736 |       1.189 | 2.03  |    0.514 |    1.052 |  0.777 |     0.786 |
|        48 | 0.653 |       1.213 | 2.506 |    0.584 |    0.936 |  0.788 |     0.827 |
|        96 | 0.884 |       1.297 | 7.872 |    0.864 |    1.802 |  0.807 |     0.799 |

TIMEOUT rows dropped: locks_shortcs.csv: 0.
