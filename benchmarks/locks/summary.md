# Cross-platform: locks at #cores, OUTSIDE_WORK=0 (median s)

| lock      |   linux-a6000 (96 thr) |   mac-m4 (10 thr) |
|:----------|-----------------------:|------------------:|
| tas       |                 5.1248 |            0.073  |
| ttas      |                 4.739  |            0.0595 |
| tas_bo    |                 4.7922 |            0.1461 |
| ttas_bo   |                 4.6198 |            0.0593 |
| ticket    |                 5.4282 |            0.2584 |
| mcs       |                 7.6485 |            0.2414 |
| std_mutex |                 4.7173 |            0.0689 |
| pthread   |                 4.7179 |            0.0668 |
| omp       |                 4.6977 |            0.8824 |
