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
#include "../../test_utils.h"
#include "../queue.h"

#define TEST_ITERATIONS 100000
#define TEST_RERUNS 1
#define NANO_TO_MINUTE 1000000000*60
#define RANDOM_RANGE 5

static pthread_barrier_t barrier;
static int* random_delays_ns;

typedef struct thread_args
{
    void* queue;
    size_t num_of_iterations;
    double* total_cycle_diff;
    double* total_ns_diff;
    pthread_mutex_t* mutex;
    pthread_t tid;
} __attribute__((aligned(CACHE_LINE_SIZE))) thread_args;

void* thread_fn(void* in_args)
{
    thread_args* args = in_args;
    long long cycle_diff;
    long long ns_diff;
    void* dequeued_item = NULL;
    double inner_total_diff = 0.0;
    double inner_total_ns_diff = 0.0;
    
    for (size_t j = 0; j < TEST_RERUNS; ++j)
    {
        // Make sure that each thread executes the test at the same time.
        pthread_barrier_wait(&barrier);
        //TODO: Use PAPI_TOT_CYC instead of PAPI_REF_CYC
        cycle_diff = PAPI_get_real_cyc();
        ns_diff = PAPI_get_real_nsec();
        for (size_t i = 0; i < args->num_of_iterations; ++i)
        {
            enqueue(args->queue, &cycle_diff);
            DELAY(random_delays_ns[i]);
            dequeue(args->queue, &dequeued_item);
            DELAY(random_delays_ns[i]);
        }
        // FIXME: Remove delay from ns_diff and cycle_diff
        cycle_diff = PAPI_get_real_cyc() - cycle_diff;
        ns_diff = PAPI_get_real_nsec() - ns_diff;
        inner_total_diff += ((double)cycle_diff) / args->num_of_iterations;
        inner_total_ns_diff += ((double)ns_diff) / args->num_of_iterations;
    }
    pthread_mutex_lock(args->mutex);
    *args->total_cycle_diff += inner_total_diff / TEST_RERUNS;
    *args->total_ns_diff += inner_total_ns_diff / TEST_RERUNS;
    pthread_mutex_unlock(args->mutex);
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

    pthread_mutex_t mutex;
    PASS_LOG(pthread_mutex_init(&mutex, NULL) == 0, "Failed to create pthread_mutex");

    double total_cycle_diff = 0, total_ns_diff = 0;

    // Ensure that exactly TEST_ITERATIONS iterations occur, as integer division
    // might cause truncation and either increase or reduce the total number of
    // iterations.
    const int iterations_per_thread = (int)floor(((double)TEST_ITERATIONS) / num_of_threads);
    const int final_thread_iterations = TEST_ITERATIONS - (iterations_per_thread * (num_of_threads - 1));

    random_delays_ns = generate_random_numbers(TEST_ITERATIONS, delay_ns - RANDOM_RANGE, delay_ns + RANDOM_RANGE);

    assert((iterations_per_thread * (num_of_threads - 1) + final_thread_iterations) == TEST_ITERATIONS);
    assert(sizeof(thread_args) == CACHE_LINE_SIZE);

    PASS_LOG(pthread_barrier_init(&barrier, NULL, num_of_threads) == 0, "Failed to create pthread_barrier");

    thread_args* args = (thread_args*)malloc(sizeof(thread_args) * num_of_threads);
    size_t total_iterations = 0;
    long long total_run_time_ns = PAPI_get_real_nsec();
    for (int i = 0; i < num_of_threads; ++i)
    {
        args[i].mutex = &mutex;
        args[i].queue = queue;
        args[i].total_cycle_diff = &total_cycle_diff;
        args[i].total_ns_diff = &total_ns_diff;
        args[i].num_of_iterations = i < num_of_threads - 1 ? iterations_per_thread : final_thread_iterations;
        total_iterations += args[i].num_of_iterations;
        pthread_create(&args[i].tid, NULL, thread_fn, &args[i]);
    }

    // Sanity check; Ensure that the total iterations match up.
    assert(total_iterations == TEST_ITERATIONS);
    
    for (int i = 0; i < num_of_threads; ++i)
    {
        pthread_join(args[i].tid, NULL);
    }

    total_run_time_ns = PAPI_get_real_nsec() - total_run_time_ns;
    
    double average_cycles = total_cycle_diff / num_of_threads;
    double average_ns = total_ns_diff / num_of_threads;
    
    printf("\"%s\", %d, %f, %f, %d, %lld\n", get_queue_name(), num_of_threads, average_cycles, average_ns, delay_ns, total_run_time_ns);

    return 0;
}
