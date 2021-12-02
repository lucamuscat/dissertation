#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include "../lock.h"
#include "../globals.h"
#include "./test_utils.h"

#define CLOCK_ID CLOCK_REALTIME
#define ITERATIONS 1000
#define INNER_ITERATIONS 100000
#define NAME_OF_LOCK "Lock"

int main(int argc, char** argv)
{
    handle_args(argc, argv);
    void* lock;
    create_lock(&lock);
    double time = 0;
    int64_t diff_time = 0;
    timespec start, stop = { 0, 0 };
    size_t j = 0;
    for (size_t i = 0; i < ITERATIONS; ++i)
    {
        clock_gettime(CLOCK_ID, &start);
        for (j = 0; j < INNER_ITERATIONS; ++j)
        {
            wait_lock(lock);
            unlock(lock);
        }
        clock_gettime(CLOCK_ID, &stop);
        diff_time = diff(&start, &stop);
        time += diff_time / INNER_ITERATIONS;
        assert(diff_time > 0);
    }
    time /= ITERATIONS;
    DEBUG_LOG_F("===============%s===============", argv[0]);
    DEBUG_LOG_F("\nNumber of threads: %d, ", omp_get_max_threads());
    DEBUG_LOG_F("\nAverage Time (nanoseconds): %lf\n\n", time);
    return 0;
}