#!/bin/bash

# $1 - Name of enqueue_dequeue benchmark binary to run
# $2 - Comment to leave in the index

current_epoch=$EPOCHSECONDS
dir_path=src/utils/readings/$1

mkdir -p $dir_path

file_name=$dir_path/$current_epoch.csv

echo "name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_ns" > $file_name
echo "$current_epoch: $2" | tee -a $dir_path/index.txt

for num_of_threads in $(seq 8)
do
    for reruns in $(seq 5)
    do
    ./build/$1 $num_of_threads 100 | tee -a $file_name
    done
done
