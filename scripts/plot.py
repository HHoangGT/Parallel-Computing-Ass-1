# /// script
# requires-python = ">=3.9"
# dependencies = ["pandas", "matplotlib", "tabulate"]
# ///
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

ROOT = Path(__file__).resolve().parent.parent / "benchmarks"
LOCK_ORDER = ["tas", "ttas", "tas_bo", "ttas_bo", "ticket", "mcs", "std_mutex", "pthread", "omp"]
LOCK_COUNTERS = ["lock_tas", "lock_ttas", "lock_tas_bo", "lock_ttas_bo", "lock_ticket", "lock_mcs",
                 "lock_mutex", "lock_pthread", "lock_omp"]
LOCKFREE_COUNTERS = ["cas", "cas_bo", "faa", "sharded"]
plt.rcParams.update({"figure.dpi": 130, "axes.grid": True, "grid.alpha": 0.3, "font.size": 9})


def load(path, cols):
    if not path.exists():
        return None, 0
    df = pd.read_csv(path)
    df[cols] = df[cols].apply(pd.to_numeric, errors="coerce")
    bad = df[cols].isna().any(axis=1) | (df["rep"] < 0 if "rep" in df else False)
    return df[~bad], int(bad.sum())


def ncpu_of(plat_dir):
    for line in (plat_dir / "platform.txt").read_text().splitlines():
        if line.startswith("logical_cpus:"):
            return int(line.split(":")[1])
    return None


def vline(ax, ncpu):
    if ncpu:
        ax.axvline(ncpu, color="gray", ls=":", lw=1)
        ax.text(ncpu, ax.get_ylim()[1], " #cores", color="gray", va="top", fontsize=7)


def median_table(df, value, order):
    table = df.pivot_table(index="threads", columns="counter", values=value, aggfunc="median")
    return table[[counter for counter in order if counter in table]]


def plot_locks(plat_dir, read, summary, name, ncpu):
    df = read("locks.csv", ["seconds"])
    if df is not None:
        med = df.groupby(["lock", "outside", "threads"])["seconds"].median().reset_index()
        locks = [lock for lock in LOCK_ORDER if lock in set(med["lock"])]
        outside_works = sorted(med["outside"].unique())
        fig, axes = plt.subplots(1, len(outside_works), figsize=(3.4 * len(outside_works), 3.4), sharey=True)
        for ax, outside_work in zip(axes if len(outside_works) > 1 else [axes], outside_works):
            for lock in locks:
                series = med[(med["lock"] == lock) & (med["outside"] == outside_work)]
                ax.plot(series["threads"], series["seconds"], marker="o", ms=3, label=lock)
            ax.set_yscale("log")
            ax.set_title(f"OUTSIDE_WORK={outside_work}  (CS = 1/{outside_work + 1} of work)")
            ax.set_xlabel("threads")
            vline(ax, ncpu)
        axes[0].set_ylabel("execution time [s] (median, log)")
        axes[-1].legend(fontsize=7, loc="upper left", bbox_to_anchor=(1, 1))
        fig.suptitle(f"{name}: mutex locks - whole-program execution time")
        fig.tight_layout()
        fig.savefig(plat_dir / "fig_locks.png", bbox_inches="tight")
        plt.close(fig)
        table = med[med["threads"] == (ncpu or med["threads"].max())].pivot(index="lock", columns="outside", values="seconds")
        summary.append(f"## Locks: median seconds at {ncpu} threads (rows = lock, cols = OUTSIDE_WORK)\n\n" + table.round(4).to_markdown() + "\n")

    df = read("locks_shortcs.csv", ["seconds"])
    if df is not None:
        df["us_per_cs"] = df["seconds"] / (df["threads"] * df["iters"]) * 1e6
        med = df.groupby(["n", "lock", "threads"])["us_per_cs"].median().reset_index()
        locks = [lock for lock in LOCK_ORDER if lock in set(med["lock"])]
        ns = sorted(med["n"].unique(), reverse=True)
        fig, axes = plt.subplots(1, len(ns), figsize=(5 * len(ns), 3.6), sharey=False)
        axes = axes if len(ns) > 1 else [axes]
        for ax, n in zip(axes, ns):
            for lock in locks:
                series = med[(med["lock"] == lock) & (med["n"] == n)]
                ax.plot(series["threads"], series["us_per_cs"], marker="o", ms=3, label=lock)
            ax.set_yscale("log"); ax.set_xlabel("threads"); ax.set_ylabel("µs per critical section (log)")
            ax.set_title(f"is_prime({n}), OUTSIDE_WORK=0"); vline(ax, ncpu)
        axes[-1].legend(fontsize=7, loc="upper left", bbox_to_anchor=(1, 1))
        fig.suptitle(f"{name}: lock hand-off cost with a short critical section")
        fig.tight_layout(); fig.savefig(plat_dir / "fig_locks_shortcs.png", bbox_inches="tight"); plt.close(fig)
        for n in ns:
            table = med[med["n"] == n].pivot(index="threads", columns="lock", values="us_per_cs")
            summary.append(f"## Short CS, n={n}: median µs per critical section\n\n" + table.round(3).to_markdown() + "\n")

    df = read("backoff.csv", ["seconds"])
    if df is not None:
        med = df.groupby(["lock", "max_delay"])["seconds"].median().reset_index()
        fig, ax = plt.subplots(figsize=(4.5, 3.2))
        for lock, series in med.groupby("lock"):
            ax.plot(series["max_delay"], series["seconds"], marker="o", label=lock)
        ax.set_xscale("log", base=2)
        ax.set_xlabel("MAX_DELAY (pause iterations)")
        ax.set_ylabel("execution time [s]")
        ax.set_title(f"{name}: backoff tuning, {ncpu} threads, OUTSIDE_WORK=0")
        ax.legend()
        fig.tight_layout()
        fig.savefig(plat_dir / "fig_backoff.png", bbox_inches="tight")
        plt.close(fig)
        summary.append("## Backoff tuning (median s)\n\n" + med.pivot(index="max_delay", columns="lock", values="seconds").round(4).to_markdown() + "\n")


def plot_barrier(plat_dir, read, summary, name, ncpu):
    df = read("barrier.csv", ["seconds", "episodes_per_sec"])
    if df is None:
        return
    med = df.groupby(["barrier", "threads"])["episodes_per_sec"].median().reset_index()
    fig, ax = plt.subplots(figsize=(5, 3.4))
    for barrier, series in med.groupby("barrier"):
        ax.plot(series["threads"], series["episodes_per_sec"], marker="o", ms=3, label=barrier)
    ax.set_yscale("log")
    ax.set_xlabel("threads")
    ax.set_ylabel("barrier episodes / second (log)")
    ax.set_title(f"{name}: barrier throughput (no work between barriers)")
    vline(ax, ncpu)
    ax.legend()
    fig.tight_layout()
    fig.savefig(plat_dir / "fig_barrier.png")
    plt.close(fig)
    summary.append("## Barrier: median episodes/s\n\n" + med.pivot(index="threads", columns="barrier", values="episodes_per_sec").round(0).to_markdown() + "\n")


def counter_figure(plat_dir, filename, df, value, ylabel, title, ncpu, setup, transform=lambda table: table, sharey=True):
    fig, axes = plt.subplots(1, 2, figsize=(10, 3.8), sharey=sharey)
    for ax, order, panel in zip(axes, (LOCK_COUNTERS, LOCKFREE_COUNTERS), ("lock-based", "lock-free")):
        table = transform(median_table(df, value, order))
        for counter in table:
            series = table[counter].dropna()
            ax.plot(series.index, series.values, marker="o", ms=3, label=counter)
        setup(ax)
        ax.set_xlabel("threads"); ax.set_title(panel)
        ax.legend(fontsize=7, loc="upper center", bbox_to_anchor=(0.5, -0.18), ncol=3)
    for ax in axes:
        vline(ax, ncpu)
    axes[0].set_ylabel(ylabel)
    if not sharey:
        axes[1].set_ylabel(ylabel)
    fig.suptitle(title)
    fig.savefig(plat_dir / filename, bbox_inches="tight"); plt.close(fig)


def plot_counter(plat_dir, read, summary, name, ncpu):
    order = LOCK_COUNTERS + LOCKFREE_COUNTERS
    df = read("counter.csv", ["seconds", "mops_per_sec"])
    if df is not None:
        total = int(df["total"].iloc[0])
        counter_figure(plat_dir, "fig_time.png", df, "seconds", "execution time [s] (median, log)",
                       f"{name}: concurrent counters, {total:,} increments in total", ncpu,
                       lambda ax: ax.set_yscale("log"))
        if 1 in set(df["threads"]):
            counter_figure(plat_dir, "fig_speedup.png", df, "seconds", "speedup T_1 / T_p",
                           f"{name}: speedup over each counter's own 1-thread time", ncpu,
                           lambda ax: ax.axhline(1, color="k", lw=0.8), lambda table: table.loc[1] / table, sharey=False)
        summary += ["## Counters: median execution time [s]\n\n" + median_table(df, "seconds", order).round(4).to_markdown() + "\n",
                    "## Counters: median throughput [M ops/s]\n\n" + median_table(df, "mops_per_sec", order).round(2).to_markdown() + "\n"]

    df = read("fairness.csv", ["total_ops", "min_ops", "max_ops", "jain"])
    if df is not None:
        counter_figure(plat_dir, "fig_jain.png", df, "jain", "Jain fairness index",
                       f"{name}: fairness of per-thread operation counts", ncpu, lambda ax: ax.set_ylim(0, 1.05))
        min_max = median_table(df, "min_ops", order).map("{:.0f}".format) + " / " + median_table(df, "max_ops", order).map("{:.0f}".format)
        summary += ["## Fairness: Jain index\n\n" + median_table(df, "jain", order).round(3).to_markdown() + "\n",
                    "## Fairness: per-thread operations, min / max\n\n" + min_max.to_markdown() + "\n"]


def plot_mpi(plat_dir, read, summary, name, ncpu):
    parts = [("mpi_lock.csv", "lock", "acq_per_sec", "lock acquisitions/s"),
             ("mpi_barrier.csv", "barrier", "episodes_per_sec", "episodes/s")]
    present = [part for part in parts if (plat_dir / part[0]).exists()]
    if not present:
        return
    fig, axes = plt.subplots(1, len(present), figsize=(4 * len(present), 3.2))
    axes = axes if len(present) > 1 else [axes]
    for ax, (filename, key, metric, label) in zip(axes, present):
        df = pd.read_csv(plat_dir / filename)
        n_timeouts = int((df["correct"].astype(str) == "timeout").sum())
        df = df[df["correct"].astype(str) != "timeout"]
        med = df.groupby([key, "procs"])[metric].median().reset_index()
        for variant, series in med.groupby(key):
            ax.plot(series["procs"], series[metric], marker="o", label=variant)
        ax.set_yscale("log"); ax.set_xlabel("MPI processes"); ax.set_ylabel(label); ax.legend(fontsize=7)
        ax.set_title(filename.replace(".csv", ""))
        bad = df[df["correct"].astype(str) == "NO"]
        summary.append(f"## {filename}: median {metric}\n\n" + med.pivot(index="procs", columns=key, values=metric).round(2).to_markdown()
                       + ("\n\n**Correctness failures:** " + str(len(bad)) if len(bad) else "\n\nAll completed runs passed the correctness check.")
                       + (f" {n_timeouts} run(s) hit the timeout and are excluded." if n_timeouts else "") + "\n")
    fig.suptitle(f"{name}: MPI-3 one-sided versions")
    fig.tight_layout()
    fig.savefig(plat_dir / "fig_mpi.png")
    plt.close(fig)


PARTS = {"locks": plot_locks, "barrier": plot_barrier, "counter": plot_counter, "mpi": plot_mpi}


def plot_platform(part, plat_dir):
    name, ncpu = plat_dir.name, ncpu_of(plat_dir)
    summary = [f"# {part} / {name}: benchmark summary (medians over repetitions)\n",
               f"```\n{(plat_dir / 'platform.txt').read_text().strip()}\n```\n"]
    dropped = []

    def read(filename, cols):
        df, n_timeouts = load(plat_dir / filename, cols)
        if df is not None:
            dropped.append(f"{filename}: {n_timeouts}")
        return df if df is not None and len(df) else None

    PARTS[part](plat_dir, read, summary, name, ncpu)
    if dropped:
        summary.append("TIMEOUT rows dropped: " + ", ".join(dropped) + ".\n")
    (plat_dir / "summary.md").write_text("\n".join(summary))
    print("  wrote", plat_dir / "summary.md")


def compare_locks(platforms):
    rows = []
    for plat_dir in platforms:
        df, _ = load(plat_dir / "locks.csv", ["seconds"])
        if df is None:
            continue
        ncpu = ncpu_of(plat_dir)
        med = df[(df["outside"] == 0) & (df["threads"] == ncpu)].groupby("lock")["seconds"].median()
        rows.append(med.rename(f"{plat_dir.name} ({ncpu} thr)"))
    if len(rows) < 2:
        return
    table = pd.concat(rows, axis=1)
    table = table.reindex([lock for lock in LOCK_ORDER if lock in table.index])
    ax = table.plot.bar(figsize=(7, 3.4), logy=True)
    ax.set_ylabel("execution time [s] (log)")
    ax.set_title("Locks across platforms (all cores busy, OUTSIDE_WORK=0)")
    plt.tight_layout()
    plt.savefig(ROOT / "locks" / "compare_locks.png")
    plt.close()
    (ROOT / "locks" / "summary.md").write_text("# Cross-platform: locks at #cores, OUTSIDE_WORK=0 (median s)\n\n" + table.round(4).to_markdown() + "\n")
    print("wrote", ROOT / "locks" / "compare_locks.png")


def main():
    names = sys.argv[1:]
    done = 0
    for part in PARTS:
        part_dir = ROOT / part
        if not part_dir.is_dir():
            continue
        platforms = [plat_dir for plat_dir in sorted(part_dir.iterdir())
                     if (plat_dir / "platform.txt").exists() and (not names or plat_dir.name in names)]
        for plat_dir in platforms:
            print("plotting", part, plat_dir.name)
            plot_platform(part, plat_dir)
            done += 1
        if part == "locks":
            compare_locks(platforms)
    if not done:
        print("nothing to do: no benchmarks/<part>/<platform>/platform.txt found" + (f" for {names}" if names else ""))


if __name__ == "__main__":
    main()
