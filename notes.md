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

# Meeting notes 16-03-2022
* Differences of averages across threads is useful to determine if each core is given the same amount of priority by the scheduler.
* Create a configuration file which contains the number of iterations which will give the least amount of standard deviation across readings
(No need to automatically figure that out)
* Since the average of each enqueue/dequeue pair is being taken, the total number of
iterations done is not really relevant, the most important thing is that no matter
the number of iterations, they all converge to one value.

# Meeting notes 23-03-2022
* AR - Generates a files (statically linked .a files)

## 11-05-2022
* I should reach a stage where I send a draft of the paper in its complete form for proper review
* If large portions are lifted, including in quotes.
### Placement of Related Work
* Don't worry about the distinction between literature review and related work,
it is possible to mingle them together.
* Can be placed at the end.
* Just write it for now and then decide where to place it.

## 18-05-2022
* Finalize the research objectives in the introduction
* Aims and Objectives - Describe the aims in a paragraph, following the concrete definitions of the objectives
* Research Objectives
1. Implement a benchmarking framework for concurrent queues
2. Validate the benchmarking framework
3. Implement a number of blocking and non-blocking queues
4. Evaluate the performance of every implemented queue and compare with researcher's results

## TODO
* Find a formality to prove that the timings are correct (can use a mock queue)
* Stop being a perfectionist and just start expanding on the chapters, don't allow it to halt progress.

## More notes
* Power Managment - sys/devices/cpu/intel/pstate
* A Comparitive Study of Concurrent Queuing Algorithms & Their Performance
* To avoid confusion, the newer title encompasses 
* Keep Zobel pg 97 in mind when writing the intro.

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

