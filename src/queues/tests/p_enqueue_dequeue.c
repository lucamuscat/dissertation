/**
 * @file p_enqueue_dequeue.c
 * @brief Each queue will have probabilities p, and (1-p) to enqueue and dequeue
 * respectively.
 * @warning Make sure that the turbo boost and CPU frequency scaling are turned off.
 * Failure to do so will cause the DELAY macro to misbehave and the PAPI_get_real_cyc()
 * function to not match the actual total cycles.
 *
 * High values of p may cause the system to run out of memory if TEST_ITERATIONS
 * is set to a high value.
 */

#include "../../affinity_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>

#include "../../test_utils.h"
#include "../../assertion_utils.h"
#include "../queue.h"

delay_t delay;
pthread_barrier_t barrier;
size_t _Atomic prefill_counter;

typedef struct thread_args
{
    void* queue;
    int* stats;
    double p;
    double* random_probabilities;
    size_t num_of_iterations;
    size_t num_of_warm_up_iterations;
    readings_t* readings;
    pthread_t tid;
} CACHE_ALIGNED thread_args;

void* thread_fn(void* in_args)
{
    thread_args* args = in_args;

    size_t num_of_enqueues = count_enqueues_from_probabilities(
        args->random_probabilities,
        args->p,
        args->num_of_iterations
    );

    size_t num_of_enqueues_warmup = count_enqueues_from_probabilities(
        args->random_probabilities,
        args->p,
        args->num_of_warm_up_iterations
    );

    size_t total_enqueues = num_of_enqueues + num_of_enqueues_warmup + PREFILL_SIZE;

    register_thread(total_enqueues);

    int enqueued_item = 10;
    void* dequeued_item[PAD_TO_CACHELINE(void*)];

    pthread_barrier_wait(&barrier);
    for (size_t i = 0; i < args->num_of_warm_up_iterations; ++i)
    {
        if (args->random_probabilities[i] < args->p)
            enqueue(args->queue, &enqueued_item);
        else if (dequeue(args->queue, &dequeued_item[0]))
            assert(*((int*)dequeued_item[0]) == enqueued_item);

        DELAY_OPS(delay.num_of_nops);
    }

    // Empty the queue
    while (dequeue(args->queue, &dequeued_item[0]))
        assert(*((int*)dequeued_item[0]) == enqueued_item);

    while (atomic_fetch_add(&prefill_counter, 1) < PREFILL_SIZE)
    {
        enqueue(args->queue, &enqueued_item);
    }

    pthread_barrier_wait(&barrier);
    start_readings(args->readings);

    for (size_t i = 0; i < args->num_of_iterations; ++i)
    {
        if (args->random_probabilities[i] < args->p)
            enqueue(args->queue, (void**)&enqueued_item);
        else if (dequeue(args->queue, (void*)&dequeued_item[0]))
            assert(*((int*)dequeued_item[0]) == enqueued_item);

        DELAY_OPS(delay.num_of_nops);
    }

    delta_readings(args->readings, args->num_of_iterations);
    adjust_readings_for_delay(args->readings, &delay);
    pthread_barrier_wait(&barrier);
    get_stats(args->stats);
    cleanup_thread();

    atomic_thread_fence(memory_order_seq_cst);
    return NULL;
}

void p_handle_args(int argc, char** argv, size_t* num_of_threads, size_t* delay_ns, double* p)
{
    if (argc != 4)
    {
        fprintf(stderr, "Missing Args\n");
        fprintf(stderr, "\targ1: uint - Number of threads\n");
        fprintf(stderr, "\targ2: uint - Reentrancy delay in nanoseconds\n");
        fprintf(stderr, "\targ3: double - Probability of enqueue (p)\n");
        exit(EXIT_FAILURE);
    }

    char* pEnd[3] = { "" };
    *num_of_threads = strtoul(argv[1], &pEnd[0], 10);
    *delay_ns = strtoul(argv[2], &pEnd[1], 10);
    *p = strtod(argv[3], &pEnd[2]);
}

int main(int argc, char** argv)
{
    size_t num_of_threads, delay_ns;
    double p;
    p_handle_args(argc, argv, &num_of_threads, &delay_ns, &p);

    long long total_run_time_ns = PAPI_get_real_nsec();
    calibrate_delay(&delay, delay_ns);

    assert(p >= 0);
    assert(p <= 1);

    double** random_probabilities = (double**)malloc(sizeof(double*) * num_of_threads);
    ASSERT_NOT_NULL(random_probabilities);

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        int thread_iterations = iterations_per_thread(num_of_threads, i, TEST_ITERATIONS);
        random_probabilities[i] = (double*)malloc(sizeof(double) * thread_iterations);
        for(int j = 0; j < thread_iterations; ++j)
            random_probabilities[i][j] = drand48();
    }

    thread_args* args = (thread_args*)malloc(sizeof(thread_args) * num_of_threads);
    ASSERT_NOT_NULL(args);

    readings_t** readings = create_readings_2d(num_of_threads, TEST_RERUNS);
    int** per_thread_stats = create_stats(num_of_threads);
    int** accumulated_stats = create_stats(TEST_RERUNS);
    
    // pthread_barrier_init returns zero when successful, however, zero is a falsy value in c.
    ASSERT_TRUE(!pthread_barrier_init(&barrier, NULL, num_of_threads), "Failed to create barrier");
    
    for (size_t i = 0; i < TEST_RERUNS; ++i)
    {
        void* queue;
        ASSERT_TRUE(create_queue(&queue), "Failed to create queue");
        atomic_store(&prefill_counter, 0);
        for (size_t j = 0; j < num_of_threads; ++j)
        {
            int thread_iterations = iterations_per_thread(num_of_threads, j, TEST_ITERATIONS);
            args[j].readings = readings[j];
            args[j].num_of_warm_up_iterations = TEST_ITERATIONS / num_of_threads;
            args[j].num_of_iterations = thread_iterations;
            args[j].queue = queue;
            args[j].p = p;
            args[j].random_probabilities = random_probabilities[j];
            reset_stats(per_thread_stats[j]);
            args[j].stats = per_thread_stats[j];
            pthread_create(&args[j].tid, NULL, thread_fn, (void*)&args[j]);
        }

        for (size_t j = 0; j < num_of_threads; ++j)
        {
            pthread_join(args[j].tid, NULL);
        }

        for (size_t j = 1; j < num_of_threads; ++j)
        {
            add_stats(per_thread_stats[0], per_thread_stats[j], per_thread_stats[0]);
        }

        copy_stats(accumulated_stats[i], per_thread_stats[0]);
        destroy_queue(&queue);
    }

    total_run_time_ns = PAPI_get_real_nsec() - total_run_time_ns;
    
    readings_t* aggregates = aggregate_readings_2d(readings, num_of_threads, TEST_RERUNS);

    double total_run_time_minutes = NANO_TO_MINUTE(total_run_time_ns);
    
    printf("\n\"%s\",%ld,%zu,", get_queue_name(), num_of_threads, delay_ns);
    display_readings(aggregates);
    printf(",%f", total_run_time_minutes);
    display_stats(accumulated_stats, TEST_RERUNS);
    pthread_barrier_destroy(&barrier);

    return EXIT_SUCCESS;
}
