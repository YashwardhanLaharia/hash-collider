import matplotlib.pyplot as plt
import numpy as np
import os

os.makedirs("figures", exist_ok=True)

threads = ["1", "2", "4", "8", "16", "32", "64", "96"]
phase_a = [1082.85, 545.35, 271.11, 206.44, 68.61, 34.42, 17.31, 11.72]
phase_b = [867.23, 254.31, 579.09, 398.70, 169.58, 63.55, 50.86, 4.26]
total = [a + b for a, b in zip(phase_a, phase_b)]

plt.style.use("seaborn-v0_8-whitegrid" if "seaborn-v0_8-whitegrid" in plt.style.available else "default")
fig, ax = plt.subplots(figsize=(9, 5), dpi=300)

width = 0.55
x = np.arange(len(threads))

color_a = "#2b5c8f"
color_b = "#d95f02"

ax.bar(x, phase_a, width, label="Phase A (Table Insertion)", color=color_a, edgecolor="none")
ax.bar(x, phase_b, width, bottom=phase_a, label="Phase B (Collision Search)", color=color_b, edgecolor="none")

# 15-minute threshold line
ax.axhline(900, color="#d9383a", linestyle="--", linewidth=1.5, alpha=0.8, label="15-minute Limit (900 s)")
ax.text(len(threads) - 0.5, 930, "15 min cutoff (900s)", color="#d9383a", fontweight="bold", fontsize=9, ha="right")

# Annotate total times above each bar
for i, tot in enumerate(total):
    ax.text(i, tot + 30, f"{tot:.1f}s", ha="center", va="bottom", fontsize=9.5, fontweight="bold", color="#222222")

ax.set_ylabel("Execution Time (seconds)", fontsize=11, fontweight="semibold")
ax.set_xlabel("Thread Count", fontsize=11, fontweight="semibold")
ax.set_title("Thread Scaling Benchmark (1_kilo): Phase A & Phase B Execution Time", fontsize=12, fontweight="bold", pad=14)
ax.set_xticks(x)
ax.set_xticklabels([f"{t} threads" if t != "1" else "1 thread" for t in threads], fontsize=9.5)
ax.set_ylim(0, 2200)

ax.grid(axis="y", linestyle=":", alpha=0.6)
ax.grid(axis="x", visible=False)
for spine in ["top", "right", "left", "bottom"]:
    ax.spines[spine].set_visible(False)

ax.legend(frameon=True, facecolor="white", edgecolor="#cccccc", fontsize=9.5, loc="upper right")

plt.tight_layout()
plt.savefig("figures/scaling_kilo.png", dpi=300)
print("Saved figures/scaling_kilo.png")
