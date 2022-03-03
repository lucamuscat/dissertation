#include <papi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../test_utils.h"
#include "../queue.h"

#define TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD 100000LL
#define TOTAL_RERUNS 100000LL
#define NANO_TO_MINUTE 1000000000*60

int main(int argc, char** argv)
{
    int num_of_threads = handle_args(argc, argv);
    
    if (num_of_threads == -1)
        return num_of_threads;
    
    void* queue;

    
    PASS((create_queue(&queue)));
    double total_cycle_diff = 0;
    double total_ns_diff = 0;
    long long total_run_time_ns = PAPI_get_real_nsec();
    
    #pragma omp parallel shared(queue, total_cycle_diff, total_ns_diff)
    {
        long long cycle_diff;
        long long ns_diff;
        void* dequeued_item = NULL;
        double inner_total_diff = 0.0;
        double inner_total_ns_diff = 0.0;
        for (size_t j = 0; j < TOTAL_RERUNS; ++j)
        {
            cycle_diff = PAPI_get_real_cyc();
            ns_diff = PAPI_get_real_nsec();
            for (size_t i = 0; i < TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD; ++i)
            {
                enqueue(queue, &cycle_diff);
                dequeue(queue, &dequeued_item);
            }
            inner_total_diff += (((double)PAPI_get_real_cyc()) - cycle_diff)/TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD;
            inner_total_ns_diff += (((double)PAPI_get_real_nsec()) - ns_diff)/TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD;
        }
        #pragma omp critical
        {
            total_cycle_diff += inner_total_diff / TOTAL_RERUNS;
            total_ns_diff += inner_total_ns_diff / TOTAL_RERUNS;
        }

    }

    total_run_time_ns = PAPI_get_real_nsec() - total_run_time_ns;
    
    double average_cycles = total_cycle_diff / num_of_threads;
    double average_ns = total_ns_diff / num_of_threads;
    
    printf("%s, %f, %f, %lld\n", argv[0], average_cycles, average_ns, total_run_time_ns);

    return 0;
}
