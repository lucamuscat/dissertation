#!/bin/bash

# $1 - Name of enqueue_dequeue benchmark binary to run
# $2 - Comment to leave in the index

current_epoch=$EPOCHSECONDS
dir_path=src/utils/readings/$1

mkdir -p $dir_path

enqueue_dequeue_results=enqueue_dequeue_results.csv
p_enqueue_dequeue_results=p_enqueue_dequeue_results.csv


echo "name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_ns" > $enqueue_dequeue_results
echo "name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_ns" > $p_enqueue_dequeue_results

# echo "$current_epoch: $2" | tee -a $dir_path/index.txt

delays=(0 100 200 300 400 500 600 700 800 900 1000)
queues=(
nonblocking_ms_queue
nonblocking_ms_queue_with_tagged_ptr
nonblocking_baskets_queue
nonblocking_baskets_queue_with_tagged_ptr
nonblocking_valois_queue
blocking_linked_queue_ttas_lock
)

for queue in ${queues[@]}
for num_of_threads in $(seq 12)
do
    for delay in ${delays[@]}
    do
        for reruns in $(seq 5)
        do
        ./build/$queue $num_of_threads $delay | tee -a $enqueue_dequeue_results
        ./build/p_$queue $num_of_threads $delay | tee -a $p_enqueue_dequeue_results
        done
    done
done
