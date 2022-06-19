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


csv_header="name,threads,delay,time_ns,net_runtime_s,stdev_ns,p_ns,total_runtime_min"

export PAPI_EVENTS="PAPI_BR_MSP,PAPI_BR_PRC,PAPI_L2_TCM,MEM_INST_RETIRED:LOCK_LOADS,MEM_LOAD_L3_HIT_RETIRED:XSNP_HITM"
export PAPI_MULTIPLEX=1

echo $csv_header > $file_name
echo $csv_header > $p_file_name

# echo "$current_epoch: $2" | tee -a $dir_path/index.txt
mkdir -p output

for num_of_threads in $(seq $up_to_threads)
do
    for delay in ${delays[@]}
    do
        for reruns in $(seq $reruns)
        do
            export PAPI_OUTPUT_DIRECTORY="output/$1"
            ./build/$1 $num_of_threads $delay | tee -a $file_name
            export PAPI_OUTPUT_DIRECTORY="output/p_$1"
            ./build/p_$1 $num_of_threads $delay 0.5 | tee -a $p_file_name
        done
    done
done
