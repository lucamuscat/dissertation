/**
 * @file enqueue_dequeue.c
 * @brief Measure the performance of a queue's enqueue-dequeue pairs
 *
 * The benchmark setup is similar to ConcurrencyFreak's CR Turn Queue
 * benchmark. This setup in particular was used to benchmark a small number
 * of queues, the results of which were published in an academic paper.
 
 * A number of queue benchmarks were evaluated before taking this approach.
 * Most C/C++ benchmarks and queue implementations ignored the fact that the dynamic
 * allocation of memory (malloc/free for C, new/delete for C++) is not non-blocking.

 * Another flaw of most benchmarks is that memory was being allocated and deallocated
 * without any proper memory management procedures. This may cause segmentation
 * faults in specific interleavings, as a node may be held by one thread, but freed
 * by another.

 * For instance, if one prior enqueue has occurred, and another two threads enqueue and dequeue
 * at the same time, the enqueueing thread might be making use of a node which has been
 * freed by the other thread's dequeue.

 * Source - https://github.com/pramalhe/ConcurrencyFreaks/blob/c61189546805c67792df7931f9484e09a3cda3bf/CPP/papers/crturnqueue/include/BenchmarkQ.hpp
 * @warning Make sure that turbo boost and CPU frequency scaling settings are turned off.
 * Failure to do so will cause the DELAY macro to misbehave and the PAPI_get_real_cyc()
 * function to report the actual total cycles.
 *
 * https://web.eece.maine.edu/~vweaver/papers/papi/papi_v5_changes.pdf §13.17
 */

// Needed for setting thread affinity
#define _GNU_SOURCE
#include <papi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>
#include "../../affinity_utils.h"
#include "../../test_utils.h"
#include "../queue.h"

static pthread_barrier_t barrier;
static delay_t delay;

typedef struct thread_args
{
    void* queue;
    size_t num_of_iterations;
    size_t num_of_warm_up_iterations;
    readings_t* readings;
    pthread_t tid;
} CACHE_ALIGNED /* Align to cache to avoid false sharing */ thread_args;

void* thread_fn(void* in_args)
{
    thread_args* args = in_args;
    void* dequeued_item = NULL;
    int enqueued_item = 10;
    register_thread(args->num_of_iterations + args->num_of_warm_up_iterations);
    pthread_barrier_wait(&barrier);
    for (size_t i = 0; i < args->num_of_warm_up_iterations; ++i)
    {
        enqueue(args->queue, &enqueued_item);
        DELAY_OPS(delay.num_of_nops);
        if (dequeue(args->queue, &dequeued_item))
            assert(*((int*)dequeued_item) == enqueued_item);
        DELAY_OPS(delay.num_of_nops);
    }

    // Empty the queue
    while (dequeue(args->queue, &dequeued_item))
        assert(*((int*)dequeued_item) == enqueued_item);

    // Make sure that each thread executes the test at the same time.
    pthread_barrier_wait(&barrier);
    start_readings(args->readings);
    for (size_t i = 0; i < args->num_of_iterations; ++i)
    {
        enqueue(args->queue, &enqueued_item);
        DELAY_OPS(delay.num_of_nops);
        if (dequeue(args->queue, &dequeued_item))
            assert(*((int*)dequeued_item) == enqueued_item);
        DELAY_OPS(delay.num_of_nops);
    }
    delta_readings(args->readings, args->num_of_iterations);
    adjust_readings_for_delay(args->readings, &delay);
    adjust_readings_for_delay(args->readings, &delay);
    // This barrier protects against the scenario where a thread
    // finishes before another thread that frees memory that is used by another
    // thread
    pthread_barrier_wait(&barrier);
    cleanup_thread();
    // Make sure to synchronize all changes to args
    atomic_thread_fence(memory_order_seq_cst);
    return 0;
}

int main(int argc, char** argv)
{    
    size_t num_of_threads, delay_ns;
    handle_queue_args(argc, argv, &num_of_threads, &delay_ns);

    calibrate_delay(&delay, delay_ns);

    assert(sizeof(thread_args) == CACHE_LINE_SIZE);

    ASSERT_TRUE(!pthread_barrier_init(&barrier, NULL, num_of_threads), "Failed to create pthread_barrier");

    thread_args* args = (thread_args*)malloc(sizeof(thread_args) * num_of_threads);
    ASSERT_NOT_NULL(args);

    long long total_run_time_ns = PAPI_get_real_nsec();

    readings_t** readings = create_readings_2d(num_of_threads, TEST_RERUNS);

    for (size_t i = 0; i < TEST_RERUNS; ++i)
    {
        size_t accumulated_iterations = 0;
        void* queue;
        ASSERT_TRUE(create_queue(&queue), "Failed to create queue");
        for (size_t j = 0; j < num_of_threads; ++j)
        {
            args[j].queue = queue;
            args[j].readings = readings[j];
            args[j].num_of_warm_up_iterations = WARMUP_ITERATIONS/num_of_threads;
            args[j].num_of_iterations = iterations_per_thread(num_of_threads, j, TEST_ITERATIONS);
            accumulated_iterations += args[j].num_of_iterations;

            pthread_attr_t attr = create_thread_affinity_attr(j);
            pthread_create(&args[j].tid, &attr, thread_fn, &args[j]);
        }

        // Sanity check; Ensure that the total iterations match up.
        assert(accumulated_iterations == TEST_ITERATIONS);

        for (size_t j = 0; j < num_of_threads; ++j)
        {
            pthread_join(args[j].tid, NULL);
        }
        destroy_queue(&queue);
    }

    total_run_time_ns = PAPI_get_real_nsec() - total_run_time_ns;

    readings_t* aggregates = aggregate_readings_2d(readings, num_of_threads, TEST_RERUNS);

    printf("\n\"%s\", %zu, %zu, ", get_queue_name(), num_of_threads, delay_ns);
    display_readings(aggregates);
    printf(", %lld", total_run_time_ns);

    // for (size_t i = 0; i < num_of_threads; ++i)
    // {
    //     destroy_readings(&temp[i]);
    // }
    //free(temp);
    //destroy_readings(&readings);
    pthread_barrier_destroy(&barrier);
    //free(args);

    return 0;
}
