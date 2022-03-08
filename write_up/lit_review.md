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

# PAPI performance measurement library.
PAPI is a performance measurement library that offer abstractions to accessing hardware counters [http://icl.cs.utk.edu/papi/]. PAPI offers both a low and a high level API [http://icl.cs.utk.edu/papi/docs/] that offer varying levels of granularity of control over several different types of measurements. Utility commands are provided to aid with the adoption of this tool; most notably, it comes with a script that measures the latency of each of its performance measuring functions. PAPI follows a modular design philosophy, this is evident in its 'Components' [http://icl.cs.utk.edu/papi/docs/d9/dc0/component_readme.html] feature where users are able to install adapters for hardware counters that are not supported out of the box (for instance, GPU hardware counters, CPU temperature counters, and so on).

# Art of Multiprocessor Programming - Herlihy
* A text book that helps with obtaining a fundemental understanding of the theoretical and practical side of parallel programming.

# The Performance of Spin Lock Alternatives for Shared-Memory MultiProcessors - [@anderson1990performance]
This paper examines if efficient algorithms for spin locks exist given hardware support for atomic instructions. Anderson shows that the simple approaches to spin locks have poor performance due to amount of bandwidth consumed in communication channels.

Andreson's benchmark simulates the performance of an application with a small critical section which shows spin lock latency and performance for small and large amounts of contention. Anderson also notes that similar results have been measured using a fixed delay between lock accesses.

# Is Parallel Programming Hard, And, If So, What Can You Do About It [@mckenney2017parallel]?
This book focuses on shared-memory programming with an emphasis on low level software, such as OS kernels and low-level libraries. The topics covered in this book range from the use of POSIX threads and locks all the way to advanced synchronization techniques, such as non-blocking synchronization, memory barriers, and read copy update semantics (among other things). McKenney dedications a section of the book to performance estimation, where he mentions different ways where errors may creep up in microbenchmark readings.

# A Methodology to Characterize Critical Section Bottlenecks in DSM Multiprocessors [@sahelices2009methodology]
This paper introduces a new methodology for tuning performance and critical sections inside of parallel programs. The authors characterize critical sections according to lock contention and degree of data sharing. By measuring the latency (time between acquiring the lock and releasing it), the authors were able to determine a number of inefficiencies caused by data sharing patterns and the chosen data layout. Notably, benchmarks were using a multiprocessor simulator (RSIM); allowing the authors to take fine grained and accurate statistics.

# Implementing Lock-Free Queues [@valois1994implementing]
Valois covers multiple lock-free queue implementations using linked lists, noting that most of the algorithms can be implemented using the Compare and Swap atomic primitive. Valois notes that the algorithms that make use of the CAS atomic primitive suffer from the ABA problem. 

Valois proposed the safe read protocol, which was a solution to the ABA problem that did not require stronger versions of CAS (such as DCAS) primitive. A new lock free queue that made use of  arrays and CAS was also proposed. The novel lock-free queue outperformed most lock-based queues both sequentially and under contention. Valois measures the performance of each lock-free queue with and without protection against the ABA problem, noting that even with the overhead, lock-free queues are still competitive with their lock-based counterparts.

Most notably, Valois measured the performance of each queue affected by random delay. Valois did this by simulating a delay of a random amount of cycles following an exponential distribution with a mean of 1000 cycles and a 10% chance of occuring. The results showed that the response time of the lock-free implementations were approximately 17% of that of the spin lock protected queues, highlighting the superiority of lock-based algorithms. The massive difference in performance was attributed to the blocking nature of the spin lock, noting that a delayed spin lock not only affected the current operation, but also the other pending critical sections.

Valois also concludes that spin lock algorithms require some type of backoff mechanism in order to decrease the degraded performance caused due to interconnect saturation.

Performance analysis was conducted on a parallel architecture simulator called Proteus. The simulator was a work around to the infeasability of obtaining the necessary hardware and resources needed to obtain readings. Further exasperating this issue, not all machines used architectures that supported CAS primitives; most machines did not have good instrumentation facilities either. Porting code over to other architectures would have proved to be very time consuming.

Fortunately, these are mostly issues of the past, as hardware is more accessible and instrumentation facilities have matured over the last few decades.

(Compared sequential latency using histograms)
# [Performance Analysis Methodology](https://www.brendangregg.com/methodology.html)
Gregg describes several performance 'Anti-Methodologies' and 'Methodologies'. Anti-Methodologies are methods of benchmarking performance that do not lead to accurate or correct results. This is an article that anyone in the field of performance analysis should read, as it reveals methodical and structured ways of carrying out performance analysis. 


# Wait-Free Queues With Multiple Enqueuers and Dequeuers [@kogan2011wait]
Kogan et. al propose a novel and efficient wait-free queue that is on based link-lists and thread helping (ie. threads helping other threads by completing their intended operation for them). Similar to the filter and the bakery locks[quote herlihy], the queue is required to read and write to _n_ distinct locations, where n is the maximum number of concurrent threads. This is one of the unfortunate drawbacks of implementing a deadlock-free algorithm. The proposed queue heavily relies on Read Modify Write atomic primitives in order to function correctly and to remain in a consistent state.

The proposed wait-free queue assumes that the garbage collector is responsible for preventing the ABA problem. Since a wait-free garbage collector does not exist, Kogan et al propose that the hazard pointer technique can be used to solve the ABA problem.

Performance of each queue mentioned in the paper was collected using the following methodology:
+ `enqueue-dequeue` pairs: Starting with an empty queue, each thread iteratively performs an enqueue followed by a dequeue.
+ `50% enqueues`: The queue is intialized with 1000 elements, each thread does a 'coin toss' to decide 