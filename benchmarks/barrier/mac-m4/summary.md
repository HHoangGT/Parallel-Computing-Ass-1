# barrier / mac-m4: benchmark summary (medians over repetitions)

```
platform: mac-m4
date: Fri Oct 30 16:40:49 +07 2026
uname: Darwin HoangTrans-MacBook-Air.local 25.5.0 Darwin Kernel Version 25.5.0: Tue Jun  9 22:26:22 PDT 2026; root:xnu-12377.121.10~1/RELEASE_ARM64_T8132 arm64
logical_cpus: 10
macos: 26.5.2
cpu: Apple M4
perf_cores: 4  efficiency_cores: 6
memory_bytes: 25769803776
compiler: Apple clang version 16.0.0 (clang-1600.0.26.3)
threads_swept: 1 2 4 6 8 10 15 20
build: build  CACHE_LINE=128  SEQ_CST=OFF
runner: none
bench_args: none
load1_at_start: 95.16
```

## Barrier: median episodes/s

|   threads |              omp |               sr |            sr_bo |              std |
|----------:|-----------------:|-----------------:|-----------------:|-----------------:|
|         1 |      1.87793e+08 |      4.49186e+08 |      3.42563e+08 |      4.80125e+07 |
|         2 | 405973           |      1.31374e+07 |      1.14321e+07 |      7.18722e+06 |
|         4 | 144146           |      6.90084e+06 |      3.12291e+06 |      3.87041e+06 |
|         6 |  75746           |      2.18508e+06 |      1.86976e+06 |      1.19858e+06 |
|         8 |  49113           |      2.00654e+06 |      1.49499e+06 |      1.26534e+06 |
|        10 |  39823           | 897747           | 898697           | 866654           |
|        15 |  27810           |    nan           |    nan           |  59646           |
|        20 |  20375           |    nan           |    nan           |  35313           |

TIMEOUT rows dropped: barrier.csv: 4.
