#!/usr/bin/env bash

set -u
cd "$(dirname "$0")/.."

NCPU=$( (nproc 2>/dev/null || sysctl -n hw.ncpu) | head -1)
PLATFORM=${PLATFORM:-$(hostname -s)}
REPS=${REPS:-5}
TIMEOUT=${TIMEOUT:-60}
PARTS=${PARTS:-"locks shortcs backoff barrier counter fairness mpi"}
TOTAL=${TOTAL:-4000000}
FAIR_SECONDS=${FAIR_SECONDS:-1}
BUILD=${BUILD:-build}
# Sweep knobs (defaults = the original sweep, except MPI_PROCS and SHORT_ITERS). RUN is a command prefix, e.g. "numactl --physcpubind=0-23".
ITERS=${ITERS:-1000}
OWS=${OWS:-"0 1 2 4 8"}
SHORT_NS=${SHORT_NS:-"1009 101"}
LOCKS=${LOCKS:-"tas ttas tas_bo ttas_bo ticket mcs std_mutex pthread omp"}
EPISODES=${EPISODES:-100000}
MPI_PROCS=${MPI_PROCS:-"2 4 8 16"}
RUN=${RUN:-}
BENCH_ARGS=${BENCH_ARGS:-}   # extra flags for bench_lock, e.g. "--sleep 1"
SHORT_ITERS=${SHORT_ITERS:-20000}   # short-CS runs: long enough that start-up noise is negligible
COUNTERS="lock_tas lock_ttas lock_tas_bo lock_ttas_bo lock_ticket lock_mcs lock_mutex lock_pthread lock_omp cas cas_bo faa sharded"
if [ -z "${THREADS:-}" ]; then
  THREADS="1 2"
  threads=4; while [ $threads -lt "$NCPU" ]; do THREADS="$THREADS $threads"; threads=$((threads + (threads < 16 ? 2 : threads / 4))); done
  THREADS="$THREADS $NCPU $((NCPU * 3 / 2)) $((NCPU * 2))"
fi
[ -f "$BUILD/CMakeCache.txt" ] || cmake -S . -B "$BUILD" >/dev/null || exit 1
cmake --build "$BUILD" >/dev/null || exit 1
CXX_BIN=$(sed -n 's/^CMAKE_CXX_COMPILER:[A-Z]*=//p' "$BUILD/CMakeCache.txt")

load_now() {
  if [ -r /proc/loadavg ]; then cut -d' ' -f1 /proc/loadavg; else sysctl -n vm.loadavg | awk '{print $2}'; fi
}
# One row per configuration in <dir>/load.csv, so noisy points on shared machines can be spotted.
log_load() {
  [ -f "$2/load.csv" ] || echo "config,unix_time,load1" > "$2/load.csv"
  echo "$1,$(date +%s),$(load_now)" >> "$2/load.csv"
}

platform_info() {
  echo "platform: $PLATFORM"
  echo "date: $(date)"
  echo "uname: $(uname -a)"
  echo "logical_cpus: $NCPU"
  if [ "$(uname)" = Darwin ]; then
    echo "macos: $(sw_vers -productVersion)"
    echo "cpu: $(sysctl -n machdep.cpu.brand_string)"
    echo "perf_cores: $(sysctl -n hw.perflevel0.physicalcpu 2>/dev/null)  efficiency_cores: $(sysctl -n hw.perflevel1.physicalcpu 2>/dev/null)"
    echo "memory_bytes: $(sysctl -n hw.memsize)"
  else
    lscpu 2>/dev/null | grep -E 'Model name|Socket|Core\(s\)|Thread\(s\)|NUMA node\(s\)|L1d|L2|L3'
    grep MemTotal /proc/meminfo
    grep -qi microsoft /proc/version 2>/dev/null && echo "note: running under WSL2 (Hyper-V VM)"
  fi
  echo "compiler: $("$CXX_BIN" --version | head -1)"
  echo "threads_swept: $THREADS"
  echo "build: $BUILD  CACHE_LINE=$(sed -n 's/^CACHE_LINE:[A-Z]*=//p' "$BUILD/CMakeCache.txt")  SEQ_CST=$(sed -n 's/^SEQ_CST:[A-Z]*=//p' "$BUILD/CMakeCache.txt")"
  echo "runner: ${RUN:-none}"
  echo "bench_args: ${BENCH_ARGS:-none}"
  echo "load1_at_start: $(load_now)"
}
platform_info

has() { case " $PARTS " in *" $1 "*) return 0;; esac; return 1; }

# Fresh load.csv for every output folder this run writes to (no stale rows from earlier runs).
for part in $PARTS; do
  case $part in locks|shortcs|backoff) d=locks;; fairness) d=counter;; *) d=$part;; esac
  rm -f "benchmarks/$d/$PLATFORM/load.csv"
done

# Warn loudly if a CSV got no data rows (e.g. a launcher such as numactl failed for every point).
check_rows() {
  [ -f "$1" ] && [ "$(wc -l < "$1")" -le 1 ] && echo "WARNING: $1 has no data rows" >&2
  return 0
}
out_dir() {
  local dir=benchmarks/$1/$PLATFORM
  mkdir -p "$dir"
  platform_info > "$dir/platform.txt"
  echo "$dir"
}

if has locks; then
  csv=$(out_dir locks)/locks.csv; echo "lock,threads,iters,outside,n,rep,seconds" > "$csv"
  for lock in $LOCKS; do
    for outside_work in $OWS; do
      for threads in $THREADS; do
        log_load "locks:$lock:ow=$outside_work:t=$threads" "$(dirname "$csv")"
        $RUN "$BUILD"/bench_lock --lock $lock --threads $threads --iters "$ITERS" --outside $outside_work --n 10007 $BENCH_ARGS \
                            --reps "$REPS" --timeout "$TIMEOUT" >> "$csv" 2>/dev/null
        echo "locks: $lock ow=$outside_work t=$threads"
      done
    done
  done
fi

if has shortcs; then
  csv=$(out_dir locks)/locks_shortcs.csv; echo "lock,threads,iters,outside,n,rep,seconds" > "$csv"
  for n in $SHORT_NS; do
    for lock in $LOCKS; do
      for threads in $THREADS; do
        [ "$threads" -gt "$NCPU" ] && continue
        log_load "shortcs:$lock:n=$n:t=$threads" "$(dirname "$csv")"
        $RUN "$BUILD"/bench_lock --lock $lock --threads $threads --iters "$SHORT_ITERS" --outside 0 --n $n $BENCH_ARGS \
                            --reps "$REPS" --timeout "$TIMEOUT" >> "$csv" 2>/dev/null
      done
      echo "shortcs: n=$n $lock"
    done
  done
fi

if has backoff; then
  csv=$(out_dir locks)/backoff.csv; echo "lock,threads,iters,outside,n,rep,seconds,max_delay" > "$csv"
  for lock in tas_bo ttas_bo; do
    for max_delay in 16 64 256 1024 4096 16384; do
      log_load "backoff:$lock:max_delay=$max_delay" "$(dirname "$csv")"
      $RUN "$BUILD"/bench_lock --lock $lock --threads "$NCPU" --iters "$ITERS" --outside 0 --n 10007 $BENCH_ARGS \
                          --reps "$REPS" --max_delay $max_delay --timeout "$TIMEOUT" 2>/dev/null | sed "s/$/,$max_delay/" >> "$csv"
      echo "backoff: $lock max_delay=$max_delay"
    done
  done
fi

if has barrier; then
  csv=$(out_dir barrier)/barrier.csv; echo "barrier,threads,episodes,rep,seconds,episodes_per_sec" > "$csv"
  barriers=${BARRIERS:-"sr sr_bo std omp"}; [ -z "${BARRIERS:-}" ] && [ "$(uname)" = Linux ] && barriers="$barriers pthread"
  for barrier in $barriers; do
    for threads in $THREADS; do
      log_load "barrier:$barrier:t=$threads" "$(dirname "$csv")"
      $RUN "$BUILD"/bench_barrier --barrier $barrier --threads $threads --episodes "$EPISODES" --reps "$REPS" \
                             --timeout "$TIMEOUT" >> "$csv" 2>/dev/null
      echo "barrier: $barrier t=$threads"
    done
  done
fi

if has counter; then
  csv=$(out_dir counter)/counter.csv; echo "counter,threads,total,rep,seconds,mops_per_sec" > "$csv"
  for counter in $COUNTERS; do
    for threads in $THREADS; do
      "$BUILD"/bench_counter --counter $counter --threads $threads --total "$TOTAL" --reps "$REPS" \
                             --timeout "$TIMEOUT" >> "$csv" 2>/dev/null
      echo "counter: $counter t=$threads"
    done
  done
fi

if has fairness; then
  csv=$(out_dir counter)/fairness.csv; echo "counter,threads,seconds,total_ops,min_ops,max_ops,jain" > "$csv"
  for counter in $COUNTERS; do
    for threads in $THREADS; do
      "$BUILD"/bench_fairness --counter $counter --threads $threads --seconds "$FAIR_SECONDS" \
                              --timeout "$TIMEOUT" >> "$csv" 2>/dev/null
      echo "fairness: $counter t=$threads"
    done
  done
fi

if has mpi && command -v mpirun >/dev/null && [ -x "$BUILD/mpi_lock" ]; then
  export OMPI_MCA_rmaps_base_oversubscribe=1
  MPIRUN="mpirun"; mpirun --version 2>&1 | grep -qi "open mpi" && MPIRUN="mpirun --oversubscribe"
  OUT=$(out_dir mpi)
  lock_csv=$OUT/mpi_lock.csv;       echo "lock,procs,iters_per_rank,seconds,acq_per_sec,correct" > "$lock_csv"
  barrier_csv=$OUT/mpi_barrier.csv; echo "barrier,procs,episodes,seconds,episodes_per_sec,correct" > "$barrier_csv"
  counter_csv=$OUT/mpi_counter.csv; echo "counter,procs,ops_per_rank,seconds,mops_per_sec,correct" > "$counter_csv"
  for procs in $MPI_PROCS; do
    log_load "mpi:p=$procs" "$OUT"
    for rep in $(seq "$REPS"); do
      for variant in faa cas cas_bo;  do $MPIRUN -np $procs "$BUILD"/mpi_counter --variant $variant --ops 20000 --timeout "$TIMEOUT" </dev/null 2>/dev/null | tail -1 >> "$counter_csv"; done
      for variant in tas mcs winlock; do $MPIRUN -np $procs "$BUILD"/mpi_lock --variant $variant --iters 10000 --timeout "$TIMEOUT" </dev/null 2>/dev/null | tail -1 >> "$lock_csv"; done
      for variant in sr sr_bo mpi;       do $MPIRUN -np $procs "$BUILD"/mpi_barrier --variant $variant --episodes 5000 --check 1 --timeout "$TIMEOUT" </dev/null 2>/dev/null | tail -1 >> "$barrier_csv"; done
    done
    echo "mpi: p=$procs"
  done
fi
for f in benchmarks/*/"$PLATFORM"/*.csv; do case $f in */load.csv) ;; *) check_rows "$f";; esac; done
echo "done -> benchmarks/*/$PLATFORM"
