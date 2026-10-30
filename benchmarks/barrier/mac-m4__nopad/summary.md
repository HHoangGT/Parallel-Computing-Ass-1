# barrier / mac-m4__nopad: benchmark summary (medians over repetitions)

```
platform: mac-m4__nopad
date: Fri Oct 30 16:59:31 +07 2026
uname: Darwin HoangTrans-MacBook-Air.local 25.5.0 Darwin Kernel Version 25.5.0: Tue Jun  9 22:26:22 PDT 2026; root:xnu-12377.121.10~1/RELEASE_ARM64_T8132 arm64
logical_cpus: 10
macos: 26.5.2
cpu: Apple M4
perf_cores: 4  efficiency_cores: 6
memory_bytes: 25769803776
compiler: Apple clang version 16.0.0 (clang-1600.0.26.3)
threads_swept: 1 2 4 5 8 10 
build: build-nopad  CACHE_LINE=8  SEQ_CST=OFF
runner: none
bench_args: none
load1_at_start: 69.66
```

## Barrier: median episodes/s

|   threads |               sr |         std |
|----------:|-----------------:|------------:|
|         1 |      3.06084e+08 | 5.55787e+07 |
|         2 |      9.52188e+06 | 9.16685e+06 |
|         4 |      5.21061e+06 | 4.96625e+06 |
|         5 |      2.19682e+06 | 1.76086e+06 |
|         8 |      1.41538e+06 | 1.31474e+06 |
|        10 | 904825           | 1.00445e+06 |

TIMEOUT rows dropped: barrier.csv: 0.
