# Notes
* Cache hits and misses can be measured through performance event monitors in the CPU (intel vol 3a. sect. 19.1 & 19.7)
* Cache hits, misses, hotspots in the CPU.
* Will need to make assumptions about the use case .
* Agner Fogg.
* Give importance to figures in a thesis.
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

