import pandas as pd
import matplotlib.pyplot as plt

markersize = 4
linewidth = 0.5

queue_markers = {
    "Baskets Queue with DWCAS": ("8", "solid"),
    "Baskets Queue with tagged ptr": ("8", "dashdot"),
    "MS Two-Lock TTAS": ("*", "solid"),
    "MS Lock-Free Queue with DWCAS": ("+", "solid"),
    "MS Lock-Free Queue with tagged ptr": ("+", "dashdot"),
    "Valois Queue": ("x", "solid")
}

readings = pd.read_csv("enqueue_dequeue_results.csv")

readings_grouped_by_name = readings.groupby(["name", "threads"])
mean_readings = readings_grouped_by_name["net_runtime_s"].mean()
std_readings = readings_grouped_by_name["net_runtime_s"].std()
cov_readings = (std_readings/mean_readings)*100
print(cov_readings)

queue_names = sorted(list(set([x for (x, _) in mean_readings.keys()])))
print(queue_names)

fig, ax = plt.subplots()

for queue_name in queue_names:
    temp = mean_readings.get(queue_name)
    thread_nums = list(range(1, len(temp) + 1))
    (marker, line_style) = queue_markers[queue_name]
    ax.plot(
        thread_nums,
        temp.tolist(),
        'k',
        label=queue_name,
        markersize=markersize,
        marker=marker, 
        linestyle=line_style,
        linewidth=linewidth
    )

ax.set_title("$10^{7}$ Enqueue Dequeue Pairs (delay = 100ns)")
ax.set_xlabel("Threads")
ax.set_ylabel("Net Runtime (s)")
plt.legend()
plt.show()