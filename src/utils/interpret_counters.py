"""
Interpret counters found in the data gathered.
"""

import os
import pandas as pd
import numpy as np
from typing import Dict

def interpret_counters(
    dataframe: pd.DataFrame, counter_labels: Dict[str, str], queue_name: str
) -> pd.DataFrame:
    def apply_func(x: pd.Series):
        counters = [[j for j in i.split(" ") if j != ""] for i in x]
        counters = np.array([np.asarray(x, dtype=int) for x in counters])
        counters = counters.reshape((-1, TEST_RERUNS, len(counter_labels)))
        counters = np.array([x.transpose() for x in counters])
        mean = np.apply_along_axis(
            np.mean, 1, np.apply_along_axis(np.mean, 2, counters).transpose()
        )
        # std = np.apply_along_axis(np.std, 2, counters)
        return mean

    df = (
        dataframe.loc[dataframe["name"].str.contains(queue_name)]
        .groupby(by=["name", "threads", "delay"])["counters"]
        .apply(apply_func)
        .reset_index()
    )
    # https://datascienceparichay.com/article/split-pandas-column-of-lists-into-multiple-columns/#:~:text=To%20split%20a%20pandas%20column,The%20following%20is%20the%20syntax.&text=You%20can%20also%20pass%20the,the%20help%20of%20an%20example.
    temp = pd.DataFrame(df["counters"].to_list(), columns=counter_labels.values())
    df = pd.concat([df, temp], axis=1)
    df = df.drop("counters", axis=1)
    df.round(3)
    return df


MS_QUEUE_NAME_TAGGED_PTR = "MS Queue using Tagged Pointers"
MS_QUEUE_NAME_DWCAS = "MS Queue using DWCAS"
MS_QUEUE_COUNTER_LABELS = {
    "e3":"Enqueue Count (E3-E4)", # Useless
    "e7":"Enqueue Consistent Tail (E7-E8)",
    "e8":"Enqueue Next Pointer is Null (E8-E9)",
    "e12":"Enqueue Next Pointer is Not Null (E12-E13)",
    "e9":"Enqueue attempt to update tail (E9-E10)", # Useless
    # Can be larger than the total number of iterations, as it includes retries
    # caused by failed Compare-and-Swaps
    "d4":"Dequeue Attempts D4-D5",
    "d5":"Dequeue Consistent Head (D5-D6)",
    "d7":"Empty Queue Dequeue (D7-D8)",
    "d10":"Single Item Queue Swing Tail (D10-D11)",
    "d12":"Dequeue Queue Not Empty (D12-D13)",
    "d15":"Dequeue Failed to Swing Non Empty Queue (D15-D16)",
}

BASKETS_QUEUE_COUNTER_LABELS = {
    "E05-E06" : "enqueue_count (E05-E06)",
    "E06-E07" : "enqueue_consistent_tail_count (E06-E07)",
    "E07-E08" : "enqueue_next_ptr_null_count (E07-E08)",
    "E09-E10" : "enqueue_cas_tail_count (E09-E10)",
    "E18-E19" : "enqueue_build_basket_count (E18-E19)",
    "E22" : "enqueue_update_tail_count (E22)",
    "E21-E22" : "enqueue_next_ptr_not_null_count (E21-E22)",
    "D04-D05" : "dequeue_count (D04-D05)",
    "D06-D07" : "dequeue_consistent_head_count (D06-D07)",
    "D06-D07 (2)" : "dequeue_single_item_count (D06-D07)",
    "D07-D08" : "dequeue_empty_count (D07-D08)",
    "D19-D20" : "dequeue_inconsistent_head_count (D19-D20)",
    "D21-D22" : "dequeue_consistent_iter_tail_count (D21-D22)",
    "CAS0": "dequeue_update_tail_count",
    "CAS1":"dequeue_freechain_update_head_count",
    "freechain" : "dequeue_freechain_count",
    "D28 - D29" : "dequeue_cas_failed_count (D28 - D29)"
}

ENQUEUE_DEQUEUE_FILE_NAME = "enqueue_dequeue_results.csv"
P_ENQUEUE_DEQUEUE_FILE_NAME = "p_enqueue_dequeue_results.csv"
TEST_RERUNS = 10

counters = [
    (
        ENQUEUE_DEQUEUE_FILE_NAME,
        MS_QUEUE_COUNTER_LABELS,
        MS_QUEUE_NAME_TAGGED_PTR,
        "ms_queue_tagged_ptr",
    ),
    (
        ENQUEUE_DEQUEUE_FILE_NAME,
        MS_QUEUE_COUNTER_LABELS,
        MS_QUEUE_NAME_DWCAS,
        "ms_queue_dwcas",
    ),
    (
        P_ENQUEUE_DEQUEUE_FILE_NAME,
        MS_QUEUE_COUNTER_LABELS,
        MS_QUEUE_NAME_TAGGED_PTR,
        "p_ms_queue_tagged_ptr",
    ),
    (
        P_ENQUEUE_DEQUEUE_FILE_NAME,
        MS_QUEUE_COUNTER_LABELS,
        MS_QUEUE_NAME_DWCAS,
        "p_ms_queue_dwcas",
    ),
    (
        ENQUEUE_DEQUEUE_FILE_NAME,
        BASKETS_QUEUE_COUNTER_LABELS,
        "Baskets Queue using Tagged Pointers",
        "baskets_tagged_ptr"
    ),
    (
        ENQUEUE_DEQUEUE_FILE_NAME,
        BASKETS_QUEUE_COUNTER_LABELS,
        "Baskets Queue using DWCAS",
        "baskets_dwcas"
    )
]


def save_counters_to_html(input_file_name: str, labels, queue_name, output_file_name) -> pd.DataFrame:
    temp = pd.read_csv(input_file_name)
    df = interpret_counters(temp, labels, queue_name)
    df.to_html(
        f"{COUNTERS_DIR}/{output_file_name}.html",
    )
    return df

if __name__ == "__main__":
    COUNTERS_DIR = "src/utils/counters"

    os.makedirs(COUNTERS_DIR, exist_ok=True)

    # Disable scientific notation from output
    pd.options.display.float_format = "{:.4f}".format

    counters_df = list()

    for counter in counters:
        counters_df.append(save_counters_to_html(*counter))
