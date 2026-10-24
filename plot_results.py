import os

import pandas as pd
import matplotlib.pyplot as plt

# Folder with times.csv and where the figures will be saved
# (created if it does not exist yet)
output_dir = "graphs"
os.makedirs(output_dir, exist_ok=True)

# Reads the average times (in seconds), one row per instance.
# Expected CSV header: instance,sequential,1,2,4,8
# (the numbers are the thread counts of the parallel version)
df = pd.read_csv(os.path.join(output_dir, "times.csv"))
threads = [1, 2, 4, 8]
columns = ["sequential"] + [str(t) for t in threads]

# ---------- Figure 1: average execution time per instance ----------
ax = df.set_index("instance")[columns].plot(kind="bar", figsize=(10, 5))
ax.set_ylabel("Tempo médio (s)")
ax.set_xlabel("Instância")
ax.legend(["Sequencial"] + [f"{t} thread(s)" for t in threads])
plt.xticks(rotation=0)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "fig_time.png"), dpi=200)
plt.close()

# ---------- Figure 2: speedup = sequential time / parallel time ----------
plt.figure(figsize=(7, 5))
for _, row in df.iterrows():
    # One speedup value per thread count for this instance
    speedup = [row["sequential"] / row[str(t)] for t in threads]
    plt.plot(threads, speedup, marker="o", label=row["instance"])

# Ideal speedup: doubling the threads doubles the speed
plt.plot(threads, threads, "k--", label="Ideal")

plt.xticks(threads)
plt.xlabel("Número de threads")
plt.ylabel("Speedup")
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "fig_speedup.png"), dpi=200)

# ---------- Figure 3: cost of the parallel version relative to the sequential one ----------
# Costs have very different scales between instances, so each value is divided by the
# sequential average of its own instance (sequential = 100%).
cost_df = pd.read_csv(os.path.join(output_dir, "costs.csv")).set_index("instance")
relative_cost = cost_df[[str(t) for t in threads]].div(cost_df["sequential"], axis=0) * 100

ax = relative_cost.plot(kind="bar", figsize=(10, 5))
ax.axhline(100, color="k", linestyle="--", linewidth=1)  # sequential reference line
ax.set_ylim(0, 110)
ax.set_ylabel("Custo em relação ao sequencial (%)")
ax.set_xlabel("Instância")
ax.legend([f"{t} thread(s)" for t in threads] + ["Sequencial (100%)"], loc="lower right")
plt.xticks(rotation=0)
plt.tight_layout()
plt.savefig(os.path.join(output_dir, "fig_cost.png"), dpi=200)
plt.close()
