#include <papi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../test_utils.h"
#include "../queue.h"

#define TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD 100000

int main(int argc, char** argv)
{
    int num_of_threads = handle_args(argc, argv);
    
    if (num_of_threads == -1)
        return num_of_threads;
    
    void* queue;
    
    PASS((create_queue(&queue)));
    long long diff;
    long long total_diff = 0;

    #pragma omp parallel shared(queue, total_diff) private(diff)
    {
        void* dequeued_item = NULL;
        diff = PAPI_get_real_cyc();
        for (size_t i = 0; i < TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD; ++i)
        {
            enqueue(queue, &diff);
            dequeue(queue, &dequeued_item);
        }
        diff = PAPI_get_real_cyc() - diff;
        #pragma omp critical
        total_diff += diff;
    }

    long long average_per_thread = total_diff / num_of_threads;
    long long average_per_iteration = average_per_thread / TOTAL_ENQUEUE_DEQUEUE_PAIRS_PER_THREAD;
    
    printf("Average Per Thread: %lld cycles\n", average_per_thread);
    printf("Average Iteration: %lld cycles\n", average_per_iteration);

    return 0;
}
