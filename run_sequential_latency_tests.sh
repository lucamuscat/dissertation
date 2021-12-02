#!/bin/bash
MAX_THREADS=$(getconf _NPROCESSORS_ONLN)
FILES=$(ls ./build/*seq_lat.o)
OUTPUT_FILE_NAME="seq_lat_results.txt"

output=""

if [ ! -f "$OUTPUT_FILE_NAME" ]; then
output+="lock_name, number_of_threads, average_time_ns"
fi

for file in $FILES
do
    if [[ "$file" != *"peterson"* ]]; then
        for((i = 2; i <= $MAX_THREADS;i++));
        do
            output+=$(( $file $i ) 2>&1)
        done
    else
        output+=$(($file 2) 2>&1)
    fi
done

$output >> $OUTPUT_FILE_NAME