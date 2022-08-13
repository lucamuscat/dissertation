"""
Automatically generates plots for the results obtained in the root directory.
"""

import os
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np
import plot_config
from helpers import get_difference_in_time_between_threads
from multiprocessing import Process, Pool
from typing import Union, List

os.makedirs(plot_config.IMAGES_PATH, exist_ok=True)

labels_kwargs = {"xlabel": "Threads", "ylabel": "Net Runtime (s)"}

sns.set_style(plot_config.grid_style)
sns.set_context(plot_config.PLOT_CONTEXT)
sns.set_palette("colorblind")

# Dataframe containing pairwise data
df_pairwise = pd.read_csv("enqueue_dequeue_results.csv")
# Dataframe containing 50% enqueue data
df_coin_toss = pd.read_csv("p_enqueue_dequeue_results.csv")

def set_legend_location(ax: plt.Axes):
    sns.move_legend(ax, "upper left", bbox_to_anchor=(1, 1))

def save_plot(
    ax: Union[plt.Axes, sns.FacetGrid, plt.figure],
    output_file_name: str,
    dpi: int,
):
    fig = ax.get_figure() if ax is plt.Axes else ax.figure
    fig.set_size_inches(*plot_config.FIG_SIZE)
    fig.savefig(f"{plot_config.IMAGES_PATH}/{output_file_name}.jpg", dpi=dpi, bbox_inches="tight", )
    fig.clf()
    # Set figure to matplotlib default size to prevent figure size adjustments
    # from carrying on to independent figures.
    fig.set(size_inches=(6.4, 4.8))

def plot_grouped_results(
    df: pd.DataFrame,
    output_file_name: str,
    plot_title: str,
    dpi: int = 300,
):
    """
    Plot the readings of each queue for a specific delay, all in one graph.

    Args:
        df:pd.DataFrame - DataFrame containing readings.
        output_file_name:str - The name of the outputted JPEG file.
        plot_title:str - The title of the generated plot.
        dpi:int - Controls the DPI that the JPEG file will be rendered in.
    """
    
    num_of_unique_names = len(df["name"].unique())
    palette = sns.color_palette(n_colors=num_of_unique_names)
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
        **plot_config.sns_kwargs,
    )

    temp_plot_title = plot_title.replace("(", "").replace(")", "") 
    ax.legend.set_title(None)
    sns.move_legend(ax, "upper center", ncol=num_of_unique_names//2, bbox_to_anchor=(0.45, 1.08))
    plt.setp(ax._legend.get_texts(), fontsize=plot_config.LEGEND_TEXT_FONT_SIZE)

    ax.figure.suptitle(temp_plot_title, y=plot_config.SUPTITLE_Y, fontsize=plot_config.SUPTITLE_FONT_SIZE)

    ax.set_axis_labels(*labels_kwargs.values(), fontsize=plot_config.X_LABEL_FONT_SIZE)
    #ax.set_yticklabels(size=X_LABEL_FONT_SIZE-2)
    ax.set_xticklabels(size=plot_config.X_LABEL_FONT_SIZE-2)
    ax.set_titles(size=12)
    ax.set(xlim=(0.8,12.2), ylim=(0,None))
    plt.subplots_adjust(hspace=0.1)

    save_plot(ax, output_file_name, dpi)


def plot_delay_and_threads(thread_count:int, dataframes: List[pd.DataFrame]):
    fig, ax = plt.subplots(
        1, 
        2, 
        sharey=True, 
        figsize=plot_config.FIG_SIZE
    )

    for axis, df in zip(ax, dataframes):
        df_threads = df.copy().loc[df["threads"] == thread_count]
        palette = sns.color_palette(n_colors=len(df_threads["name"].unique()))
        df_threads = df_threads.sort_values(by="name")
        sns.lineplot(
            data=df_threads,
            ax=axis,
            x = "delay",
            y = "net_runtime_s",
            style = "name",
            hue = "name",
            palette = palette,
            **plot_config.sns_kwargs
        )

        # title = f"Performance at {thread_count} threads {plot_title}"
        # ax.set_title(title)
        axis.set_xlabel("Delay")
        axis.set_ylabel("Net Runtime (s)")

    ax[0].set_title("Pairwise")
    ax[1].set_title("50% Enqueue")
    thread_text = "Thread" if thread_count == 1 else "Threads"
    fig.suptitle(f"Performance at {plot_config.num_to_word_dict[thread_count]} {thread_text}", y = 1.12)
    adjust_double_plot(ax)

    output_path = f"delay_thread_{thread_count}"
    save_plot(fig, output_path, 300)


def adjust_double_plot(axis):
    axis[1].legend().set_visible(False)
    sns.move_legend(
        axis[0],
        "upper center",
        bbox_to_anchor=(1,1.25),
        frameon=False,
        ncol=3,
        title=""
    )
    plt.subplots_adjust(wspace=0.05, left=0)


def get_speedup_by_adjacent_thread_axis(thread_count:int, df: pd.DataFrame, plot_title: str, axis=None):
    df_diff = get_difference_in_time_between_threads(df, start=thread_count)
    df_diff = df_diff.sort_values(by="name")
    ax = sns.lineplot(
        data=df_diff,
        x="delay",
        y="perf_deg",
        style="name",
        hue="name",
        ax=axis,
        **plot_config.sns_kwargs
    )
    
    sns.move_legend(
        ax, 
        "upper center", 
        bbox_to_anchor=(0.5,1.2), 
        frameon=False, 
        ncol=3, 
        title=""
    )

    ax.set_title(
        f"Magnitude of Performance Degradation at {plot_config.num_to_word_dict[thread_count + 1]} Threads {plot_title}",
        y=1.2
    )
    ax.set_xlabel("Delay")
    ax.set_ylabel("Magnitude of Performance Degradation")
    return ax


def plot_speedup_by_adjacent_thread(
    thread_count:int, df: pd.DataFrame, output_file: str, plot_title: str, dpi: int = 300):
    ax = get_speedup_by_adjacent_thread_axis(thread_count, df, plot_title)
    file_path = f"speedup_{output_file}_{thread_count}"
    save_plot(ax, file_path, dpi)


def plot_coefficient_of_variance(
    df: pd.DataFrame, output_file: str, plot_title: str, dpi: int = 300
):
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
        col_wrap=4,
    )

    title = "Coefficient of Variance of Tests $\\frac{\\sigma}{\\mu}$ " + plot_title

    ax.set_xlabels("Delay")
    ax.set_ylabels("Coefficient of Variance (%)")
    ax.figure.suptitle(title, y=plot_config.SUPTITLE_Y)

    save_plot(ax, f"{output_file}_cov", dpi)


def plot_pairwise_and_cointoss_speedup(start):
    fig, (ax1, ax2) = plt.subplots(1, 2, sharey=True, figsize=plot_config.FIG_SIZE)
    fig.suptitle(f"Magnitude of Performance Degradation at {plot_config.num_to_word_dict[start+1]} Threads", y=1.12)
    get_speedup_by_adjacent_thread_axis(start, df_pairwise, ENQUEUE_DEQUEUE_TITLE, ax1)
    ax1.set_title("Pairwise Benchmark")
    get_speedup_by_adjacent_thread_axis(start, df_coin_toss, P_ENQUEUE_DEQUEUE_TITLE, ax2)
    ax2.set_title("50% Enqueue Benchmark")
    adjust_double_plot([ax1, ax2])

    save_plot(fig, f"speedup_{start}", 300)

ENQUEUE_DEQUEUE_TITLE = "(Pairwise Benchmark)"
ENQUEUE_DEQUEUE_FILE_NAME = "enqueue_dequeue_results"
P_ENQUEUE_DEQUEUE_TITLE = "(50% Enqueue Benchmark)"
P_ENQUEUE_DEQUEUE_FILE_NAME = "p_enqueue_dequeue_results"

dataframes = [df_pairwise, df_coin_toss]

plot_pairwise_and_cointoss_speedup(2)
plot_pairwise_and_cointoss_speedup(3)
plot_pairwise_and_cointoss_speedup(4)
plot_pairwise_and_cointoss_speedup(5)

plot_delay_and_threads(1, dataframes)
plot_delay_and_threads(2, dataframes)
plot_delay_and_threads(3, dataframes)
plot_delay_and_threads(4, dataframes)
