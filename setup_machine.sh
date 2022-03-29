#!/bin/bash

# Switch off hyper-threading
# https://askubuntu.com/a/1168248
# sudo echo off > /sys/devices/system/cpu/smt/control

# Disable CPU Frequency scaling
# https://askubuntu.com/a/580785
sudo cpufreq-set -g performance

# Disable intel turbo
# https://askubuntu.com/a/620114
sudo echo "1" | tee /sys/devices/system/cpu/intel_pstate/no_turbo