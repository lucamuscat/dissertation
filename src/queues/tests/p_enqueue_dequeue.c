/**
 * @file p_enqueue_dequeue.c
 * @brief Each queue will have probabilities p, and (1-p) to enqueue and dequeue
 * respectively.
 * @warning Make sure that the turbo boost and CPU frequency scaling are turned off.
 * Failure to do so will cause the DELAY macro to misbehave and the PAPI_get_real_cyc()
 * function to not match the actual total cycles.
 *
 * Low values of p may cause the system to run out of memory if TEST_ITERATIONS
 * is set to a high value.
 */

#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#include <pthread.h>
#include <assert.h>
#include <stdatomic.h>

#include "../../test_utils.h"
#include "../queue.h"

#define TEST_ITERATIONS 100000
#define TEST_RERUNS 25

double total_elapsed_time_ns = 0.0;
pthread_barrier_t barrier;
pthread_mutex_t mutex;

typedef struct thread_args
{
    void* queue;
    double p;
    int delay_ns;
    int iterations;
    double* random_probabilities;
    readings_t* readings;
    pthread_t tid;
} CACHE_ALIGNED thread_args;

void* thread_fn(void* in_args)
{
    PASS_LOG(PAPI_register_thread() == PAPI_OK, "Failed to register thread");
    thread_args* args = in_args;
    int enqueued_item = 10;
    void* dequeued_item = 0;
    for (size_t i = 0; i < TEST_RERUNS; ++i)
    {
        pthread_barrier_wait(&barrier);
        start_readings(args->readings);
        for (int j = 0; j < args->iterations; ++j)
        {
            if (args->random_probabilities[j] < args->p)
                enqueue(args->queue, (void**)&enqueued_item);
            else
                dequeue(args->queue, (void*)&dequeued_item);
            DELAY(args->delay_ns);
        }
        delta_readings(args->readings, args->iterations);
        // Empty the queue before the next test run.
        while (dequeue(args->queue, (void**)&dequeued_item))
            assert(*((int*)dequeued_item) == enqueued_item);
        
    }
    PASS_LOG(PAPI_unregister_thread() == PAPI_OK, "Failed to unregister thread");
    atomic_thread_fence(memory_order_seq_cst);
    return NULL;
}

int main(int argc, char const* argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Missing Args\n");
        fprintf(stderr, "\targ1: uint - Number of threads\n");
        fprintf(stderr, "\targ2: uint - Reentrancy delay in nanoseconds\n");
        fprintf(stderr, "\targ3: double - Probability of enqueue (p)\n");
        return EXIT_FAILURE;
    }

    int retval;
    if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT)
    {
        PAPI_perror("");
        return EXIT_FAILURE;
    }

    PASS_LOG(PAPI_thread_init((unsigned long (*)(void)) (pthread_self)) == PAPI_OK, "PAPI_thread_init failed");
    srand48(0);

    char* pEnd[3] = { "" };
    size_t num_of_threads = strtoul(argv[1], &pEnd[0], 10);
    int delay_ns = strtol(argv[2], &pEnd[1], 10);
    double p = strtod(argv[3], &pEnd[2]);

    assert(p >= 0);
    assert(p <= 1);

    double** random_probabilities = (double**)malloc(sizeof(double*) * num_of_threads);
    thread_args* args = (thread_args*)malloc(sizeof(thread_args) * num_of_threads);

    void* queue;
    PASS_LOG(create_queue(&queue), "Failed to create queue");
    PASS_LOG(pthread_mutex_init(&mutex, NULL) == 0, "Failed to create mutex");
    PASS_LOG(pthread_barrier_init(&barrier, NULL, num_of_threads) == 0, "Failed to create barrier");

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        int thread_iterations = iterations_per_thread(num_of_threads, i, TEST_ITERATIONS);
        random_probabilities[i] = (double*)malloc(sizeof(double) * thread_iterations);
        for (int j = 0; j < thread_iterations; ++j)
        {
            random_probabilities[i][j] = drand48();
        }
        create_readings(&args[i].readings, TEST_RERUNS);
        args[i].iterations = thread_iterations;
        args[i].queue = queue;
        args[i].p = p;
        args[i].random_probabilities = random_probabilities[i];
        args[i].delay_ns = delay_ns;
        pthread_create(&args[i].tid, NULL, thread_fn, (void*)&args[i]);
    }

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        pthread_join(args[i].tid, NULL);
    }

    readings_t** temp = (readings_t**)malloc(sizeof(readings_t*) * num_of_threads);

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        temp[i] = args[i].readings;
    }

    readings_t* readings = aggregate_readings(temp, num_of_threads, TEST_RERUNS);

    printf("\"%s\", %ld, %d, ", get_queue_name(), num_of_threads, delay_ns);
    display_readings(readings);
    puts("");

    return EXIT_SUCCESS;
}
