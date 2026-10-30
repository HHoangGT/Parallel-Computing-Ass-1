# locks / mac-m4__nopad: benchmark summary (medians over repetitions)

```
platform: mac-m4__nopad
date: Fri Oct 30 16:59:28 +07 2026
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

## Short CS, n=101: median µs per critical section

|   threads |   mcs |   std_mutex |   tas |   tas_bo |   ticket |   ttas |   ttas_bo |
|----------:|------:|------------:|------:|---------:|---------:|-------:|----------:|
|         1 | 0.051 |       0.05  | 0.192 |    0.05  |    0.05  |  0.05  |     0.05  |
|         2 | 0.09  |       0.077 | 0.105 |    0.051 |    0.077 |  0.05  |     0.05  |
|         4 | 0.124 |       0.084 | 0.111 |    0.053 |    0.089 |  0.05  |     0.05  |
|         5 | 0.27  |       0.125 | 0.136 |    0.06  |    0.205 |  0.061 |     0.06  |
|         8 | 0.352 |       0.164 | 0.303 |    0.139 |    0.302 |  0.109 |     0.078 |
|        10 | 0.394 |       0.199 | 0.369 |    0.179 |    0.528 |  0.084 |     0.115 |

TIMEOUT rows dropped: locks_shortcs.csv: 0.
