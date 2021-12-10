#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <papi.h>
#include <math.h>
#include "../lock.h"
#include "../globals.h"
#include "./test_utils.h"

#define CLOCK_ID CLOCK_REALTIME
#define ITERATIONS 1000
#define INNER_ITERATIONS 1000
#define NAME_OF_LOCK "Lock"

__always_inline void PAPI_handle_error(int retval, char* context)
{
    if (retval != PAPI_OK)
    {
        PAPI_perror(context);
        exit(retval);
    }
}

#define INIT_ERROR "Library Init"
#define ADD_EVENT_ERROR "Add event"

int init_papi()
{
    int event_set = PAPI_NULL;
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT)
    {
        fprintf(stderr, "PAPI library init error!\n");
        exit(1);
    }

    PAPI_handle_error(PAPI_create_eventset(&event_set), INIT_ERROR);
    // Add number of Total Instructions Event Counter
    PAPI_handle_error(PAPI_add_event(event_set, PAPI_TOT_INS), "Add Event PAPI_TOT_INS");
    // Add number of Total Cycles Event Counter
    PAPI_handle_error(PAPI_add_event(event_set, PAPI_TOT_CYC), "Add Event PAPI_TOT_CYC");
    return event_set;
}

int main(int argc, char** argv)
{
    /* Initialize the PAPI library */
    int event_set = init_papi();
    int num_of_threads = handle_args(argc, argv);
    if (num_of_threads == -1)
        return -1;
    
    void* lock;
    create_lock(&lock);
    long long PAPI_results[2] = { 0, 0 };
    size_t i = 0, j = 0;
    for (i = 0; i < ITERATIONS; ++i)
    {
        PAPI_handle_error(PAPI_start(event_set), "Start timer:");
        for (j = 0; j < INNER_ITERATIONS; ++j)
        {
            wait_lock(lock);
            unlock(lock);
        }
        PAPI_handle_error(PAPI_stop(event_set, PAPI_results), "Stop timer:");
    }
    PAPI_results[0] /= ITERATIONS * INNER_ITERATIONS;
    PAPI_results[1] /= ITERATIONS * INNER_ITERATIONS;

#ifndef DEBUG
    // Output csv results
    printf("%s, %d, %lld, %lld\n", argv[0], num_of_threads, PAPI_results[0], PAPI_results[1]);
    #else
    printf("===============%s===============", argv[0]);
    printf("\nNumber of threads: %d, ", omp_get_max_threads());
    printf("\nAverage Total Instructions: %lld\n\n", PAPI_results[0]);
    printf("\nAverage Total Cycles: %lld\n\n", PAPI_results[0]);

#endif
    return 0;
}
