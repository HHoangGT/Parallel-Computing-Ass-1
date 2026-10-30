# locks / linux-a6000__seqcst: benchmark summary (medians over repetitions)

```
platform: linux-a6000__seqcst
date: Fri Oct 30 05:08:45 PM +07 2026
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
load1_at_start: 30.96
```

## Short CS, n=101: median µs per critical section

|   threads |   mcs |   std_mutex |   tas |   tas_bo |   ticket |   ttas |   ttas_bo |
|----------:|------:|------------:|------:|---------:|---------:|-------:|----------:|
|         1 | 0.475 |       0.473 | 0.468 |    0.476 |    0.469 |  0.471 |     0.473 |
|         2 | 0.578 |       0.768 | 0.465 |    0.469 |    0.869 |  0.468 |     0.471 |
|         4 | 0.815 |       1.068 | 0.574 |    0.475 |    1.196 |  0.479 |     0.609 |
|         8 | 0.841 |       1.104 | 1.1   |    0.486 |    1.14  |  0.676 |     0.739 |
|        16 | 0.843 |       1.244 | 2.296 |    0.515 |    1.186 |  0.805 |     0.802 |
|        24 | 0.764 |       1.198 | 2.726 |    0.536 |    1.053 |  0.78  |     0.808 |
|        48 | 0.686 |       1.207 | 3.846 |    0.65  |    1.001 |  0.79  |     0.823 |
|        96 | 1.773 |       1.297 | 8.484 |    0.946 |    1.699 |  0.823 |     0.822 |

TIMEOUT rows dropped: locks_shortcs.csv: 0.
