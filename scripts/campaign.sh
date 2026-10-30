#!/usr/bin/env bash
# Report campaign for one machine: baseline sweep, then the machine's ablations.
#   ./scripts/campaign.sh mac-m4 | linux-a6000 | linux-h200
set -u
cd "$(dirname "$0")/.."
M=$1
if [ -f ~/miniconda3/etc/profile.d/conda.sh ]; then        # servers: clang + Open MPI from conda-forge
  source ~/miniconda3/etc/profile.d/conda.sh && conda activate asm1 && export CXX=$CONDA_PREFIX/bin/clang++
fi
export REPS=5
case $M in
  mac-m4)
    PLATFORM=$M PARTS="locks shortcs backoff barrier mpi" MPI_PROCS="2 4 8 16" ./scripts/run_all.sh
    REPS=3 MACHINE=$M ./scripts/run_ablation.sh padding seqcst backoff ;;
  linux-a6000)
    PLATFORM=$M PARTS="locks shortcs barrier mpi" THREADS="1 2 4 8 12 16 24 32 48 64 96 144" \
      ITERS=300 OWS="0 2 8" SHORT_ITERS=5000 EPISODES=20000 MPI_PROCS="2 4 8 16" ./scripts/run_all.sh
    REPS=3 SHORT_ITERS=5000 EPISODES=20000 MACHINE=$M ./scripts/run_ablation.sh placement padding seqcst ;;
  linux-h200)
    PLATFORM=$M PARTS="locks shortcs barrier mpi" THREADS="1 2 4 8 16 32 64 128 256 384 512" \
      ITERS=100 OWS="0 2 8" SHORT_ITERS=2000 EPISODES=10000 MPI_PROCS="2 4 8 16" TIMEOUT=45 ./scripts/run_all.sh
    REPS=3 SHORT_ITERS=2000 EPISODES=10000 TIMEOUT=45 MACHINE=$M ./scripts/run_ablation.sh placement ;;
  *) echo "unknown machine $M"; exit 1 ;;
esac
echo CAMPAIGN_DONE
