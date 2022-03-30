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

delay_t delay;
pthread_barrier_t barrier;

typedef struct thread_args
{
    void* queue;
    double p;
    int iterations;
    double* random_probabilities;
    readings_t* readings;
    pthread_t tid;
} CACHE_ALIGNED thread_args;

void* thread_fn(void* in_args)
{
    thread_args* args = in_args;
    register_thread(args->iterations);
    int enqueued_item = 10;
    void* dequeued_item = NULL;

    pthread_barrier_wait(&barrier);
    start_readings(args->readings);
    
    for (int i = 0; i < args->iterations; ++i)
    {
        if (args->random_probabilities[i] < args->p)
            enqueue(args->queue, (void**)&enqueued_item);
        else if (dequeue(args->queue, (void*)&dequeued_item))
            assert(*((int*)dequeued_item) == enqueued_item);

        DELAY_OPS(delay.num_of_nops);
    }
    
    delta_readings(args->readings, args->iterations);
    adjust_readings_for_delay(args->readings, &delay);
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

    calibrate_delay(&delay, delay_ns);
    
    assert(p >= 0);
    assert(p <= 1);

    double** random_probabilities = (double**)malloc(sizeof(double*) * num_of_threads);
    ASSERT_NOT_NULL(random_probabilities);
    thread_args* args = (thread_args*)malloc(sizeof(thread_args) * num_of_threads);
    ASSERT_NOT_NULL(args);

    readings_t** readings = create_readings_2d(num_of_threads, TEST_RERUNS);
    
    // pthread_barrier_init returns zero when successful, however, zero is a falsy value in c.
    ASSERT_TRUE(!pthread_barrier_init(&barrier, NULL, num_of_threads), "Failed to create barrier");

    for (size_t i = 0; i < TEST_RERUNS; ++i)
    {
        void* queue;
        ASSERT_TRUE(create_queue(&queue), "Failed to create queue");
        for (size_t j = 0; j < num_of_threads; ++j)
        {
            int thread_iterations = iterations_per_thread(num_of_threads, j, TEST_ITERATIONS);
            random_probabilities[j] = (double*)malloc(sizeof(double) * thread_iterations);
            ASSERT_NOT_NULL(random_probabilities[j]);
            for (int k = 0; k < thread_iterations; ++k)
            {
                random_probabilities[j][k] = drand48();
            }
            args[j].readings = readings[j];
            args[j].iterations = thread_iterations;
            args[j].queue = queue;
            args[j].p = p;
            args[j].random_probabilities = random_probabilities[j];
            pthread_create(&args[j].tid, NULL, thread_fn, (void*)&args[j]);
        }

        for (size_t j = 0; j < num_of_threads; ++j)
        {
            pthread_join(args[j].tid, NULL);
        }
        destroy_queue(&queue);
    }

    readings_t* aggregates = aggregate_readings_2d(readings, num_of_threads, TEST_RERUNS);

    printf("\"%s\", %ld, %zu, ", get_queue_name(), num_of_threads, delay_ns);
    display_readings(aggregates);
    puts("");

    return EXIT_SUCCESS;
}
