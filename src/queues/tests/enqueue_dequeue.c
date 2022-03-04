#include <papi.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>
#include "../../test_utils.h"
#include "../queue.h"

#define TEST_ITERATIONS 10000000
#define TEST_RERUNS 25
#define NANO_TO_MINUTE 1000000000*60
#define CACHE_LINE_SIZE 64

//static pthread_barrier_t barrier;

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
        //pthread_barrier_wait(&barrier);
        cycle_diff = PAPI_get_real_cyc();
        ns_diff = PAPI_get_real_nsec();
        for (size_t i = 0; i < args->num_of_iterations; ++i)
        {
            enqueue(args->queue, &cycle_diff);
            dequeue(args->queue, &dequeued_item);
        }
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
    int num_of_threads = handle_args(argc, argv);
    
    if (num_of_threads == -1)
        return num_of_threads;
    
    void* queue;
    PASS_LOG(create_queue(&queue), "Failed to create queue");

    pthread_mutex_t mutex;
    PASS_LOG(pthread_mutex_init(&mutex, NULL) == 0, "Failed to create pthread_mutex");

    double total_cycle_diff = 0;
    double total_ns_diff = 0;
    long long total_run_time_ns = PAPI_get_real_nsec();

    // Ensure that exactly TEST_ITERATIONS iterations occur, as integer division
    // might cause truncation and either increase or reduce the total number of
    // iterations.
    const int iterations_per_thread = (int)floor(((double)TEST_ITERATIONS) / num_of_threads);
    const int final_thread_iterations = TEST_ITERATIONS - (iterations_per_thread * (num_of_threads - 1));
    assert((iterations_per_thread * (num_of_threads - 1) + final_thread_iterations) == TEST_ITERATIONS);
    assert(sizeof(thread_args) == CACHE_LINE_SIZE);

    //PASS_LOG(pthread_barrier_init(&barrier, NULL, num_of_threads) == 0, "Failed to create pthread_barrier");

    thread_args* args = (thread_args*)malloc(sizeof(thread_args) * num_of_threads);
    for (int i = 0; i < num_of_threads; ++i)
    {
        args[i].mutex = &mutex;
        args[i].queue = queue;
        args[i].total_cycle_diff = &total_cycle_diff;
        args[i].total_ns_diff = &total_ns_diff;
        args[i].num_of_iterations = i < num_of_threads - 1 ? iterations_per_thread : final_thread_iterations;
        pthread_create(&args[i].tid, NULL, thread_fn, args);
    }

    for (int i = 0; i < num_of_threads; ++i)
    {
        pthread_join(args[i].tid, NULL);
    }

    total_run_time_ns = PAPI_get_real_nsec() - total_run_time_ns;
    
    double average_cycles = total_cycle_diff / num_of_threads;
    double average_ns = total_ns_diff / num_of_threads;
    
    printf("%s, %d, %f, %f, %lld\n", argv[0], num_of_threads, average_cycles, average_ns, total_run_time_ns);

    return 0;
}
