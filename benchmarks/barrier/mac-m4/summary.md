# barrier / mac-m4: benchmark summary (medians over repetitions)

```
platform: mac-m4
date: Wed Oct 28 22:40:44 +07 2026
uname: Darwin HoangTrans-MacBook-Air.local 25.5.0 Darwin Kernel Version 25.5.0: Tue Jun  9 22:26:22 PDT 2026; root:xnu-12377.121.10~1/RELEASE_ARM64_T8132 arm64
logical_cpus: 10
cpu: Apple M4
perf_cores: 4  efficiency_cores: 6
memory_bytes: 25769803776
compiler: Apple clang version 16.0.0 (clang-1600.0.26.3)
threads_swept: 1 2 4 6 8 10 15 20
```

## Barrier: median episodes/s

|   threads |             omp |            sr |         sr_bo |              std |
|----------:|----------------:|--------------:|--------------:|-----------------:|
|         1 |      1.5454e+08 |   2.70514e+08 |   3.4818e+08  |      4.53738e+07 |
|         2 | 398938          |   1.17013e+07 |   1.03061e+07 |      6.58017e+06 |
|         4 | 135058          |   7.39882e+06 |   2.91871e+06 |      1.39471e+06 |
|         6 |  67567          |   2.87846e+06 |   1.7268e+06  | 386648           |
|         8 |  53166          |   2.14089e+06 |   1.265e+06   | 825500           |
|        10 |  41447          |   1.3277e+06  |   1.10631e+06 | 245414           |
|        15 |  25937          | nan           | nan           |  50713           |
|        20 |  18337          | nan           | nan           |  28460           |

TIMEOUT rows dropped: barrier.csv: 4.
