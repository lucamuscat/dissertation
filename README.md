---
Author:
  - Luca Muscat
Supervisor:
  - Prof Kevin Vella
---
# A Comparative Study of Concurrent Queueing Algorithms & Their Performance
## Abstract
Queues are among the most ubiquitous data structures in the field of Computing
Science. With the advent of multiprocessor programming, concurrent queues are
at the core of many concurrent and distributed algorithms.

We were able to
identify few studies surveying and replicating concurrent queuing algorithms
from their seminal works. This work is an experimental study, with the aim of
comparing the performance of multiple concurrent queues with one another, under different levels
of contention, whilst replicating the results of other researchers. The
production of a benchmarking framework for concurrent queues also forms part of
this study's contributions. 

We successfully replicate results to support the
claims that non-blocking queues are superior to blocking ones under high
contention, and remain competitive under low contention. We also expose a
number of unreported, yet significant biases in a number of classical works. 

A limiting factor of this study is that each algorithm is benchmarked on
consumer-grade hardware, fortunately, the benchmarking framework is developed
to run on multiple machines, making this limitation transient in scope.

## Structure of Repository
### `src`
Contains the implemented queueing algorithms and the benchmarking framework.
### `src/locks`
Contains lock/mutex implementations.
### `src/queues/non-blocking`
Contains the implementations for non-blocking queues.
### `src/queues/blocking`
Contains the implementations for blocking queues.
### `src/utils`
Contains the scripts used to produce plots and analyse the benchmarking framework's results. `src/utils/scratchpad.ipynb` is a jupyter notebook that gives a lower-level view of the study's _Evaluation_ chapter. `src/utils/plot.py` produces the plots used in the _Evaluation_ chapter, whilst `src/utils/interpret_counters.py` takes the counters from the benchmarking framework's results and outputs them in an HTML table and an excel spreadsheet.
### `src/write_up/thesis`
Contains all the LaTex files related to the write up of my dissertation.
### Setup
* Set the `PHYSICAL_CORES` macro (inside of `src/affinity_utils.h`) to the number of physical cores your CPU has 
## Extending the Framework
In order to benchmark your own concurrent queueing algorithm, you need to identify if the queue is blocking (requires a lock/mutex) or non-blocking. Once a new queue is placed in their respective directory, the `Makefile` variables `BLOCKING_QUEUE_NAMES` and `NONBLOCKING_QUEUE_NAMES` are to be adjusted accordingly.
## Dependencies
*  `PAPI`. The `install_papi.sh` script can be used to install it;
* `make`;
* `pdflatex`;
## Build Instructions
* Call `make` to build the entire project, all binaries are placed inside the `build` directory. See the `Makefile` for further recipes.
* Call `cd write_up/thesis && make references` to compile a PDF of the latex documents.
* Call `py src/utils/plot.py` inside the same directory as the benchmarking framework results to produce the study's plots.
* Call `py src/utils/interpret_counters.py` inside the same directory as the benchmarking framework results to produce detailed tables of the counters in the results.
## Running the Benchmarking Framework
First, execute `setup_machine.sh`, then execute the `run_benchmarks.sh` script.

