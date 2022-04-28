#!/bin/bash

reruns=3
up_to_threads=12
delays=(0 250 500 750 1000)

# $1 - Name of enqueue_dequeue benchmark binary to run

# current_epoch=$EPOCHSECONDS
dir_path=src/utils/readings/$1

mkdir -p $dir_path

file_name=enqueue_dequeue_results.csv
p_file_name=p_enqueue_dequeue_results.csv

echo "name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_ns" > $file_name
echo "name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_ns" > $p_file_name

# echo "$current_epoch: $2" | tee -a $dir_path/index.txt

for num_of_threads in $(seq $up_to_threads)
do
    for delay in ${delays[@]}
    do
        for reruns in $(seq $reruns)
        do
            ./build/$1 $num_of_threads $delay | tee -a $file_name
            ./build/p_$1 $num_of_threads $delay 0.5 | tee -a $p_file_name
        done
    done
done
