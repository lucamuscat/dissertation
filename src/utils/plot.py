"""
Automatically generates plots for the results obtained in the root directory.
"""

import os
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt

# Taken from: https://regenerativetoday.com/some-tricks-to-make-matplotlib-visualization-even-better/
# from mpl_toolkits.axes_grid1.inset_locator import mark_inset, inset_axes

IMAGES_PATH = "src/utils/images/"
MARKER_SIZE = 7
LINE_WIDTH = 0.7

os.makedirs(IMAGES_PATH, exist_ok=True)

sns_kwargs = {
    "markers": True,
    "dashes": True,
    "ci": "sd",
    "markeredgecolor": "none",
    "err_style": "bars",
    "linewidth": LINE_WIDTH,
    "markersize": MARKER_SIZE,
    "err_kws": {"linewidth": LINE_WIDTH, "capsize": 2, "ecolor": "k"},
}

labels_kwargs = {"xlabel": "Threads", "ylabel": "Net Runtime (s)"}

sns.set_style("whitegrid")
sns.set_context("paper")


def save_plot(ax: plt.Axes, output_file_name: str, plot_title: str, dpi: int):
    ax.set(title=plot_title, **labels_kwargs)
    ax.get_figure().savefig(output_file_name, dpi=dpi)
    ax.get_figure().clf()


def plot_grouped_results(
    delay: int,
    input_file_name: str,
    output_file_name: str,
    plot_title: str,
    dpi: int = 300,
):
    """
    Plot the readings of each queue for a specific delay, all in one graph.

    Args:
        delay:int - Chooses what readings to display based on the provided delay.
        input_file_name:str - The name of the csv file (excluding ".csv") where
        the data to be processed resides.
        output_file_name:str - The name of the outputted JPEG file.
        plot_title:str - The title of the generated plot.
        dpi:int - Controls the DPI that the JPEG file will be rendered in.

    """

    df = pd.read_csv(f"{input_file_name}.csv")
    individual_delay_df = df.query(f"delay == {str(delay)}").sort_values(by="name")
    palette = sns.color_palette(n_colors=len(df["name"].unique()))

    ax = sns.lineplot(
        data=individual_delay_df,
        x="threads",
        y="net_runtime_s",
        style="name",
        hue="name",
        palette=palette,
        **sns_kwargs,
    )

    save_plot(ax, f"{IMAGES_PATH}/{output_file_name}.jpg", plot_title, dpi)


def plot_individual_results(
    input_file_name: str, output_file_name: str, plot_title: str, dpi: int = 300
):
    """
    Plot the readings of each individual queue grouped by delay.

    Args:
        input_file_name:str - Name of the csv file that will be plotted.
        output_file_name:str - Name of the outputted JPEG file.
        plot_title:str - Title of the generated plot.
        dpi:int - Controls the DPI that the JPEG file will be rendered in.
    """
    df = pd.read_csv(f"{input_file_name}.csv", index_col="name")
    queue_names = df.index.unique().sort_values()
    palette = sns.color_palette(n_colors=len(df["delay"].unique()))

    for queue_name in queue_names:
        individual_df = df.loc[queue_name, ["net_runtime_s", "threads", "delay"]]
        individual_df.reset_index(drop=True, inplace=True)

        ax = sns.lineplot(
            data=individual_df,
            x="threads",
            y="net_runtime_s",
            style="delay",
            hue="delay",
            palette=palette,
            **sns_kwargs,
        )

        cleaned_queue_name = str.lower(queue_name).replace(" ", "_")

        file_path = f"{IMAGES_PATH}/{output_file_name}_{cleaned_queue_name}.jpg"
        title = f"{queue_name}: {plot_title}"
        save_plot(ax, file_path, title, dpi)


ENQUEUE_DEQUEUE_TITLE = "Pairwise Enqueue Dequeue Benchmark"
ENQEUUE_DEQUEUE_FILE_NAME = "enqueue_dequeue_results"
P_ENQUEUE_DEQUEUE_TITLE = "50% Enqueue Benchmark"
P_ENQUEUE_DEQUEUE_FILE_NAME = "p_enqueue_dequeue_results"

plot_grouped_results(
    250, "enqueue_dequeue_results", "grouped_enqueue_dequeue", ENQUEUE_DEQUEUE_TITLE
)
plot_grouped_results(
    250,
    "p_enqueue_dequeue_results",
    "p_grouped_enqueue_dequeue",
    P_ENQUEUE_DEQUEUE_TITLE,
)

plot_individual_results(
    "enqueue_dequeue_results", "enqueue_dequeue", ENQUEUE_DEQUEUE_TITLE
)
plot_individual_results(
    "p_enqueue_dequeue_results", "p_enqueue_dequeue", P_ENQUEUE_DEQUEUE_TITLE
)
