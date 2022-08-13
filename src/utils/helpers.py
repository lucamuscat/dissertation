import pandas as pd
import numpy as np

net_runtime_key = "net_runtime_s"

def aggregate_readings(df: pd.DataFrame):
    df = df[["name", "threads", "delay", net_runtime_key]]
    df = df.groupby(
        by=["name", "threads", "delay"],
        as_index=False
    ).agg(net_runtime_s=(net_runtime_key, np.mean), std=(net_runtime_key, np.std))
    df["cov(%)"] = ((df["std"]*100)/df["net_runtime_s"]).round(3)
    return df

# Taken from: https://stackoverflow.com/questions/71895146/pandas-to-latex-how-to-make-column-names-bold
def save_dataframe_to_latex_table(filtered_df: pd.DataFrame, file_name: str, caption:str, label:str):
    (filtered_df
        .style
        .hide(axis="index")
        .applymap_index(lambda _: "font-weight:bold;", axis="columns")
        .to_latex(
            file_name,
            convert_css=True,
            environment="table*",
            position_float="centering",
            caption=caption,
            label=label,
            position="h"
        )
    )

def get_difference_in_time_between_threads(
    df: pd.DataFrame, 
    start: int=1):
    temp = df.copy()
    df_1 = temp.loc[df["threads"]==start]
    df_2 = temp.loc[df["threads"]==start+1]
    df_1.insert(3, "perf_deg", np.array(df_2["net_runtime_s"])/np.array(df_1["net_runtime_s"]))
    return df_1[["name", "delay", "perf_deg"]]

def compare_two_queues(df:pd.DataFrame, queue_name_1:str, queue_name_2:str, thread_count:int):
    temp = df.loc[(df["threads"]==thread_count)]
    temp = temp.groupby(by=["name", "delay"], as_index=False)["net_runtime_s"].mean()
    queue_1 = temp.loc[temp["name"]==queue_name_1].copy()
    queue_2 = temp.loc[temp["name"]==queue_name_2].copy()

    queue_1_runtime = np.array(queue_1["net_runtime_s"])
    queue_2_runtime = np.array(queue_2["net_runtime_s"])

    result_key = f"{queue_name_1} is x% slower than {queue_name_2}"
    queue_1[result_key] = ((queue_1_runtime-queue_2_runtime)/queue_1_runtime)*100
    queue_1 = queue_1[["delay", result_key]]
    queue_1 = queue_1.rename({"delay":"Delay"}, axis="columns")
    queue_1["Is Slower?"] = queue_1[result_key] > 0

    return queue_1