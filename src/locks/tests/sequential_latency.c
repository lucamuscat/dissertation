#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <papi.h>
#include "../lock.h"
#include "../globals.h"
#include "../../test_utils.h"

#define CLOCK_ID CLOCK_REALTIME
#define ITERATIONS 1000
#define INNER_ITERATIONS 1000
#define NAME_OF_LOCK "Lock"

int main()
{
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
    printf("%s, %lld\n", get_lock_name(), time);
    #else
    DEBUG_LOG_F("===============%s===============", get_lock_name());
    DEBUG_LOG_F("\nAverage Number of Cycles: %lld\n\n", time);
    #endif
    destroy_lock(&lock);
    return 0;
}
