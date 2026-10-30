# locks / mac-m4__seqcst: benchmark summary (medians over repetitions)

```
platform: mac-m4__seqcst
date: Fri Oct 30 16:59:34 +07 2026
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

## Short CS, n=101: median µs per critical section

|   threads |   mcs |   std_mutex |   tas |   tas_bo |   ticket |   ttas |   ttas_bo |
|----------:|------:|------------:|------:|---------:|---------:|-------:|----------:|
|         1 | 0.05  |       0.05  | 0.069 |    0.049 |    0.05  |  0.05  |     0.05  |
|         2 | 0.083 |       0.079 | 0.066 |    0.051 |    0.091 |  0.05  |     0.05  |
|         4 | 0.087 |       0.084 | 0.086 |    0.053 |    0.102 |  0.05  |     0.052 |
|         5 | 0.2   |       0.137 | 0.114 |    0.059 |    0.208 |  0.061 |     0.061 |
|         8 | 0.486 |       0.172 | 0.222 |    0.093 |    0.293 |  0.078 |     0.078 |
|        10 | 0.472 |       0.202 | 0.258 |    0.124 |    0.387 |  0.084 |     0.118 |

TIMEOUT rows dropped: locks_shortcs.csv: 0.
