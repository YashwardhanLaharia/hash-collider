import matplotlib.pyplot as plt
import numpy as np
import os

os.makedirs("figures", exist_ok=True)

threads = np.array([1, 2, 4, 8, 16, 32, 64, 96])
phase_a = np.array([1082.85, 545.35, 271.11, 206.44, 68.61, 34.42, 17.31, 11.72])
phase_b = np.array([867.23, 254.31, 579.09, 398.70, 169.58, 63.55, 50.86, 4.26])
total = phase_a + phase_b

phase_a_speedup = phase_a[0] / phase_a
total_speedup = total[0] / total

plt.style.use("seaborn-v0_8-whitegrid" if "seaborn-v0_8-whitegrid" in plt.style.available else "default")
fig, (time_ax, speedup_ax) = plt.subplots(1, 2, figsize=(11, 4.8), dpi=300)

color_a = "#2b5c8f"
color_b = "#d95f02"
color_total = "#222222"

# Logarithmic time axis preserves visibility across the large 1-to-96 range.
time_ax.plot(threads, phase_a, marker="o", linewidth=2, color=color_a, label="Phase A")
time_ax.plot(threads, phase_b, marker="o", linewidth=2, color=color_b, label="Phase B")
time_ax.plot(threads, total, marker="o", linewidth=2, color=color_total, label="Total")
time_ax.set_xscale("log", base=2)
time_ax.set_yscale("log")
time_ax.set_xticks(threads)
time_ax.set_xticklabels([str(t) for t in threads])
time_ax.set_xlabel("Threads", fontweight="semibold")
time_ax.set_ylabel("Mean time (seconds)", fontweight="semibold")
time_ax.set_title("Execution time", fontweight="bold")
time_ax.grid(which="both", linestyle=":", alpha=0.55)
time_ax.legend(frameon=True, facecolor="white", edgecolor="#cccccc", fontsize=9)

# Compare measured speedups with ideal fixed-work scaling.
speedup_ax.plot(threads, threads, linestyle="--", linewidth=1.7, color="#888888", label="Ideal linear")
speedup_ax.plot(threads, phase_a_speedup, marker="o", linewidth=2, color=color_a, label="Phase A")
speedup_ax.plot(threads, total_speedup, marker="o", linewidth=2, color=color_total, label="Total")
speedup_ax.set_xscale("log", base=2)
speedup_ax.set_xticks(threads)
speedup_ax.set_xticklabels([str(t) for t in threads])
speedup_ax.set_xlabel("Threads", fontweight="semibold")
speedup_ax.set_ylabel("Speedup relative to 1 thread", fontweight="semibold")
speedup_ax.set_title("Measured speedup", fontweight="bold")
speedup_ax.set_ylim(0, 130)
speedup_ax.grid(which="both", linestyle=":", alpha=0.55)
speedup_ax.legend(frameon=True, facecolor="white", edgecolor="#cccccc", fontsize=9)

fig.suptitle("Thread Scaling Benchmark: 1_kilo", fontsize=13, fontweight="bold")
fig.tight_layout()
fig.savefig("figures/scaling_kilo.png", dpi=300, bbox_inches="tight")
print("Saved figures/scaling_kilo.png")
