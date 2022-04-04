#!/bin/bash
git clone https://bitbucket.org/icl/papi.git
sudo sh -c "echo 2 > /proc/sys/kernel/perf_event_paranoid" 
cd papi/src 
./configure
make
make fulltest
make install all
rm -rf papi