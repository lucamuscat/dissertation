import pandas as pd
import matplotlib.pyplot as plt
import os
from typing import List

images_path = "src/utils/images/"

os.makedirs(images_path, exist_ok=True)

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

individual_queue_markers = {
    0: ("8", "solid"),
    250: ("s", "dashdot"),
    500: ("+", "dashed"),
    750: ("*", "dotted"),
    1000: ("x", (0, (3, 1, 1, 1, 1, 1)))
}


def plot_results(file_name:str, title:str, output_file_name_prefix=""):
    runtime_key = "net_runtime_s"
    readings = pd.read_csv(f"{file_name}")
    queue_names = sorted(readings["name"].unique())
    max_threads = readings["threads"].max()
    grouped_readings = readings.groupby(["name", "delay", "threads"])
    thread_axis = list(range(1, max_threads + 1))
    delay_times = sorted(readings["delay"].unique())
    #print(mean_readings.get(queue_names[0]).keys())
    # Could be made cleaner using agg()
    min_readings = grouped_readings[runtime_key].min()
    max_readings = grouped_readings[runtime_key].max()
    mean_readings = grouped_readings[runtime_key].mean()
    for queue_name in queue_names:
        current_mean_readings = mean_readings.get(queue_name)
        current_min_readings = min_readings.get(queue_name)
        current_max_readings = max_readings.get(queue_name)
        fig, ax = plt.subplots()
        ax.set_title(queue_name + f" {title}")
        ax.set_xlabel("Threads")
        ax.set_ylabel("Total Runtime (s)")
        for delay_time in delay_times:
            temp_total_runtime = current_mean_readings.get(delay_time)
            temp_min_runtime = current_min_readings.get(delay_time)
            temp_max_runtime = current_max_readings.get(delay_time)
            
            marker, linestyle = individual_queue_markers[delay_time]
            ax.errorbar(
                thread_axis,
                temp_total_runtime.to_list(),
                yerr=[temp_total_runtime-temp_min_runtime, temp_max_runtime-temp_total_runtime],
                label = f"{delay_time}ns",
                marker = marker,
                linestyle = linestyle,
                markersize = markersize,
                linewidth = linewidth,
                # ecolor="k",
                capsize=2
            )
            
        
        ax.legend()
        cleaned_queue_name = queue_name.lower().replace(" ", "_")
        fig.savefig(f"{images_path}{output_file_name_prefix}{cleaned_queue_name}.png")
        # https://stackoverflow.com/a/39116381
        plt.gca().set_prop_cycle(None)
        plt.close()
        
plot_results("enqueue_dequeue_results.csv", "Enqueue Dequeue Benchmark")
plot_results("p_enqueue_dequeue_results.csv", "50% Enqueue Dequeue", "p_")
