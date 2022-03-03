from cProfile import label
import pandas as pd
import matplotlib.pyplot as plt

results = pd.read_csv("../../enqueue_dequeue_results.csv")
grouping = results.groupby(by="name")

fig, (ax1, ax2) = plt.subplots(nrows=2, ncols=1, figsize=(10, 7))

for name, group in grouping:
    group.plot(x="num_of_threads", y="average_cycles", ax=ax1, label=name)
    group.plot(x="num_of_threads", y="average_ns", ax=ax2, label=name)

fig.suptitle("Enqueue-Dequeue Pairs")

ax1.locator_params(integer=True)
ax2.locator_params(integer=True)

ax1.set_xlabel("Threads")
ax1.set_ylabel("Average Cycles")

ax2.set_xlabel("Threads")
ax2.set_ylabel("Average Time (ns)")

plt.savefig("enqueue_dequeue.png")