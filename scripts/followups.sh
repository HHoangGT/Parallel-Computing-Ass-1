#!/usr/bin/env bash
# Follow-up experiments chosen from the analysis of the baseline (see the report, Section 4).
#   PLATFORM=linux-h200 ./scripts/followups.sh convoy ctxsw ttasbump
# Output: benchmarks/followup/<PLATFORM>/<study>.csv
set -u
cd "$(dirname "$0")/.."
PLATFORM=${PLATFORM:?set PLATFORM}
BUILD=${BUILD:-build}
REPS=${REPS:-5}
TIMEOUT=${TIMEOUT:-60}
OUT=benchmarks/followup/$PLATFORM; mkdir -p "$OUT"
NCPU=$( (nproc 2>/dev/null || sysctl -n hw.ncpu) | head -1)
others_cpus() {  # CPUs other users keep busy right now (instantaneous, from top)
  top -b -n 1 -w 512 2>/dev/null | awk -v me="$USER" 'NR>7 && $9+0>0 && $2!=me { o+=$9 } END { printf "%.1f", o/100 }'
}
for study in "$@"; do
  case $study in
    convoy)  # Where does the FIFO collapse start relative to the number of free hardware threads?
      csv=$OUT/convoy.csv; echo "lock,threads,iters,outside,n,rep,seconds,others_cpus" > "$csv"
      for lock in mcs ticket ttas pthread; do
        for t in $((NCPU - 64)) $((NCPU - 32)) $((NCPU - 16)) $((NCPU - 8)) $NCPU; do
          o=$(others_cpus)
          "$BUILD"/bench_lock --lock $lock --threads $t --iters 100 --outside 0 --n 10007 --reps "$REPS" \
            --timeout "$TIMEOUT" 2>/dev/null | sed "s/$/,$o/" >> "$csv"
          echo "convoy: $lock t=$t others=$o"
        done
      done ;;
    ctxsw)  # Do mutex waiters sleep on every hand-off when there is outside work? (voluntary context switches)
      csv=$OUT/ctxsw.csv; echo "lock,threads,outside,seconds_total,voluntary_cs,involuntary_cs" > "$csv"
      for lock in pthread std_mutex mcs ttas; do
        for ow in 0 2; do
          t=64; [ "$NCPU" -lt 64 ] && t=$NCPU
          r=$( { /usr/bin/time -v "$BUILD"/bench_lock --lock $lock --threads $t --iters 100 --outside $ow --n 10007 \
                   --reps 1 --timeout "$TIMEOUT" >/dev/null; } 2>&1 )
          v=$(echo "$r" | awk -F: '/Voluntary context switches/{gsub(/ /,"",$2); print $2}')
          i=$(echo "$r" | awk -F: '/Involuntary context switches/{gsub(/ /,"",$2); print $2}')
          s=$(echo "$r" | awk -F': ' '/Elapsed \(wall clock\)/{print $2}')
          echo "$lock,$t,$ow,$s,$v,$i" >> "$csv"; echo "ctxsw: $lock ow=$ow vol=$v invol=$i"
        done
      done ;;
    ttasbump)  # Fine sweep around P = 1 + W for W = 2
      csv=$OUT/ttasbump.csv; echo "lock,threads,iters,outside,n,rep,seconds" > "$csv"
      for lock in tas ttas ttas_bo mcs pthread; do
        for t in 2 3 4 5 6 8 12; do
          "$BUILD"/bench_lock --lock $lock --threads $t --iters 100 --outside 2 --n 10007 --reps "$REPS" \
            --timeout "$TIMEOUT" 2>/dev/null >> "$csv"
        done
        echo "ttasbump: $lock"
      done ;;
    interleave)  # Padding / memory-order A/B/C test with the builds interleaved round by round, so slow drift
                 # (thermal, background load) hits every variant equally. std_mutex is the control: it is not
                 # affected by CACHE_LINE or SEQ_CST, so its spread shows the noise floor.
      csv=$OUT/interleave.csv; echo "variant,round,lock,threads,iters,outside,n,rep,seconds" > "$csv"
      bcsv=$OUT/interleave_barrier.csv; echo "variant,round,barrier,threads,episodes,rep,seconds,episodes_per_sec" > "$bcsv"
      ROUNDS=${ROUNDS:-7}; ITHREADS=${ITHREADS:-"1 4 8 $NCPU"}
      for r in $(seq "$ROUNDS"); do
        for v in build build-nopad build-seqcst; do
          for lock in tas ttas ticket mcs std_mutex; do
            for t in $ITHREADS; do
              "$v"/bench_lock --lock $lock --threads $t --iters "${SHORT_ITERS:-20000}" --outside 0 --n 101 --reps 1 \
                --timeout "$TIMEOUT" 2>/dev/null | sed "s/^/$v,$r,/" >> "$csv"
            done
          done
          for b in sr std; do
            for t in $ITHREADS; do
              "$v"/bench_barrier --barrier $b --threads $t --episodes "${EPISODES:-100000}" --reps 1 \
                --timeout "$TIMEOUT" 2>/dev/null | sed "s/^/$v,$r,/" >> "$bcsv"
            done
          done
        done
        echo "interleave: round $r"
      done ;;
    *) echo "unknown study $study"; exit 1 ;;
  esac
done
echo "followups done -> $OUT"
