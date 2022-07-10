"""
Automatically generates plots for the results obtained in the root directory.
"""

import os
from typing import Union
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np

# Taken from: https://regenerativetoday.com/some-tricks-to-make-matplotlib-visualization-even-better/
# from mpl_toolkits.axes_grid1.inset_locator import mark_inset, inset_axes

IMAGES_PATH = "src/utils/images/"
MARKER_SIZE = 7
LINE_WIDTH = 0.7
COL_WRAP = 4
SUPTITLE_Y = 1.02

os.makedirs(IMAGES_PATH, exist_ok=True)

PLOT_CONTEXT = "paper"
# PLOT_CONTEXT = "poster"

err_style = {
    "err_style": "band" if PLOT_CONTEXT == "poster" else "bars",
}

if PLOT_CONTEXT != "poster":
    err_style["err_kws"] = {"linewidth": LINE_WIDTH, "capsize": 2, "ecolor": "k"}

sns_base_kwargs = {
    "markers": True,
    "dashes": True,
    "markeredgecolor": "none",
}

sns_kwargs = {
    **sns_base_kwargs,
    "markersize": MARKER_SIZE,
    "linewidth": LINE_WIDTH,
    "ci": "sd",
    **err_style,
}

labels_kwargs = {"xlabel": "Threads", "ylabel": "Net Runtime (s)"}

uom_color_palette = ["#9C0C35", "#E94C5E", "#F08590", "#F5B4C7", "#FAC18A"]

grid_style = "white" if PLOT_CONTEXT == "poster" else "whitegrid"

sns.set_style(grid_style)
sns.set_context(PLOT_CONTEXT)
# sns.set_palette(sns.color_palette(uom_color_palette))
sns.set_palette("colorblind")


def set_legend_location(ax: plt.Axes):
    if PLOT_CONTEXT == "poster":
        sns.move_legend(ax, "best", fontsize="small")
        return

def save_plot(
    ax: Union[plt.Axes, sns.FacetGrid],
    output_file_name: str,
    dpi: int,
):
    fig = ax.get_figure() if ax is plt.Axes else ax.figure
    fig.savefig(output_file_name, dpi=dpi, bbox_inches="tight")
    fig.clf()
    # Set figure to matplotlib default size to prevent figure size adjustments
    # from carrying on to independent figures.
    fig.set(size_inches=(6.4, 4.8))


def plot_grouped_results(
    input_file_name: str,
    output_file_name: str,
    plot_title: str,
    dpi: int = 300,
):
    """
    Plot the readings of each queue for a specific delay, all in one graph.

    Args:
        input_file_name:str - The name of the csv file (excluding ".csv") where
        the data to be processed resides.
        output_file_name:str - The name of the outputted JPEG file.
        plot_title:str - The title of the generated plot.
        dpi:int - Controls the DPI that the JPEG file will be rendered in.

    """

    df = pd.read_csv(f"{input_file_name}.csv")
    palette = sns.color_palette(n_colors=len(df["name"].unique()))
    ax = sns.relplot(
        data=df,
        x="threads",
        y="net_runtime_s",
        col="delay",
        style="name",
        hue="name",
        kind="line",
        palette=palette,
        col_wrap=3,
        **sns_kwargs,
    )

    temp_plot_title = plot_title.replace("(", "").replace(")", "")

    ax.legend.set_title("Queue")
    ax.figure.suptitle(temp_plot_title, y=SUPTITLE_Y)

    set_legend_location(ax)

    save_plot(ax, f"{IMAGES_PATH}/{output_file_name}.jpg", dpi)


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

        title = f"{queue_name} {plot_title}"
        ax.get_legend().set_title("Delay")
        ax.set_title(title)
        ax.set(**labels_kwargs)
        set_legend_location(ax)

        cleaned_queue_name = str.lower(queue_name).replace(" ", "_")

        file_path = f"{IMAGES_PATH}/{output_file_name}_{cleaned_queue_name}.jpg"

        save_plot(ax, file_path, dpi)


def plot_coefficient_of_variance(
    input_file: str, output_file: str, plot_title: str, dpi: int = 300
):
    df = pd.read_csv(f"{input_file}.csv")
    df_group = df.groupby(by=["name", "threads", "delay"], as_index=False)
    df_agg = df_group.agg(
        mean=("net_runtime_s", np.mean), stdev=("net_runtime_s", np.std)
    )
    df_agg["coeff_of_var"] = df_agg.apply(
        lambda x: (x["stdev"] * 100) / x["mean"], axis=1
    )

    ax: sns.FacetGrid = sns.catplot(
        x="delay",
        y="coeff_of_var",
        col="threads",
        hue="name",
        data=df_agg,
        kind="bar",
        palette=sns.color_palette(n_colors=len(df["name"].unique())),
        col_wrap=COL_WRAP,
    )

    title = "Coefficient of Variance of Tests $\\frac{\\sigma}{\\mu}$ " + plot_title

    ax.set_xlabels("Delay")
    ax.set_ylabels("Coefficient of Variance (%)")
    ax.figure.suptitle(title, y=SUPTITLE_Y)

    save_plot(ax, f"{IMAGES_PATH}/{output_file}_cov.jpg", dpi)


def plot_dwcas_relative_performance(
    input_file: str, output_file: str, plot_title: str, dpi: int = 300
):
    df = pd.read_csv(f"{input_file}.csv")
    df_group = (
        df.groupby(by=["name", "threads", "delay"], as_index=False)["net_runtime_s"]
        .mean()
        .sort_values(by="name", ascending=False)
    )
    nonblocking_queue_names = ["MS Queue", "Valois Queue", "Baskets Queue"]
    accumulated_df = pd.DataFrame()
    for name in nonblocking_queue_names:
        temp_df = df_group.loc[df_group["name"].str.contains(name)]
        temp_df = temp_df.groupby(by=["threads", "delay"], as_index=False).agg(
            {"net_runtime_s": lambda x: (x.iloc[0] / x.iloc[1])}
        )
        temp_df["name"] = name
        accumulated_df = pd.concat([accumulated_df, temp_df])
    ax = sns.relplot(
        x="threads",
        y="net_runtime_s",
        hue="delay",
        style="delay",
        col="name",
        kind="line",
        data=accumulated_df,
        palette=sns.color_palette(n_colors=len(df["delay"].unique())),
        **sns_base_kwargs,
    )
    ax.figure.suptitle(
        f"Relative Performance {plot_title}", y=SUPTITLE_Y
    )
    ax.set(xlabel="Threads", ylabel="Ratio of Tagged Pointer runtime to DWCAS runtime")
    save_plot(
        ax,
        f"{IMAGES_PATH}/{output_file}_rel_perf.jpg",
        dpi,
    )


ENQUEUE_DEQUEUE_TITLE = "(Pairwise Benchmark)"
ENQUEUE_DEQUEUE_FILE_NAME = "enqueue_dequeue_results"
P_ENQUEUE_DEQUEUE_TITLE = "(50% Enqueue Benchmark)"
P_ENQUEUE_DEQUEUE_FILE_NAME = "p_enqueue_dequeue_results"

plot_grouped_results(
    "enqueue_dequeue_results", "grouped_enqueue_dequeue", ENQUEUE_DEQUEUE_TITLE
)
plot_grouped_results(
    "p_enqueue_dequeue_results",
    "p_grouped_enqueue_dequeue",
    P_ENQUEUE_DEQUEUE_TITLE,
)
plot_coefficient_of_variance(
    "enqueue_dequeue_results", "enqueue_dequeue", ENQUEUE_DEQUEUE_TITLE
)
plot_coefficient_of_variance(
    "p_enqueue_dequeue_results", "p_enqueue_dequeue", P_ENQUEUE_DEQUEUE_TITLE
)
plot_individual_results(
    "enqueue_dequeue_results", "enqueue_dequeue", ENQUEUE_DEQUEUE_TITLE
)
plot_individual_results(
    "p_enqueue_dequeue_results", "p_enqueue_dequeue", P_ENQUEUE_DEQUEUE_TITLE
)
plot_dwcas_relative_performance(
    ENQUEUE_DEQUEUE_FILE_NAME, "enqueue_dequeue", ENQUEUE_DEQUEUE_TITLE
)
plot_dwcas_relative_performance(
    P_ENQUEUE_DEQUEUE_FILE_NAME, "p_enqueue_dequeue", P_ENQUEUE_DEQUEUE_TITLE
)
