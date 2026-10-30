# barrier / linux-h200__placement-1s-nosmt: benchmark summary (medians over repetitions)

```
platform: linux-h200__placement-1s-nosmt
date: Fri Oct 30 10:03:04 AM UTC 2026
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
threads_swept: 128
build: build  CACHE_LINE=64  SEQ_CST=OFF
runner: numactl --physcpubind=0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127
bench_args: none
load1_at_start: 61.19
```

## Barrier: median episodes/s

|   threads |     sr |   std |
|----------:|-------:|------:|
|       128 | 156657 |  8861 |

TIMEOUT rows dropped: barrier.csv: 0.
