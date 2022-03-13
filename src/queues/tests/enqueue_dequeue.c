/**
 * @file enqueue_dequeue.c
 * @brief Measure the performance of a queue's enqueue-dequeue pairs
 * @warning Make sure that the turbo boost and CPU frequency scaling are turned off.
 * Failure to do so will cause the DELAY macro to misbehave and the PAPI_get_real_cyc()
 * function to not match the actual total cycles.
 */

#include <papi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>
#include "../../test_utils.h"
#include "../queue.h"

#define TEST_ITERATIONS 100000
#define TEST_RERUNS 5
#define NANO_TO_MINUTE 1000000000*60
#define RANDOM_RANGE 5

static pthread_barrier_t barrier;
static int* random_delays_ns;

typedef struct thread_args
{
    void* queue;
    size_t num_of_iterations;
    pthread_t tid;
    double* cycles_readings;
    double* nsec_readings;
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
    long long cycle_diff;
    long long ns_diff;
    void* dequeued_item = NULL;
    int enqueued_item = 0;
    
    for (size_t i = 0; i < TEST_RERUNS; ++i)
    {
        // Make sure that each thread executes the test at the same time.
        pthread_barrier_wait(&barrier);
        cycle_diff = PAPI_get_real_cyc();
        ns_diff = PAPI_get_real_nsec();
        for (size_t j = 0; j < args->num_of_iterations; ++j)
        {
            enqueue(args->queue, &enqueued_item);
            DELAY(random_delays_ns[j]);
            dequeue(args->queue, &dequeued_item);
            DELAY(random_delays_ns[j]);
        }
        // FIXME: Remove delay from ns_diff and cycle_diff
        args->cycles_readings[i] = (PAPI_get_real_cyc() - cycle_diff) / args->num_of_iterations;
        args->nsec_readings[i] = (PAPI_get_real_nsec() - ns_diff) / args->num_of_iterations;

        // We're working with concurrent queues.
        // So might aswell concurrently empty them.
        while (dequeue(args->queue, &dequeued_item));
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
        args[i].cycles_readings = (double*)malloc(sizeof(double) * TEST_RERUNS);
        args[i].nsec_readings = (double*)malloc(sizeof(double) * TEST_RERUNS);
        P_PASS(args[i].cycles_readings);
        P_PASS(args[i].nsec_readings);
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

    double** cycles = (double**)malloc(sizeof(double*) * num_of_threads);
    double** nanoseconds = (double**)malloc(sizeof(double*) * num_of_threads);

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        cycles[i] = args[i].cycles_readings;
        nanoseconds[i] = args[i].nsec_readings;
    }
    
    double average_cycles = mean_2d(cycles, num_of_threads, TEST_RERUNS);
    double stdev_cycles = stdev_2d(cycles, num_of_threads, TEST_RERUNS);
    double average_nsecs = mean_2d(nanoseconds, num_of_threads, TEST_RERUNS);
    double stdev_nsecs = stdev_2d(nanoseconds, num_of_threads, TEST_RERUNS);

    printf("\"%s\", %zu, %zu, ", get_queue_name(), num_of_threads, delay_ns);
    printf("%f, %f, %f, %f, ", average_cycles, average_nsecs, stdev_cycles, stdev_nsecs);
    printf("%f, %f, ", stdev_cycles / average_cycles, stdev_nsecs / average_nsecs);
    printf("%lld\n", total_run_time_ns);

    pthread_barrier_destroy(&barrier);
    for (size_t i = 0; i < num_of_threads; ++i)
    {
        free(cycles[i]);
        free(nanoseconds[i]);
    }
    free(cycles);
    free(nanoseconds);
    free(args);

    return 0;
}
