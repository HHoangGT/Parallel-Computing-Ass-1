# barrier / linux-a6000: benchmark summary (medians over repetitions)

```
platform: linux-a6000
date: Wed Oct 28 06:10:18 PM +07 2026
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
compiler: c++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
threads_swept: 1 2 4 8 12 16 24 32 48 64 96 144
```

## Barrier: median episodes/s

|   threads |              omp |          pthread |               sr |            sr_bo |              std |
|----------:|-----------------:|-----------------:|-----------------:|-----------------:|-----------------:|
|         1 |      9.87795e+06 |      8.38828e+06 |      1.27555e+08 |      1.02865e+08 |      4.99494e+07 |
|         2 |      2.19073e+06 | 231305           |      3.52476e+06 |      2.68136e+06 | 397122           |
|         4 |      1.00985e+06 | 179685           |      1.05102e+06 | 696731           | 128376           |
|         8 | 571893           |  95623           | 532034           | 389386           |  96808           |
|        12 | 410946           |  82802           | 386015           | 288698           |  67240           |
|        16 | 323658           |  57940           | 294442           | 246639           |  58940           |
|        24 | 310018           |  45482           | 286545           | 106054           |  44053           |
|        32 | 296967           |  33233           | 288984           |  92419           |  31377           |
|        48 | 285424           |  22028           | 272077           |  48388           |  21163           |
|        64 | 279558           |  15157           | 270598           |  43105           |  13544           |
|        96 | 232847           |   8011           | 178156           |  25544           |   7873           |
|       144 |   5159           |   4951           |    nan           |    nan           |   5182           |

TIMEOUT rows dropped: barrier.csv: 7.
