import subprocess
from turtle import delay
import pandas as pd
import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt

delay_key = "avg_delay_time"    
nops_key = "nops"

def run_delay_test(nanoseconds:int):
    args = ("build/delay_test", str(nanoseconds))
    popen = subprocess.Popen(args, stdout=subprocess.PIPE)
    output = popen.stdout.read().decode().split("\n")
    print(output)

    average_delay_time = float(
        output[4].replace("Average Delay Time: ", "").replace("ns", "")
    )

    num_of_nops = float(output[-3].replace("Number of nops: ", ""))
    
    return (average_delay_time, num_of_nops)

def generate_delay_data():
    results = list()
    for delay in [250, 500, 750, 1000]:
        for i in range(1):
            results.append((delay, *run_delay_test(delay)))
    df = pd.DataFrame(results, columns = ["expected_delay", delay_key, nops_key])
    # Remove row number from output result
    df.to_csv("./src/utils/readings/delay_readings.csv", index=False)

generate_delay_data()