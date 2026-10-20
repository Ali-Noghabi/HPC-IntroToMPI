#!/usr/bin/env python3
"""Reads outputs/results.csv and produces, for every n:
   - speedup, efficiency and time-breakdown plots  -> report/figures/
   - timing tables (markdown printed, LaTeX saved)  -> report/tables/
   - outputs/summary.csv (median over repetitions, speedup, efficiency)

Usage: python3 analyze.py [results.csv]      (needs pandas and matplotlib)
"""
import os
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

csv_path = sys.argv[1] if len(sys.argv) > 1 else "outputs/results.csv"
fig_dir, tab_dir = "report/figures", "report/tables"
os.makedirs(fig_dir, exist_ok=True)
os.makedirs(tab_dir, exist_ok=True)

df = pd.read_csv(csv_path)

# Median over the repetitions of every (n, mode, ranks) combination
cols = ["init_s", "calc_s", "reduce_s", "total_s"]
med = df.groupby(["n", "bc_parallel", "ranks"])[cols].median().reset_index()

# Speedup and efficiency with respect to the run on 1 rank (same n and mode)
rows = []
for (n, bc), g in med.groupby(["n", "bc_parallel"]):
    g = g.sort_values("ranks").copy()
    if 1 not in set(g["ranks"]):
        print(f"warning: no 1-rank run for n={n}, bc={bc}, skipping")
        continue
    base = g[g["ranks"] == 1].iloc[0]
    g["speedup_total"] = base["total_s"] / g["total_s"]
    g["speedup_calc"] = base["calc_s"] / g["calc_s"]
    g["eff_total"] = g["speedup_total"] / g["ranks"]
    g["eff_calc"] = g["speedup_calc"] / g["ranks"]
    rows.append(g)
summary = pd.concat(rows)
summary.to_csv("outputs/summary.csv", index=False, float_format="%.6g")


def latex_table(g):
    lines = [r"\begin{tabular}{rrrrrrrr}", r"\hline",
             r"$P$ & init [s] & calc [s] & reduce [s] & total [s] & $S_{tot}$ & $S_{calc}$ & $E_{tot}$ \\",
             r"\hline"]
    for _, r in g.iterrows():
        lines.append(f"{int(r.ranks)} & {r.init_s:.4f} & {r.calc_s:.4f} & {r.reduce_s:.4f} & "
                     f"{r.total_s:.4f} & {r.speedup_total:.2f} & {r.speedup_calc:.2f} & {r.eff_total:.2f} \\\\")
    lines += [r"\hline", r"\end{tabular}"]
    return "\n".join(lines)


for n, gn in summary.groupby("n"):
    # tables
    for bc, g in gn.groupby("bc_parallel"):
        print(f"\n### n = {n}, bc_parallel = {bc}")
        print(g[["ranks"] + cols + ["speedup_total", "speedup_calc", "eff_total"]]
              .to_markdown(index=False, floatfmt=".4f"))
        with open(f"{tab_dir}/table_n{n}_bc{bc}.tex", "w") as f:
            f.write(latex_table(g) + "\n")

    # speedup and efficiency (solid = B*c parallel, dashed = B*c on rank 0)
    fig, ax = plt.subplots(1, 2, figsize=(10, 4))
    pmax = gn["ranks"].max()
    ax[0].plot([1, pmax], [1, pmax], "k:", label="ideal")
    ax[1].axhline(1, color="k", linestyle=":", label="ideal")
    for bc, g in gn.groupby("bc_parallel"):
        ls = "-" if bc == 1 else "--"
        tag = "B*c parallel" if bc == 1 else "B*c on rank 0"
        ax[0].plot(g.ranks, g.speedup_total, "o" + ls, color="C0", label=f"total, {tag}")
        ax[0].plot(g.ranks, g.speedup_calc, "s" + ls, color="C1", label=f"calculation, {tag}")
        ax[1].plot(g.ranks, g.eff_total, "o" + ls, color="C0", label=f"total, {tag}")
        ax[1].plot(g.ranks, g.eff_calc, "s" + ls, color="C1", label=f"calculation, {tag}")
    ax[0].set(xscale="log", yscale="log", xlabel="MPI processes P", ylabel="Speed up", title=f"Speed up (n = {n})")
    ax[1].set(xscale="log", xlabel="MPI processes P", ylabel="Efficiency", title=f"Efficiency (n = {n})")
    for a in ax:
        a.set_xticks(sorted(gn.ranks.unique()))
        a.set_xticklabels(sorted(gn.ranks.unique()))
        a.grid(alpha=0.3)
    ax[0].legend(fontsize=7)
    fig.tight_layout()
    for ext in ("pdf", "png"):
        fig.savefig(f"{fig_dir}/scaling_n{n}.{ext}", dpi=150)
    plt.close(fig)

    # time breakdown (B*c parallel)
    g = gn[gn.bc_parallel == 1].sort_values("ranks")
    if len(g):
        fig, a = plt.subplots(figsize=(6, 4))
        x = range(len(g))
        bottom = [0] * len(g)
        for c, lab in zip(["init_s", "calc_s", "reduce_s"], ["initialization", "calculation", "reduction"]):
            a.bar(x, g[c], bottom=bottom, label=lab)
            bottom = [b + v for b, v in zip(bottom, g[c])]
        a.set_xticks(list(x))
        a.set_xticklabels(g.ranks)
        a.set(xlabel="MPI processes P", ylabel="time [s] (max over ranks)", title=f"Time breakdown (n = {n})")
        a.legend()
        fig.tight_layout()
        for ext in ("pdf", "png"):
            fig.savefig(f"{fig_dir}/breakdown_n{n}.{ext}", dpi=150)
        plt.close(fig)

print(f"\nFigures in {fig_dir}/, tables in {tab_dir}/, summary in outputs/summary.csv")