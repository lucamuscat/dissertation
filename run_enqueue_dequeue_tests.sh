#!/bin/bash
# Switch off hyper-threading
# https://askubuntu.com/a/1168248
sudo echo off > /sys/devices/system/cpu/smt/control
# sudo echo off > /sys/devices/system/cpu/intel_pstate/status
# ./build/nonblocking_aldinucci_spsc 1
# ./build/nonblocking_aldinucci_spsc 1
# ./build/nonblocking_aldinucci_spsc 2
# ./build/nonblocking_aldinucci_spsc 2
# ./build/nonblocking_aldinucci_spsc 4
# ./build/nonblocking_aldinucci_spsc 4

./build/blocking_circular_buffer_kernel_lock 1
./build/blocking_circular_buffer_kernel_lock 2
./build/blocking_circular_buffer_kernel_lock 3
./build/blocking_circular_buffer_kernel_lock 4
./build/blocking_circular_buffer_filter_lock 1
./build/blocking_circular_buffer_filter_lock 2
./build/blocking_circular_buffer_filter_lock 3
./build/blocking_circular_buffer_filter_lock 4








