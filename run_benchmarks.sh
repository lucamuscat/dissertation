#!/bin/bash

# Validation
./build/delay_test
./build/tagged_ptr_test
./build/tagged_ptr_test_dwcas

enqueue_dequeue_results=enqueue_dequeue_results.csv
p_enqueue_dequeue_results=p_enqueue_dequeue_results.csv

csv_header="name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_min,counters"

if [ ! -f "$enqueue_dequeue_results" ]; then
    echo $csv_header > $enqueue_dequeue_results
fi

if [ ! -f "$p_enqueue_dequeue_results" ]; then
    echo $csv_header > $p_enqueue_dequeue_results
fi

delays=(0 50 100 150 200 250 500 750 1000)
queues=(
nonblocking_ms_queue
nonblocking_baskets_queue
nonblocking_valois_queue
nonblocking_dwcas_ms_queue
nonblocking_dwcas_baskets_queue
nonblocking_dwcas_valois_queue
blocking_linked_queue_ttas_lock
)

# Instrument hardware counters to be recorded
export PAPI_EVENTS="PAPI_BR_MSP,PAPI_BR_INS,PAPI_L1_TCM,PAPI_L2_TCM,PAPI_L3_TCM,LOCK_CYCLES,HW_INTERRUPTS,MISALIGN_MEM_REF,L2_LINES_IN"
export PAPI_MULTIPLEX=1

for queue in ${queues[@]}
do
    for num_of_threads in $(seq 12)
    do
        for delay in ${delays[@]}
        do
            for reruns in $(seq 3)
            do
            export PAPI_OUTPUT_DIRECTORY="output/$queue_d$delay_t$num_of_threads"
            ./build/$queue $num_of_threads $delay | tee -a $enqueue_dequeue_results
            export PAPI_OUTPUT_DIRECTORY="output/p_$queue_d$delay_t$num_of_threads"
            ./build/p_$queue $num_of_threads $delay 0.5 | tee -a $p_enqueue_dequeue_results
            done
        done
    done
done
