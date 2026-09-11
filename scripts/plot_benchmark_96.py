import matplotlib.pyplot as plt
import numpy as np
import os

os.makedirs("figures", exist_ok=True)

pairs = ["1_kilo\n(64 KB)", "2_mega\n(128 KB)", "3_giga\n(224 KB)", "4_tera\n(336 KB)", "5_peta\n(480 KB)", "6_exa\n(900 KB)"]
phase_a = [11.72, 22.94, 39.82, 59.50, 84.81, 158.83]
phase_b = [4.27, 25.41, 14.80, 22.17, 158.20, 415.98]
total = [a + b for a, b in zip(phase_a, phase_b)]

plt.style.use("seaborn-v0_8-whitegrid" if "seaborn-v0_8-whitegrid" in plt.style.available else "default")
fig, ax = plt.subplots(figsize=(9, 5), dpi=300)

width = 0.55
x = np.arange(len(pairs))

color_a = "#2b5c8f"
color_b = "#d95f02"

ax.bar(x, phase_a, width, label="Phase A (Table Insertion)", color=color_a, edgecolor="none")
ax.bar(x, phase_b, width, bottom=phase_a, label="Phase B (Collision Search)", color=color_b, edgecolor="none")

# 15-minute threshold line
ax.axhline(900, color="#d9383a", linestyle="--", linewidth=1.5, alpha=0.8, label="15-minute Limit (900 s)")
ax.text(len(pairs) - 0.5, 915, "15 min cutoff (900s)", color="#d9383a", fontweight="bold", fontsize=9, ha="right")

# Annotate total times above each bar
for i, tot in enumerate(total):
    ax.text(i, tot + 18, f"{tot:.1f}s", ha="center", va="bottom", fontsize=9.5, fontweight="bold", color="#222222")

ax.set_ylabel("Execution Time (seconds)", fontsize=11, fontweight="semibold")
ax.set_xlabel("PDF Pair (File Size)", fontsize=11, fontweight="semibold")
ax.set_title("96-Thread Benchmark: Phase A & Phase B Execution Time by Difficulty", fontsize=12, fontweight="bold", pad=14)
ax.set_xticks(x)
ax.set_xticklabels(pairs, fontsize=9.5)
ax.set_ylim(0, 1000)

ax.grid(axis="y", linestyle=":", alpha=0.6)
ax.grid(axis="x", visible=False)
for spine in ["top", "right", "left", "bottom"]:
    ax.spines[spine].set_visible(False)

ax.legend(frameon=True, facecolor="white", edgecolor="#cccccc", fontsize=9.5, loc="upper left")

plt.tight_layout()
plt.savefig("figures/benchmark_96.png", dpi=300)
