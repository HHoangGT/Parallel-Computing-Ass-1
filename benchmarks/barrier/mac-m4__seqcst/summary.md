# barrier / mac-m4__seqcst: benchmark summary (medians over repetitions)

```
platform: mac-m4__seqcst
date: Fri Oct 30 16:59:37 +07 2026
uname: Darwin HoangTrans-MacBook-Air.local 25.5.0 Darwin Kernel Version 25.5.0: Tue Jun  9 22:26:22 PDT 2026; root:xnu-12377.121.10~1/RELEASE_ARM64_T8132 arm64
logical_cpus: 10
macos: 26.5.2
cpu: Apple M4
perf_cores: 4  efficiency_cores: 6
memory_bytes: 25769803776
compiler: Apple clang version 16.0.0 (clang-1600.0.26.3)
threads_swept: 1 2 4 5 8 10 
build: build-seqcst  CACHE_LINE=128  SEQ_CST=ON
runner: none
bench_args: none
load1_at_start: 64.49
```

## Barrier: median episodes/s

|   threads |          sr |              std |
|----------:|------------:|-----------------:|
|         1 | 2.84259e+08 |      5.54093e+07 |
|         2 | 1.0814e+07  |      8.5207e+06  |
|         4 | 7.45918e+06 |      4.12266e+06 |
|         5 | 3.21042e+06 |      1.75562e+06 |
|         8 | 2.15043e+06 |      1.42649e+06 |
|        10 | 1.39e+06    | 246490           |

TIMEOUT rows dropped: barrier.csv: 0.
