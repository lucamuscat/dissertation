"""
Interpret counters found in the data gathered.
"""

import os
import pandas as pd
import numpy as np
from typing import List

COUNTERS_DIR = "src/utils/counters"

os.makedirs(COUNTERS_DIR, exist_ok=True)

ENQUEUE_DEQUEUE_FILE_NAME = "enqueue_dequeue_results.csv"
P_ENQUEUE_DEQUEUE_FILE_NAME = "p_enqueue_dequeue_results.csv"
TEST_RERUNS = 10

# Disable scientific notation from output
pd.options.display.float_format = '{:.4f}'.format

def interpret_counters(file_name:str, counter_labels:List[str], queue_name: str) -> pd.DataFrame:
    def apply_func(x: pd.Series):
        counters = [[j for j in i.split(" ") if j != ""] for i in x]
        counters = np.array([np.asarray(x, dtype=int) for x in counters])
        counters = counters.reshape((-1, TEST_RERUNS, len(counter_labels)))
        counters = np.array([x.transpose() for x in counters])
        mean = np.apply_along_axis(np.mean, 1, np.apply_along_axis(np.mean, 2, counters).transpose())
        #std = np.apply_along_axis(np.std, 2, counters)
        return mean

    df = pd.read_csv(file_name)
    df = (
        df.loc[df["name"].str.contains(queue_name)]
        .groupby(by=["name", "threads", "delay"])["counters"]
        .apply(apply_func)
        .reset_index()
    )
    # https://datascienceparichay.com/article/split-pandas-column-of-lists-into-multiple-columns/#:~:text=To%20split%20a%20pandas%20column,The%20following%20is%20the%20syntax.&text=You%20can%20also%20pass%20the,the%20help%20of%20an%20example.
    temp = pd.DataFrame(df["counters"].to_list(), columns=counter_labels)
    df = pd.concat([df, temp], axis=1)
    df = df.drop("counters", axis=1)
    df.round(3)
    return df

MS_QUEUE_NAME_TAGGED_PTR = "MS Queue using Tagged Pointers"
MS_QUEUE_NAME_DWCAS = "MS Queue using DWCAS"

MS_QUEUE_COUNTER_LABELS = [
    "E3-E4",
    "E7-E8",
    "E8-E9",
    "E12-E13",
    "E9-E10",
    "D4-D5",
    "D5-D6",
    "D7-D8",
    "D10-D11",
    "D12-D13",
    "D15-D16",
]

def save_counters_to_html(input_file_name:str, labels, queue_name, output_file_name):
    interpret_counters(input_file_name, labels, queue_name).to_html(f"{COUNTERS_DIR}/{output_file_name}.html")


counters = [
    (ENQUEUE_DEQUEUE_FILE_NAME, MS_QUEUE_COUNTER_LABELS, MS_QUEUE_NAME_TAGGED_PTR, "ms_queue_tagged_ptr"),
    (ENQUEUE_DEQUEUE_FILE_NAME, MS_QUEUE_COUNTER_LABELS, MS_QUEUE_NAME_DWCAS, "ms_queue_dwcas"),
    (P_ENQUEUE_DEQUEUE_FILE_NAME, MS_QUEUE_COUNTER_LABELS, MS_QUEUE_NAME_TAGGED_PTR, "p_ms_queue_tagged_ptr"),
]

for counter in counters:
    save_counters_to_html(*counter)
    