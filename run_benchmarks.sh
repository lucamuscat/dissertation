#!/bin/bash

dir_path=src/utils/readings/$1

mkdir -p $dir_path

./build/delay_test
./build/tagged_ptr_test
./build/tagged_ptr_test_dwcas

enqueue_dequeue_results=enqueue_dequeue_results.csv
p_enqueue_dequeue_results=p_enqueue_dequeue_results.csv

echo "name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_min,counters" > $enqueue_dequeue_results
echo "name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_min,counters" > $p_enqueue_dequeue_results

delays=(0 250 500 750 1000)
queues=(
nonblocking_ms_queue
nonblocking_baskets_queue
nonblocking_valois_queue
nonblocking_dwcas_ms_queue
nonblocking_dwcas_baskets_queue
nonblocking_dwcas_valois_queue
blocking_linked_queue_ttas_lock
)

for queue in ${queues[@]}
do
    for num_of_threads in $(seq 12)
    do
        for delay in ${delays[@]}
        do
            for reruns in $(seq 3)
            do
            ./build/$queue $num_of_threads $delay | tee -a $enqueue_dequeue_results
            ./build/p_$queue $num_of_threads $delay 0.5 | tee -a $p_enqueue_dequeue_results
            done
        done
    done
done
