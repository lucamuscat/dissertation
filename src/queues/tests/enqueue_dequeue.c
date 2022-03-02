#include <papi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../test_utils.h"
#include "../queue.h"

#define TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD 1000000
#define TOTAL_RERUNS 100000
#define NANO_TO_SEC 1000000000

int main(int argc, char** argv)
{
    int num_of_threads = handle_args(argc, argv);
    
    if (num_of_threads == -1)
        return num_of_threads;
    
    void* queue;

    
    PASS((create_queue(&queue)));
    long long total_cycle_diff = 0;
    long long total_ns_diff = 0;
    long long total_run_time_ns = PAPI_get_real_nsec();
    
    #pragma omp parallel shared(queue, total_cycle_diff, total_ns_diff)
    {
        long long cycle_diff;
        long long ns_diff;
        void* dequeued_item = NULL;
        long long inner_total_diff = 0LL;
        long long inner_total_ns_diff = 0LL;
        for (size_t j = 0; j < TOTAL_RERUNS; ++j)
        {
            cycle_diff = PAPI_get_real_cyc();
            ns_diff = PAPI_get_real_nsec();
            for (size_t i = 0; i < TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD; ++i)
            {
                enqueue(queue, &cycle_diff);
                dequeue(queue, &dequeued_item);
            }
            cycle_diff = PAPI_get_real_cyc() - cycle_diff;
            ns_diff = PAPI_get_real_nsec() - ns_diff;
            inner_total_diff += cycle_diff / TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD;
            inner_total_ns_diff += ns_diff / TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD;
        }
        #pragma omp critical
        {
            total_cycle_diff += inner_total_diff / TOTAL_RERUNS;
            total_ns_diff += inner_total_ns_diff / TOTAL_RERUNS;
        }

    }

    total_run_time_ns = total_run_time_ns - PAPI_get_real_nsec();
    
    long long average_cycles = total_cycle_diff / num_of_threads;

    printf("Total runtime: %lld", total_run_time_ns / NANO_TO_SEC);
    printf("Average Per Thread: %lld cycles\n", average_cycles);

    return 0;
}
