# locks / mac-m4__sleep: benchmark summary (medians over repetitions)

```
platform: mac-m4__sleep
date: Fri Oct 30 16:59:41 +07 2026
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
bench_args: --sleep 1
load1_at_start: 59.40
```

## Backoff tuning (median s)

|   max_delay |   tas_bo |   ttas_bo |
|------------:|---------:|----------:|
|          16 |   0.0542 |    0.0593 |
|          64 |   0.0513 |    0.0597 |
|         256 |   0.0511 |    0.0659 |
|        1024 |   0.0523 |    0.0617 |
|        4096 |   0.0523 |    0.0541 |
|       16384 |   0.0519 |    0.0652 |

TIMEOUT rows dropped: backoff.csv: 0.
