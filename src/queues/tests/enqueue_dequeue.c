/**
 * @file enqueue_dequeue.c
 * @brief Measure the performance of a queue's enqueue-dequeue pairs
 * @warning Make sure that the turbo boost and CPU frequency scaling are turned off.
 * Failure to do so will cause the DELAY macro to misbehave and the PAPI_get_real_cyc()
 * function to not match the actual total cycles.
 */

#include <papi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>
#include "../../test_utils.h"
#include "../queue.h"

#define TEST_ITERATIONS 1000000
#define TEST_RERUNS 1000
#define NANO_TO_MINUTE 1000000000*60
#define RANDOM_RANGE 5

static pthread_barrier_t barrier;
static int* random_delays_ns;

typedef struct thread_args
{
    void* queue;
    size_t num_of_iterations;
    pthread_t tid;
    readings_t* readings;
} CACHE_ALIGNED /* Align to cache to avoid false sharing */ thread_args;

/*
 * TODO:
    1. Measure misaligned memory access counters.
 *
 * Should I be measuring the mean, or the median? 
*/
void* thread_fn(void* in_args)
{
    thread_args* args = in_args;
    void* dequeued_item = NULL;
    int enqueued_item = 0;
    
    for (size_t i = 0; i < TEST_RERUNS; ++i)
    {
        // Make sure that each thread executes the test at the same time.
        pthread_barrier_wait(&barrier);
        start_readings(args->readings);
        for (size_t j = 0; j < args->num_of_iterations; ++j)
        {
            enqueue(args->queue, &enqueued_item);
            DELAY(random_delays_ns[j]);
            dequeue(args->queue, &dequeued_item);
            DELAY(random_delays_ns[j]);
        }
        delta_readings(args->readings, args->num_of_iterations);

        while (dequeue(args->queue, &dequeued_item))
            assert(*((int*)dequeued_item) == enqueued_item);
    }
    // Make sure to synchronize all changes to args
    atomic_thread_fence(memory_order_seq_cst);
    return 0;
}

int main(int argc, char** argv)
{
    size_t num_of_threads, delay_ns;
    handle_queue_args(argc, argv, &num_of_threads, &delay_ns);

    if (delay_ns < RANDOM_RANGE)
        delay_ns = RANDOM_RANGE;

    void* queue;
    PASS_LOG(create_queue(&queue), "Failed to create queue");

    random_delays_ns = generate_random_numbers(TEST_ITERATIONS, delay_ns - RANDOM_RANGE, delay_ns + RANDOM_RANGE);

    assert(sizeof(thread_args) == CACHE_LINE_SIZE);

    PASS_LOG(pthread_barrier_init(&barrier, NULL, num_of_threads) == 0, "Failed to create pthread_barrier");

    thread_args* args = (thread_args*)malloc(sizeof(thread_args) * num_of_threads);
    P_PASS(args);

    size_t accumulated_iterations = 0;
    long long total_run_time_ns = PAPI_get_real_nsec();

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        args[i].queue = queue;
        create_readings(&args[i].readings, TEST_RERUNS);
        args[i].num_of_iterations = iterations_per_thread(num_of_threads, i, TEST_ITERATIONS);
        accumulated_iterations += args[i].num_of_iterations;
        pthread_create(&args[i].tid, NULL, thread_fn, &args[i]);
    }

    // Sanity check; Ensure that the total iterations match up.
    assert(accumulated_iterations == TEST_ITERATIONS);
    
    for (size_t i = 0; i < num_of_threads; ++i)
    {
        pthread_join(args[i].tid, NULL);
    }

    total_run_time_ns = PAPI_get_real_nsec() - total_run_time_ns;

    readings_t** temp = (readings_t**)malloc(sizeof(readings_t*) * num_of_threads);
    
    for (size_t i = 0; i < num_of_threads; ++i)
    {
        temp[i] = args[i].readings;
    }

    readings_t* readings = aggregate_readings(temp, num_of_threads, TEST_RERUNS);

    printf("\"%s\", %zu, %zu, ", get_queue_name(), num_of_threads, delay_ns);
    display_readings(readings);
    printf(", %lld\n", total_run_time_ns);

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
