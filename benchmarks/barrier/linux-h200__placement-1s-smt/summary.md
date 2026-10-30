# barrier / linux-h200__placement-1s-smt: benchmark summary (medians over repetitions)

```
platform: linux-h200__placement-1s-smt
date: Fri Oct 30 10:03:33 AM UTC 2026
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
runner: numactl --physcpubind=0,256,1,257,2,258,3,259,4,260,5,261,6,262,7,263,8,264,9,265,10,266,11,267,12,268,13,269,14,270,15,271,16,272,17,273,18,274,19,275,20,276,21,277,22,278,23,279,24,280,25,281,26,282,27,283,28,284,29,285,30,286,31,287,32,288,33,289,34,290,35,291,36,292,37,293,38,294,39,295,40,296,41,297,42,298,43,299,44,300,45,301,46,302,47,303,48,304,49,305,50,306,51,307,52,308,53,309,54,310,55,311,56,312,57,313,58,314,59,315,60,316,61,317,62,318,63,319
bench_args: none
load1_at_start: 93.38
```

## Barrier: median episodes/s

|   threads |    sr |   std |
|----------:|------:|------:|
|       128 | 36598 |  7663 |

TIMEOUT rows dropped: barrier.csv: 0.
