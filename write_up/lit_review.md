# Algorithms for Scalable Synchronization on Shared-Memory Multiprocessors - Mellor-Crummey and Scott
Mellor-Crummey and Scott cover five different spin lock algorithms, one of them being a novel locking algorithm. This paper also covers implementations of barriers.

* Authors note that mutual exclusion has been used in context of distributed systems, but the characteristics of message passing are different enough to shared memory for the solutions not to carry over.

## Test and Set
* Principal short coming is contention for the flag
* The flag is accessed as frequently as possible using an expensive RMW (read modify write, ex. fetch_and_phi) instructions.
* There is degraded performance not only of the memory bank in which hte lock resides, but also of the processor/memory interconnection.
* Fetch and phi may cause many cache invalidations
### Test and Test and Set
* Introduced by [@rudolph1984dynamic]
* Reduces the total amount of network traffic caused by busy waiting on a test_and_set lock.
* Can be further reduced by adding a delay. Delay may be constant, but Anderson reports the best performance with exponential backoff [@anderson1990performance].

## Ticket Lock
* Fair, reduces the chance of starvation
* Optimization of a Lamport's bakery lock (was designed for fault tolerance as opposed to performance).
* Still causes substantial memory and network contention through polling of a common location.
* Assumes that its counters are large enough to accomodate the maximum number of simultaneous requests for the lock.
* Neither Anderson nor Graunke and Thakkar included the ticket lock in their experiments.
* Ticket algorithm can be improved using proportional backoff.

## Array based queuing locks
* Each processor spins on a different location, in a different cache line.
* Anderson claims that his array based queuing lock outperforms a test and set lock with exponential back off when more than six processors are competing for access.
* Graunke and Thakkar's experiments indicate that their lock outperforms a test_and_set lock when more than three processors are competing for access.
* Requires space per lock linear in the number of processors.

## MCS Lock

# Optimizing CPP - Agner Fog
* > Data that are read-only can be shared between multiple threads, while data that are modified should be separate for each thread.
* >  If so, then you can avoid simultaneous multithreading \[hyperthreading\] by using only the even-numbered logical processors (0, 2, 4, etc.)
* `alloca` can be used to dynamically allocate memory on the stack instead of the heap.
* > Big runtime frameworks. The .NET framework and the Java virtual machine are frameworks that typically take much more resources than the programs they are running. [Chapter 4]
* Conversion of `signed int` to `float` or `double` takes 4-16 clock cycles. 
* Conversion of `float` ot `int` costs around 50-100 clock cycles (unless `SSE2` or later instruction set is enabled)
* Branching instruction take approximately 0-2 clock cycles.
* Recovering from a branching misprediction takes approximately 12-25 clock cycles.
* Functional calls may take up to 4 clock cycles.
* There are four kinds of costs to multithreading:
    * > The cost of starting and stopping threads.
    * > The cost of task switching. This cost is minimized if the number of threads with the same priority is no more than the number of CPU cores.
    * > The cost of synchronizing and communicating between threads.
    * > The different threads need separate storage.
# [Optimizing subroutines in assembly language](https://www.agner.org/optimize/optimizing_assembly.pdf)
* > A serious problem when counting clock cycles is to avoid interrupts.
* > You will soon observe when measuring clock cycles that a piece of code always takes longer time the first time it is executed where it is not in the cache. Furthermore, it may take two or three iterations before the branch predictor has adapted to the code. The first measurement gives the execution time when code and data are not in the cache. The subsequent measurements give the execution time with the best possible caching.
* > Many microprocessors are able to change the clock frequency in the CPU core in response to changing workloads. The clock frequency is increased when the workload is high, and decreased when the workload is low, in order to save power. Intel processors have a "core clock counter" which counts at the actual core clock frequency. The core clock counter gives more consistent and reproducible results. This is useful when comparing alternative implementations of a piece of code, while the time stamp counter gives a more realistic measure of how long time the code actually takes to execute. A device driver is needed to enable the core clock counter because this requires privileged access. Once the core clock counter is activated, it can be read without privileged mode. A test program that can enable the core clock counter is mentioned below.

# [Intel CPUs: P-state, C-state, Turbo Boost, CPU frequency, etc.](https://vstinner.github.io/intel-cpus.html)
* Covers important components to configure in order to create a reliable testing machine.
* `cat /proc/cpuinfo` retrives CPU info
## P-States 
> The processor P-state is the capability of running the processor at different voltage and/or frequency levels. Generally, P0 is the highest state resulting in maximum performance, while P1, P2, and so on, will save power but at some penalty to CPU performance.

# [Lock Scaling Analysis on Intel® Xeon® Processors](https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/xeon-lock-scaling-analysis-paper.pdf)

* Benchmarks the relative contended performance of the Xeon Phi E5-2600 over X5600
* Keeps the number of threads per socket fixed (6 threads)
* Mentions the importance that locking algorithms should scale.
* > If software persists in operating in modes where all the threads that hardware makes available are simultaneously attempting to use the same hardware resources, the observed scaling will degrade.
* Makes use of two important parameters when running their benchmarks, which are:
    * > The time that each thread spends in the critical section (critical section time). This latency represents how much work is done by a thread each time it acquires the lock.
    * > The time each thread waits, after leaving the critical section, before attempting to enter the critical section again (re-entry time). This latency represents how much work, that is not relevant to the critical shared resources, is being done by each thread, while it does not own the critical section resource.
* Their critical section time scales from 10ns to 500ns (same with re-entry time).
* Intel showed that when the critical section time exceeds 300ns, the Intel Xeon processor E5-2600 transfers spinlock ownership at a slower rate (8-12% slower). There is also a sharper variation when the critical section falls in the 200-300ns interval.
* > In general, this analysis should apply to any locking algorithm that involves multiple threads polling a shared address waiting for a particular value to be written to that address. When such a lock algorithm takes the work interval close to zero, one can expect performance degradation and unpredictability.
* > When predictable performance is desired, such a lock algorithm needs reasonably long critical section and re-entry times. In a software implementation that takes either of those down to very small values, results become unpredictable and a tightly contended lock is guaranteed not to scale properly.
* > __A micro-benchmark that aims to study lock performance cannot give a reasonable
    > account of how a given lock algorithm will behave on real software, if the study of the
    > micro-benchmark does not include a study of the sensitivity to these two work intervals
    > and does not factor in whether the spread of latencies it has evaluated are representa-
    > tive of the application of interest.__

# Intel® 64 and IA-32 Architectures Optimization Reference Manual
## Memory Sub-system - Bus Characterization
* B-71 - $\text{Bus Utilization = }\frac{\text{BUS\_TRANS\_ANY.ALL\_AGENTS} * 2}{\text{CPU\_CLK\_UNHALTED.BUS}} * 100$
## B.8.10.2 Modified Cache Lines Eviction
$\text{L2 Modified Lines Eviction Rate} = \frac{\text{L2\_M\_LINES\_OUT.SELF.ANY}}{\text{INST\_RETIRED.ANY}}$
> When a new cache line is brought from memory, an existing cache line, possibly modified, is evicted from
    >the L2 cache to make space for the new line. Frequent evictions of modified lines from the L2 cache
    >increase the latency of the L2 cache misses and consume bus bandwidth.
## B.8.9.1 Modified Data Sharing
$\text{Modified Data Sharing Ratio} = \frac{\text{EXT\_SNOOP.ALL\_AGENTS.HITM}}{\text{INST\_RETIRED.ANY}}$
> Frequent occurrences of modified data sharing may be due to two threads using and modifying data laid
> in one cache line. Modified data sharing causes L2 cache misses. When it happens unintentionally (aka
> false sharing) it usually causes demand misses that have high penalty. When false sharing is removed
> code performance can dramatically improve.