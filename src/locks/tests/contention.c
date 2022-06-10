#include <papi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include "../../test_utils.h"
#include "../../assertion_utils.h"
#include "../lock.h"

static pthread_barrier_t barrier;

typedef struct thread_args_t
{
    readings_t* readings;
    delay_t critical_section_delay;
    delay_t reentrancy_delay;
    size_t num_of_iterations;
    pthread_t tid;
    void* lock;
} CACHE_ALIGNED thread_args_t;

void* thread_fn(void* thread_args)
{
    thread_args_t* args = (thread_args_t*)thread_args;
    for (size_t i = 0; i < TEST_RERUNS; ++i)
    {
        pthread_barrier_wait(&barrier);
        start_readings(args->readings);
        for (size_t j = 0; j < args->num_of_iterations; ++j)
        {
            wait_lock(args->lock);
            DELAY_OPS(args->critical_section_delay.num_of_nops);
            unlock(args->lock);
            DELAY_OPS(args->reentrancy_delay.num_of_nops);
        }
        delta_readings(args->readings, args->num_of_iterations);
        adjust_readings_for_delay(args->readings, &args->critical_section_delay);
        adjust_readings_for_delay(args->readings, &args->reentrancy_delay);
    }
    
    atomic_thread_fence(memory_order_seq_cst);
    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Missing args,");
        fprintf(stderr, "\n\tArg 1: uint - Number of threads.");
        fprintf(stderr, "\n\tArg 2: uint - Length of critical section in nanoseconds.");
        fprintf(stderr, "\n\tArg 3: uint - Length of reentrancy delay in nanoseconds.\n");
        return EXIT_FAILURE;
    }

    char* pEnds[3] = { "" };
    
    const size_t num_of_threads = strtoul(argv[1], &pEnds[0], 10);
    const size_t critical_section_length_ns = strtoul(argv[2], &pEnds[1], 10);
    const size_t reentrancy_delay_ns = strtoul(argv[3], &pEnds[2], 10);

    delay_t critical_section_delay, reentrancy_delay;
    calibrate_delay(&critical_section_delay, critical_section_length_ns);
    calibrate_delay(&reentrancy_delay, reentrancy_delay_ns);

    void* lock;
    ASSERT_TRUE(create_lock(&lock), "Failed to create lock");
    ASSERT_TRUE(pthread_barrier_init(&barrier, NULL, num_of_threads) == 0, "Failed to create pthread_barrier");

    thread_args_t* args = (thread_args_t*)malloc(sizeof(thread_args_t) * num_of_threads);
    ASSERT_NOT_NULL(args);

    readings_t** readings = create_readings_2d(num_of_threads, TEST_RERUNS);
    
    for (size_t i = 0; i < num_of_threads; ++i)
    {
        args[i].lock = lock;
        args[i].readings = readings[i];
        args[i].num_of_iterations = iterations_per_thread(num_of_threads, i, TEST_ITERATIONS);
        args[i].critical_section_delay = critical_section_delay;
        args[i].reentrancy_delay = reentrancy_delay;
        pthread_create(&args[i].tid, NULL, thread_fn, &args[i]);
    }

    for (size_t i = 0; i < num_of_threads; ++i)
    {
        pthread_join(args[i].tid, NULL);
    }

    readings_t* aggregates = aggregate_readings_2d(readings, num_of_threads, TEST_RERUNS);

    printf("\"%s\", %zu, %zu, %zu, ", get_lock_name(), num_of_threads, critical_section_length_ns, reentrancy_delay_ns);
    display_readings(aggregates);
    puts("");
    return EXIT_SUCCESS;
}