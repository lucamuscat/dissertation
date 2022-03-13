# Notes
* Cache hits and misses can be measured through performance event monitors in the CPU (intel vol 3a. sect. 19.1 & 19.7)
* Cache hits, misses, hotspots in the CPU.
* Will need to make assumptions about the use case.
* Agner Fog.
* Give importance to figures in a thesis.
* Hotspot - Repeated access to a memory location
    * Can cause the memory system to degrade

##  Memory Consistency
> In PRAM consistency, all processes view the operations of a single process in the same order that they were issued by that process, while operations issued by different processes can be viewed in different order from different processes. PRAM consistency is weaker than processor consistency. PRAM relaxes the need to maintain coherence to a location across all its processors. Here, reads to any variable can be executed before writes in a processor. Read before write, read after read and write before write ordering is still preserved in this model.

## Art of Multiprocessers
* In chapter 7, Herlihy mentions that his implementation of the petersons won't work due to the fact that his implementation assumed sequential consistency.

# Questions for Kevin 2-12-2021
* Is linux's standard clock good enough? Or should a custom clock for the process be implemented [clock_getres(2)](https://man7.org/linux/man-pages/man2/clock_gettime.2.html)
* Different types of clocks:
    * CLOCK_REALTIME
    * CLOCK_MONOTONIC - Measures relatives real time. Advances at the same rate as the actual flow of time.
    * CLOCK_BOOTTIME - Gets the running and suspension time of the process.
    * CLOCK_THREAD_CPUTIME_ID - Measures the CPU time consumed by this thread.
    * CLOCK_PROCESS_CPUTIME_ID - Measures the CPU time consumed by this process (ie. CPU time consumed by all threads in the process).
Will use multiple clock types and check their disparities.

# Meeting notes 24-02-2022
* Get one or two algorithms done and get their timings.
* Test run of all the metrics that I am likely to collect.
* Get a queue implemented so that you have something to run metrics on
* Each lock-based structure can be implemented, for example buffer based, linked-list, circular
* Just start getting code done, we need a basis of code.
* Keep it simple for now, just measure time.
* Range of results will be nice to show.
## TODO
* Create a bunch of queues, 
lock-based and maybe some simple lock-free
* At least 2 lock-based queues.
* At most 1 wait-free queue.
* Get readings on them (contention vs time)

## More notes
* Power Managment - sys/devices/cpu/intel/pstate
* A Comparitive Study of Concurrent Queuing Algorithms & Their Performance
* To avoid confusion, the newer title encompasses 

# Bash Notes
* `if [condition]` is the same as `if test condition`
```bash
if
    # exit code of commands-list1 determines 
    # wheter the else branch occurs or not.
    command-list1 
then
    command-list2
else
    command-list3
fi
```

# Guide
Call `make` to compile every single test.

```bash
# Compile every single test
make

# Compile all sequential latency tests (with logging)
make sequential_latency_tests

# Compile all sequential latency tests (output CSV)
make sequential_latency_tests SILENT=T

# Compile all increment counter tests (with logging)
make increment_counter_tests

# Compile all increment counter tests (without logging)
make increment_counter_tests SILENT=T
```

