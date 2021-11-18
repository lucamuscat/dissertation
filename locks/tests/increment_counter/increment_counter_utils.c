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
    omp_set_num_threads(num_of_threads);
    void* lock;
    create_lock(&lock);
    int counter = 0;
    #pragma omp parallel
    {
        for (int i = 0; i < ITERATIONS; ++i)
        {
            wait_lock(lock);
            counter += INCREMENT_VALUE;
            unlock(lock);
        }
    }
    assert(counter == (ITERATIONS * INCREMENT_VALUE * num_of_threads));
    free_lock(lock);
    return true;
}