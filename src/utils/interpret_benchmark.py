import os
import pandas as pd
from datetime import datetime


def read_index():
    index = dict()
    with open(f"{readings_path}/index.txt", "r") as index_file:
        lines = [x.rstrip("\n") for x in index_file.readlines()]
        split_lines = [line.split(":") for line in lines]
        assert all(len(x) == 2 for x in split_lines), "You have more than one entry in a line"
        for epoch_time, comment in split_lines:
            index[int(epoch_time)] = comment
    return index

dir = input("queue directory:")
readings_path = f"src/utils/readings/{dir}"

files = os.listdir(f"{readings_path}")
assert len(files) >= 2, "You need two or more readings before you can run this script"
time_started = sorted([int(x.replace(".csv", "")) for x in files if x.endswith(".csv")])

index = read_index()

for i, epoch_time in enumerate(time_started):
    print(i, datetime.fromtimestamp(epoch_time), index[epoch_time])

previous_index = int(input("Previous:"))
assert 0 <= previous_index < len(time_started), "Previous index is out of bounds"
current_index = int(input("Current:"))
# Make sure indices are in bounds
assert 0 <= current_index < len(time_started), "Current index is out of bounds"

previous_reading = pd.read_csv(f"{readings_path}/{time_started[previous_index]}.csv")
current_reading = pd.read_csv(f"{readings_path}/{time_started[current_index]}.csv")

runtime_key = "net_runtime_s"
threads_key = "threads"

def aggregate(dataframe:pd.DataFrame):
    grouping = dataframe.groupby(by = threads_key)
    net_runtime_mean = grouping[runtime_key].mean()
    net_runtime_std = grouping[runtime_key].std()
    net_runtime_coeff_of_var = (net_runtime_std/net_runtime_mean) * 100
    return (net_runtime_mean, net_runtime_coeff_of_var)


previous_mean, previous_cov = aggregate(previous_reading)
current_mean, current_cov = aggregate(current_reading)

print("Speed up ratio (previous/current reading)")
print(previous_mean/current_mean)
print("Difference in previous and current coefficient of variance (%)")
print(current_cov - previous_cov)
