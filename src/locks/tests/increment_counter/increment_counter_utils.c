#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "increment_counter_utils.h"
#include "../../lock.h"
#include "../../globals.h"

#define ITERATIONS 10
#define INCREMENT_VALUE 10

bool counter_test(int num_of_threads)
{
    void* lock;
    if (create_lock(&lock) != 0)
    {
        DEBUG_LOG("Create lock failed");
        assert(false);
    }
    int counter = 0;
    #pragma omp parallel shared(lock, counter)
    {
        for (int i = 0; i < ITERATIONS; ++i)
        {
            wait_lock(lock);
            counter += INCREMENT_VALUE;
            unlock(lock);
        }
    }
    DEBUG_LOG_F("Counter: %d", counter);
    assert(counter == (ITERATIONS * INCREMENT_VALUE * num_of_threads));
    destroy_lock(lock);
    return true;
}