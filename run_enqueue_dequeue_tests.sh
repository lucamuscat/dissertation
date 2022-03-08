#!/bin/bash

# Switch off hyper-threading
# https://askubuntu.com/a/1168248
sudo echo off > /sys/devices/system/cpu/smt/control

# Disable CPU Frequency scaling
# https://askubuntu.com/a/580785
sudo cpufreq-set -g performance

# Disable intel turbo
# https://askubuntu.com/a/620114
sudo echo "1" > /sys/devices/system/cpu/intel_pstate/no_turbo

for i in 1 2 3 4
do
    echo "Threads: $i"
    ./build/blocking_linked_queue_pthread_lock $i >> enqueue_dequeue_results.csv
done
