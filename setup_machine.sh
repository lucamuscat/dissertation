#!/bin/bash

# This script will disable hyperthreading, cpu frequency scaling
# and set the CPU to performance mode.

# These changes are only temporary; restarting the machine will
# revert these changes.

# Switch off hyper-threading
# https://askubuntu.com/a/1168248
echo "Disable Intel Hyperthreading..."
echo off | sudo tee /sys/devices/system/cpu/smt/control

# Disable CPU Frequency scaling
# https://askubuntu.com/a/580785
echo "Disable CPU Frequency Scaling... "
sudo cpufreq-set -g performance

# Disable intel turbo
# https://askubuntu.com/a/620114
echo "Disable Intel Turbo..."
echo "1" | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
