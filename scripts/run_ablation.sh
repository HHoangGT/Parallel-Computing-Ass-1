#!/usr/bin/env bash
# Ablations; each writes to benchmarks/<part>/<MACHINE>__<condition>/.
#   MACHINE=linux-a6000 ./scripts/run_ablation.sh placement padding seqcst backoff
# padding: -DCACHE_LINE=8; seqcst: -DSEQ_CST=ON; backoff: sleep_for backoff;
# placement (Linux): T threads pinned to 1 or 2 sockets, with or without SMT siblings.
# Knobs: MACHINE, REPS (3), TIMEOUT (60), T, ABL_THREADS, ABL_LOCKS.
set -u
cd "$(dirname "$0")/.."
MACHINE=${MACHINE:?set MACHINE, e.g. linux-a6000}
REPS=${REPS:-3}
TIMEOUT=${TIMEOUT:-60}
ABL_LOCKS=${ABL_LOCKS:-"tas ttas tas_bo ttas_bo ticket mcs std_mutex"}
ABL_BARRIERS=${ABL_BARRIERS:-"sr std"}
NCPU=$( (nproc 2>/dev/null || sysctl -n hw.ncpu) | head -1)
ABL_THREADS=${ABL_THREADS:-$(for t in 1 2 4 8 16 $((NCPU / 4)) $((NCPU / 2)) $NCPU; do [ $t -le $NCPU ] && echo $t; done | sort -n -u | tr '\n' ' ')}
export REPS TIMEOUT

build_variant() {  # build_variant <dir> <cmake flags...>
  local dir=$1; shift
  cmake -S . -B "$dir" "$@" >/dev/null || exit 1   # always configure: never reuse stale options
  cmake --build "$dir" -j >/dev/null || exit 1
}

short_cs_run() {  # short_cs_run <build dir> <condition> [RUN prefix] [threads]
  BUILD=$1 PLATFORM="${MACHINE}__$2" RUN="${3:-}" THREADS="${4:-$ABL_THREADS}" \
    PARTS="shortcs barrier" SHORT_NS=101 LOCKS="$ABL_LOCKS" BARRIERS="$ABL_BARRIERS" \
    ./scripts/run_all.sh
}

# CPU list for a placement: <sockets 1|2> <smt 0|1> <threads>. Uses lscpu -p (CPU,CORE,SOCKET).
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

for study in "$@"; do
  case $study in
    padding)
      build_variant build-nopad -DCACHE_LINE=8
      short_cs_run build-nopad nopad ;;
    seqcst)
      build_variant build-seqcst -DSEQ_CST=ON
      short_cs_run build-seqcst seqcst ;;
    backoff)
      BUILD=build PLATFORM="${MACHINE}__sleep" PARTS=backoff BENCH_ARGS="--sleep 1" ./scripts/run_all.sh ;;
    placement)
      [ "$(uname)" = Linux ] || { echo "placement needs Linux (numactl)"; continue; }
      command -v numactl >/dev/null || { echo "placement needs numactl"; exit 1; }
      sockets=$(lscpu -p=SOCKET | grep -v '^#' | sort -u | wc -l)
      cores_per_socket=$(( $(lscpu -p=CORE | grep -v '^#' | sort -u | wc -l) / sockets ))
      T=${T:-$cores_per_socket}   # fits every layout: 1 socket without SMT uses all its cores
      for layout in "1 0 1s-nosmt" "1 1 1s-smt" "2 0 2s-nosmt" "2 1 2s-smt"; do
        set -- $layout
        cpus=$(cpu_list "$1" "$2" "$T")
        echo "placement $3: T=$T cpus=$cpus"
        # sanity: exactly T CPUs, and numactl accepts the set (otherwise every point would fail silently)
        [ "$(echo "$cpus" | tr ',' '\n' | grep -c .)" -eq "$T" ] || { echo "placement $3: got wrong CPU count"; exit 1; }
        numactl --physcpubind="$cpus" true || { echo "placement $3: numactl rejected the CPU set"; exit 1; }
        short_cs_run build "placement-$3" "numactl --physcpubind=$cpus" "$T"
      done ;;
    *) echo "unknown study '$study'"; exit 1 ;;
  esac
done
echo "ablations done -> benchmarks/*/${MACHINE}__*"
