#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <papi.h>
#include "../lock.h"
#include "../globals.h"
#include "./test_utils.h"

#define CLOCK_ID CLOCK_REALTIME
#define ITERATIONS 1000
#define INNER_ITERATIONS 1000
#define NAME_OF_LOCK "Lock"

int main(int argc, char** argv)
{
    int num_of_threads = handle_args(argc, argv);
    if (num_of_threads == -1)
        return -1;
    void* lock;
    create_lock(&lock);
    long long diff_time = 0, time = 0;
    size_t j = 0;
    for (size_t i = 0; i < ITERATIONS; ++i)
    {
        diff_time = PAPI_get_real_cyc();
        for (j = 0; j < INNER_ITERATIONS; ++j)
        {
            wait_lock(lock);
            unlock(lock);
        }
        diff_time = PAPI_get_real_cyc() - diff_time;
        time += diff_time / INNER_ITERATIONS;
        assert(diff_time > 0);
    }
    time /= ITERATIONS;
    #ifndef DEBUG
    // Output csv results
    printf("%s, %d, %lld\n", argv[0], num_of_threads, time);
    #else
    DEBUG_LOG_F("===============%s===============", argv[0]);
    DEBUG_LOG_F("\nNumber of threads: %d, ", omp_get_max_threads());
    DEBUG_LOG_F("\nAverage Number of Cycles: %lld\n\n", time);
    #endif
    return 0;
}
