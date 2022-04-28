"""
Report the coefficient of variance of enqueue_dequeue and p_enqueue_dequeue
"""

import pandas as pd

def get_cov_from_csv(file_name:str):
    readings = pd.read_csv(f"{file_name}.csv")

    delay_group = readings.groupby(["name", "delay", "threads"])
    avg_delay_group = delay_group["net_runtime_s"].mean()
    std_delay_group = delay_group["net_runtime_s"].std()
    result = (std_delay_group/avg_delay_group)*100
    return result.sort_values(ascending=False)
    
enqueue_dequeue = get_cov_from_csv("enqueue_dequeue_results")
p_enqueue_dequeue = get_cov_from_csv("p_enqueue_dequeue_results")

print(enqueue_dequeue)

#enqueue_dequeue_250 = enqueue_dequeue[enqueue_dequeue.index.get_level_values("delay").isin([2]
#p_enqueue_dequeue_250 = p_enqueue_dequeue[p_enqueue_dequeue.index.get_level_values("delay").isin([250])]

# print(enqueue_dequeue.head(15))
# print(p_enqueue_dequeue.head(15))

