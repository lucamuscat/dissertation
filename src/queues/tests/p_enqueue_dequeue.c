/**
 * @file p_enqueue_dequeue.c
 * @brief Each queue will have probabilities p, and (1-p) to enqueue and dequeue
 * respectively.
 */

#include <stdlib.h>
#include <stdio.h>
#include <papi.h>
#include <pthread.h>
#include <assert.h>

#include "../../test_utils.h"
#include "../queue.h"

#define TEST_ITERATIONS 100000

double total_elapsed_time_ns = 0.0;
pthread_barrier_t barrier;
pthread_mutex_t mutex;

typedef struct thread_args
{
    void* queue CACHE_ALIGNED;
    double p;
    int delay_ns;
    int iterations;
    double* random_probabilities;
    pthread_t tid;
} thread_args;

void* thread_fn(void* in_args)
{
    thread_args* args = in_args;
    int dummy = 10;
    int output = 0;
    pthread_barrier_wait(&barrier);
    long long elapsed_time = PAPI_get_real_nsec();
    for (int i = 0; i < args->iterations; ++i)
    {
        if (args->random_probabilities[i] < args->p)
            enqueue(args->queue, (void**)&dummy);
        else
            dequeue(args->queue, (void*)&output);
        DELAY(args->delay_ns);
    }
    elapsed_time = PAPI_get_real_nsec() - elapsed_time;
    pthread_mutex_lock(&mutex);
    total_elapsed_time_ns += ((double)elapsed_time) / args->iterations;
    pthread_mutex_unlock(&mutex);
    return 0;
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

    PAPI_library_init(PAPI_VER_CURRENT);
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

    printf("%s, %ld, %f, %d", get_queue_name(), num_of_threads, total_elapsed_time_ns / num_of_threads, delay_ns);

    return EXIT_SUCCESS;
}
