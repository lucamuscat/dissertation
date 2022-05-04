import os
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
# Taken from: https://regenerativetoday.com/some-tricks-to-make-matplotlib-visualization-even-better/
# from mpl_toolkits.axes_grid1.inset_locator import mark_inset, inset_axes

images_path = "src/utils/images/"

os.makedirs(images_path, exist_ok=True)

markersize = 7
linewidth = 0.7

sns_kwargs = {
    "markers": True,
    "dashes": True,
    "ci": "sd",
    "markeredgecolor": "none",
    "err_style": "bars",
    "linewidth": linewidth,
    "markersize": markersize,
    "err_kws": {"linewidth" : linewidth, "capsize" : 2, "ecolor" : "k"},
}

labels_kwargs = {"xlabel": "Threads", "ylabel": "Net Runtime (s)"}

sns.set_style("whitegrid")
sns.set_context("paper")

def save_plot(ax: plt.Axes, output_file_name:str, plot_title:str, dpi:int):
    ax.set(
        title = plot_title,
        **labels_kwargs
    )
    ax.get_figure().savefig(output_file_name, dpi = dpi)
    ax.get_figure().clf()
    


def plot_grouped_results(delay:int, input_file_name:str, output_file_name:str, plot_title:str, dpi:int = 300):
    df = pd.read_csv(f"{input_file_name}.csv")
    individual_delay_df = df.query(f"delay == {str(delay)}").sort_values(by="name")
    palette = sns.color_palette(n_colors=len(df["name"].unique()))

    ax = sns.lineplot(
        data = individual_delay_df,
        x = "threads",
        y = "net_runtime_s",
        style = "name",
        hue = "name",
        palette = palette,
        **sns_kwargs
    )

    save_plot(ax, f"{images_path}/{output_file_name}.jpg", plot_title, dpi)

def plot_individual_results(input_file_name:str, output_file_name:str, plot_title:str, dpi:int = 300):
    df = pd.read_csv(f"{input_file_name}.csv", index_col="name")
    queue_names = df.index.unique().sort_values()
    palette = sns.color_palette(n_colors=len(df["delay"].unique()))


    for queue_name in queue_names:
        individual_df = df.loc[queue_name, ["net_runtime_s", "threads", "delay"]]
        individual_df.reset_index(drop = True, inplace = True)

        ax = sns.lineplot(
            data = individual_df,
            x = "threads",
            y = "net_runtime_s",
            style = "delay",
            hue = "delay",
            palette = palette,
            **sns_kwargs
        )

        cleaned_queue_name = str.lower(queue_name).replace(" ", "_")

        file_path = f"{images_path}/{output_file_name}_{cleaned_queue_name}.jpg"
        title = f"{queue_name}: {plot_title}"
        save_plot(ax, file_path, title, dpi)

enqueue_dequeue_title = "Pairwise Enqueue Dequeue Benchmark"
enqueue_dequeue_file_name = "enqueue_dequeue_results"
p_enqueue_dequeue_title = "50% Enqueue Benchmark"
p_enqueue_dequeue_file_name = "p_enqueue_dequeue_results"

plot_grouped_results(250, "enqueue_dequeue_results", "grouped_enqueue_dequeue", enqueue_dequeue_title)
plot_grouped_results(250, "p_enqueue_dequeue_results", "p_grouped_enqueue_dequeue", p_enqueue_dequeue_title)

plot_individual_results("enqueue_dequeue_results", "enqueue_dequeue", enqueue_dequeue_title)
plot_individual_results("p_enqueue_dequeue_results", "p_enqueue_dequeue", p_enqueue_dequeue_title)

