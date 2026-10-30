#!/usr/bin/env bash
# Follow-up experiments.
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
cpu_list() {
  lscpu -p=CPU,CORE,SOCKET | grep -v '^#' | awk -F, -v ns="$1" -v smt="$2" -v T="$3" '
    { cpu[NR]=$1; core[NR]=$2; sock[NR]=$3; n=NR }
    END {
      per_sock = int(T / ns); out = ""
      for (s = 0; s < ns; s++) {
        taken = 0; delete seen
        # first pass: one CPU per core; with smt, also take its sibling right away
        for (i = 1; i <= n && taken < per_sock; i++) {
          if (sock[i] != s || (core[i] in seen)) continue
          seen[core[i]] = 1; out = out (out ? "," : "") cpu[i]; taken++
          if (smt) for (j = 1; j <= n && taken < per_sock; j++)
            if (j != i && core[j] == core[i] && sock[j] == s) { out = out "," cpu[j]; taken++ }
        }
      }
      print out
    }'
}

# busy_in <cpu list>: how many of these CPUs were > 50% busy during the last second (/proc/stat delta)
busy_in() {
  local a b; a=$(grep '^cpu[0-9]' /proc/stat); sleep 1; b=$(grep '^cpu[0-9]' /proc/stat)
  paste <(echo "$a") <(echo "$b") | awk -v list=",$1," '{ c=substr($1,4); if (index(list, ","c",")==0) next;
    t1=$2+$3+$4+$5+$6+$7+$8; i1=$5+$6; t2=$13+$14+$15+$16+$17+$18+$19; i2=$16+$17;
    if (t2>t1 && 1-(i2-i1)/(t2-t1) > 0.5) n++ } END { print n+0 }'
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
    interleave)  # builds interleaved per round so drift hits every variant; std_mutex is the control
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
    joincost)  # Timing overhead of the harness: start flag -> last join with an EMPTY body (bench_lock --iters 0).
               # Bounds how much thread teardown inflates the lock/barrier timings (OpenMP is timed without it).
      csv=$OUT/joincost.csv; echo "threads,rep,seconds" > "$csv"
      for t in 1 2 4 8 16 32 64 128 256 384 512; do
        [ "$t" -gt "$NCPU" ] && continue
        "$BUILD"/bench_lock --lock tas --threads $t --iters 0 --reps 7 --timeout "$TIMEOUT" 2>/dev/null \
          | awk -F, '{print $2","$6","$7}' >> "$csv"
      done
      echo "joincost done" ;;
    placement2)  # Placement re-run: layouts interleaved round by round; records how many of the pinned CPUs
                 # were already busy (other users) just before each run.
      csv=$OUT/placement2.csv; echo "layout,round,busy_pinned_cpus,lock,threads,iters,outside,n,rep,seconds" > "$csv"
      bcsv=$OUT/placement2_barrier.csv; echo "layout,round,busy_pinned_cpus,barrier,threads,episodes,rep,seconds,episodes_per_sec" > "$bcsv"
      sockets=$(lscpu -p=SOCKET | grep -v '^#' | sort -u | wc -l)
      T=${T:-$(( $(lscpu -p=CORE | grep -v '^#' | sort -u | wc -l) / sockets ))}
      for r in $(seq "${ROUNDS:-3}"); do
        for layout in "1 0 1s-nosmt" "1 1 1s-smt" "2 0 2s-nosmt" "2 1 2s-smt"; do
          set -- $layout; cpus=$(cpu_list "$1" "$2" "$T"); busy=$(busy_in "$cpus")
          for lock in tas tas_bo ttas ticket mcs std_mutex; do
            numactl --physcpubind="$cpus" "$BUILD"/bench_lock --lock $lock --threads "$T" --iters "${SHORT_ITERS:-2000}" \
              --outside 0 --n 101 --reps 1 --timeout "$TIMEOUT" 2>/dev/null | sed "s/^/$3,$r,$busy,/" >> "$csv"
          done
          for b in sr std; do
            numactl --physcpubind="$cpus" "$BUILD"/bench_barrier --barrier $b --threads "$T" --episodes "${EPISODES:-10000}" \
              --reps 1 --timeout "$TIMEOUT" 2>/dev/null | sed "s/^/$3,$r,$busy,/" >> "$bcsv"
          done
          echo "placement2: round $r $3 busy_pinned=$busy"
        done
      done ;;
    *) echo "unknown study $study"; exit 1 ;;
  esac
done
echo "followups done -> $OUT"
